#include "WebDashboard.h"

#include <WebServer.h>
#include <WiFi.h>

#include "DashboardPage.h"
#include "TestController.h"

#if __has_include("NetworkSecrets.h")
#include "NetworkSecrets.h"
#else
namespace NetworkSecrets {
constexpr char WifiSsid[] = "";
constexpr char WifiPassword[] = "";
constexpr char DashboardPassword[] = "";
}
#endif

namespace {

WebServer server(80);
constexpr char AccessPointPassword[] = "adminpassword";
String dashboardPassword;
uint32_t connectStartedMs = 0;
uint32_t commandId = 0;
bool accessPointStarted = false;
bool stationAnnounced = false;

bool authenticate() {
    if (!server.authenticate("admin", dashboardPassword.c_str())) {
        server.requestAuthentication(BASIC_AUTH, "Furnace bench");
        return false;
    }
    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.sendHeader("X-Frame-Options", "DENY");
    return true;
}

void sendStatus() {
    if (!authenticate()) {
        return;
    }
    const TestController::Snapshot state = TestController::snapshot();
    char json[320];
    snprintf(json, sizeof(json),
             "{\"enabled\":%s,\"ssr_on\":%s,\"duty\":%u,\"last_command_id\":%lu,\"uptime_s\":%lu}",
             state.enabled ? "true" : "false", state.ssrOn ? "true" : "false",
             static_cast<unsigned int>(state.dutyPercent),
             static_cast<unsigned long>(state.lastCommandId),
             static_cast<unsigned long>(millis() / 1000));
    server.send(200, "application/json", json);
}

void receiveControl(bool dutyCommand) {
    if (!authenticate()) {
        return;
    }
    if (server.header("X-Furnace-Control") != "1" || server.args() != 1 || !server.hasArg("value")) {
        server.send(400, "application/json", "{\"error\":\"Invalid control request\"}");
        return;
    }
    const String value = server.arg("value");
    if (value.length() == 0 || value.length() > 3) {
        server.send(400, "application/json", "{\"error\":\"Invalid value\"}");
        return;
    }
    for (size_t index = 0; index < value.length(); ++index) {
        if (value[index] < '0' || value[index] > '9') {
            server.send(400, "application/json", "{\"error\":\"Expected integer\"}");
            return;
        }
    }
    const int parsed = value.toInt();
    if (parsed > (dutyCommand ? 100 : 1)) {
        server.send(400, "application/json", "{\"error\":\"Out of range\"}");
        return;
    }
    if (++commandId == 0) {
        ++commandId;
    }
    const auto type = dutyCommand ? TestController::CommandType::SetDuty :
                      (parsed ? TestController::CommandType::Enable : TestController::CommandType::Disable);
    if (!TestController::submit(type, static_cast<uint8_t>(parsed), commandId)) {
        server.send(503, "application/json", "{\"error\":\"Command queue full\"}");
        return;
    }
    server.send(202, "application/json", "{\"queued\":true,\"id\":" + String(commandId) + "}");
}

void startAccessPoint() {
    WiFi.mode(NetworkSecrets::WifiSsid[0] == '\0' ? WIFI_AP : WIFI_AP_STA);
    accessPointStarted = WiFi.softAP("Furnace-Test", AccessPointPassword);
    if (accessPointStarted) {
        Serial.printf("Test Wi-Fi: Furnace-Test, password: %s\n", AccessPointPassword);
        Serial.printf("Dashboard: http://%s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("Could not start test Wi-Fi access point");
    }
}

}

namespace WebDashboard {

void begin() {
    dashboardPassword = NetworkSecrets::DashboardPassword;
    if (dashboardPassword.length() < 8 || dashboardPassword.length() > 63) {
        dashboardPassword = AccessPointPassword;
    }
    Serial.printf("Dashboard login: admin / %s\n", dashboardPassword.c_str());
    WiFi.persistent(false);
    if (NetworkSecrets::WifiSsid[0] != '\0') {
        WiFi.mode(WIFI_STA);
        WiFi.setHostname("furnace-test");
        WiFi.setAutoReconnect(true);
        WiFi.begin(NetworkSecrets::WifiSsid, NetworkSecrets::WifiPassword);
        connectStartedMs = millis();
        Serial.println("Connecting to configured Wi-Fi...");
    } else {
        startAccessPoint();
    }
    const char *headers[] = {"X-Furnace-Control"};
    server.collectHeaders(headers, 1);
    server.on("/", HTTP_GET, []() {
        if (authenticate()) {
            server.send_P(200, "text/html", DashboardPage);
        }
    });
    server.on("/api/status", HTTP_GET, sendStatus);
    server.on("/api/duty", HTTP_POST, []() { receiveControl(true); });
    server.on("/api/output", HTTP_POST, []() { receiveControl(false); });
    server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
    server.begin();
}

void service() {
    if (NetworkSecrets::WifiSsid[0] != '\0') {
        if (WiFi.status() == WL_CONNECTED) {
            if (!stationAnnounced) {
                Serial.printf("LAN dashboard: http://%s\n", WiFi.localIP().toString().c_str());
                stationAnnounced = true;
            }
        } else {
            stationAnnounced = false;
            if (!accessPointStarted && millis() - connectStartedMs >= 20000) {
                startAccessPoint();
            }
        }
    }
    server.handleClient();
}

void printInfo() {
    Serial.printf("Dashboard login: admin / %s\n", dashboardPassword.c_str());
    if (accessPointStarted) {
        Serial.printf("Wi-Fi: Furnace-Test, password: %s\n", AccessPointPassword);
        Serial.printf("Dashboard: http://%s\n", WiFi.softAPIP().toString().c_str());
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("LAN dashboard: http://%s\n", WiFi.localIP().toString().c_str());
    }
}

}

