#pragma once

#include <Arduino.h>

const char DashboardPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Furnace control</title>
<style>
:root{font-family:system-ui,sans-serif;color:#202622;background:#f3f5f4;color-scheme:light}
*{box-sizing:border-box}body{margin:0}main{max-width:760px;margin:auto;padding:28px 22px 48px}
header{display:flex;align-items:center;justify-content:space-between;gap:14px;border-bottom:1px solid #cbd2ce;padding-bottom:20px}
h1{font-size:24px;margin:0}p{line-height:1.5}.sub{color:#59655e;font-size:14px;margin:7px 0 0}
.connection{display:flex;align-items:center;gap:7px;font-size:13px;white-space:nowrap}.dot{width:9px;height:9px;border-radius:50%;background:#8a948e}.dot.live{background:#18784c}
.readings{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:16px;padding:26px 0;border-bottom:1px solid #cbd2ce}
dt{font-size:12px;color:#59655e;margin-bottom:10px}dd{margin:0;font-size:22px;font-weight:650;overflow-wrap:anywhere}dl{margin:0}
.section{padding:26px 0;border-bottom:1px solid #cbd2ce}label{font-size:15px;font-weight:600}.duty-heading{display:flex;align-items:center;justify-content:space-between;gap:12px}
output{font-size:26px;font-weight:650}input[type=range]{display:block;width:100%;height:38px;margin:16px 0 2px;accent-color:#18784c;cursor:pointer}
.scale{display:flex;justify-content:space-between;color:#59655e;font-size:12px}.buttons{display:grid;grid-template-columns:1fr 1fr;gap:14px;margin-top:26px}
button{font:inherit;min-height:52px;border:1px solid transparent;border-radius:6px;font-weight:650;cursor:pointer;padding:12px 18px}
.on{background:#18784c;color:white}.off{background:#fff;color:#a0292e;border-color:#a0292e}button:disabled,input:disabled{opacity:.45;cursor:not-allowed}
button:focus-visible,input:focus-visible{outline:3px solid #69a8e0;outline-offset:4px}#message{min-height:44px;margin:16px 0 0;font-size:14px;color:#59655e}
.error{color:#a0292e!important}.note{font-size:13px;color:#59655e}footer{margin-top:25px;display:flex;justify-content:space-between;gap:15px;font-size:12px;color:#59655e}
@media(max-width:460px){main{padding:22px 16px}header{align-items:flex-start;flex-direction:column}.readings{gap:10px}dd{font-size:18px}h1{font-size:22px}}
</style>
</head>
<body>
<main>
<header><div><h1>Furnace control</h1><p class="sub">Manual bench control</p></div><div class="connection" role="status"><span id="dot" class="dot"></span><span id="connection">Connecting</span></div></header>
<section class="readings" aria-label="Controller readings">
<dl><dt>Output</dt><dd id="enabled">--</dd></dl>
<dl><dt>Applied duty</dt><dd id="applied">--</dd></dl>
<dl><dt>SSR command</dt><dd id="ssr">--</dd></dl>
</section>
<section class="section">
<div class="duty-heading"><label for="duty">Duty cycle</label><output id="draft" for="duty">0%</output></div>
<input id="duty" type="range" min="0" max="100" step="5" value="0" disabled aria-describedby="duty-note">
<div class="scale"><span>0%</span><span>50%</span><span>100%</span></div>
<p id="duty-note" class="note">1-second switching window. 50% = 0.5 s ON / 0.5 s OFF.</p>
<div class="buttons"><button id="on" class="on" disabled>Enable output</button><button id="off" class="off">OFF</button></div>
<p id="message" role="status" aria-live="polite">Waiting for controller state.</p>
</section>
<p class="note">No automatic timeout or temperature regulation. SSR command is not measured heater current.</p>
<footer><span>ESP32-S3 / N16R8</span><span id="uptime">Uptime --</span></footer>
</main>
<script>
const ui=Object.fromEntries(['dot','connection','enabled','applied','ssr','duty','draft','on','off','message','uptime'].map(id=>[id,document.getElementById(id)]));
let online=false,busy=false,editing=false,polling=false,lastState=null;
function controls(){ui.on.disabled=!online||busy;ui.duty.disabled=!online||busy;ui.off.disabled=busy;}
function message(text,error=false){ui.message.textContent=text;ui.message.classList.toggle('error',error);}
function disconnected(){online=false;ui.dot.classList.remove('live');ui.connection.textContent='Disconnected';ui.enabled.textContent='Unknown';ui.applied.textContent='--';ui.ssr.textContent='Unknown';controls();}
async function request(path,options={}){const abort=new AbortController(),timer=setTimeout(()=>abort.abort(),3000);try{const response=await fetch(path,{...options,signal:abort.signal,cache:'no-store'});if(!response.ok)throw new Error(response.status===401?'Authentication required':'Request failed ('+response.status+')');return await response.json();}finally{clearTimeout(timer);}}
function render(state){lastState=state;online=true;ui.dot.classList.add('live');ui.connection.textContent='Connected';ui.enabled.textContent=state.enabled?'Enabled':'Disabled';ui.applied.textContent=state.duty+'%';ui.ssr.textContent=state.ssr_on?'ON':'OFF';ui.uptime.textContent='Uptime '+state.uptime_s+' s';if(!editing){ui.duty.value=state.duty;ui.draft.textContent=state.duty+'%';}controls();}
async function refresh(){if(polling||busy)return;polling=true;try{const state=await request('/api/status');if(!busy)render(state);}catch(error){if(!busy){disconnected();message('Connection lost. Output state is unknown; it may still be running.',true);}}finally{polling=false;}}
async function command(path,value){if(busy)return;busy=true;editing=false;controls();message('Waiting for controller confirmation...');try{const queued=await request(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded','X-Furnace-Control':'1'},body:new URLSearchParams({value:String(value)})});let confirmed=false;for(let attempt=0;attempt<12;attempt++){const state=await request('/api/status');render(state);if(((state.last_command_id-queued.id)|0)>=0){confirmed=true;break;}await new Promise(resolve=>setTimeout(resolve,75));}if(!confirmed)throw new Error('Controller did not confirm the command');message(lastState.enabled?'Output enabled. No automatic timeout.':'Output disabled.');}catch(error){disconnected();message(error.message+'. Check controller state before continuing.',true);}finally{busy=false;controls();}}
ui.duty.addEventListener('input',()=>{editing=true;ui.draft.textContent=ui.duty.value+'%';});
ui.duty.addEventListener('change',()=>command('/api/duty',ui.duty.value));
ui.on.addEventListener('click',()=>command('/api/output',1));
ui.off.addEventListener('click',()=>command('/api/output',0));
message('Select a duty, then enable output.');refresh();setInterval(refresh,500);
</script>
</body>
</html>)HTML";

