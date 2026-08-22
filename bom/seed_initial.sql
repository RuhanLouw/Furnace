PRAGMA foreign_keys = ON;

INSERT INTO assemblies (assembly_id, name, description) VALUES
    (1, 'Furnace controller prototype', 'ESP32-WROOM-32E controller, local HMI, sensor/control interface, and safety/control components for the first design pass.');

INSERT INTO categories (category_id, name, parent_category_id, notes) VALUES
    (1, 'Controller', NULL, 'Microcontroller and controller carrier hardware.'),
    (2, 'Local HMI', NULL, 'Furnace-mounted non-touch display and physical controls.'),
    (3, 'Safety', NULL, 'Emergency stop, interlock, over-temperature, and safety chain components.'),
    (4, 'Heater Control', NULL, 'SSR, contactor, heater-enable, and power switching.'),
    (5, 'Temperature Sensing', NULL, 'Thermocouples and thermocouple interface hardware.'),
    (6, 'Power', NULL, 'Auxiliary supplies, converters, protection, and distribution.'),
    (7, 'Indicators', NULL, 'Panel LEDs and audible indicators.'),
    (8, 'Interconnect', NULL, 'Connectors, terminal blocks, wiring, and enclosure hardware.');

INSERT INTO suppliers (supplier_id, supplier_name, country, website, notes) VALUES
    (1, 'Micro Robotics', 'South Africa', 'https://www.robotics.org.za/', 'Local hobby/electronics supplier.'),
    (2, 'DIYElectronics', 'South Africa', 'https://www.diyelectronics.co.za/', 'Local hobby/electronics supplier.'),
    (3, 'Communica', 'South Africa', 'https://www.communica.co.za/', 'Local electronics supplier.'),
    (4, 'RS South Africa', 'South Africa', 'https://za.rs-online.com/', 'Industrial component distributor.'),
    (5, 'Mantech', 'South Africa', 'https://www.mantech.co.za/', 'Local electronics supplier.'),
    (6, 'Mouser South Africa', 'South Africa', 'https://www.mouser.co.za/', 'International distributor with South African storefront.');

INSERT INTO parts (part_id, category_id, manufacturer, manufacturer_part_number, part_name, description, selection_status, notes) VALUES
    (1, 1, 'Espressif', 'ESP32-WROOM-32E', 'ESP32-WROOM-32E development board or carrier module', 'Autonomous furnace controller for profile execution, PID, telemetry, and local HMI.', 'selected-owned', 'User already has an ESP32-WROOM-32E. Add supplier rows only for replacement/reference.'),
    (2, 2, 'Generic', 'ILI9341-2.8-SPI-TFT', '2.8 inch SPI TFT display, ILI9341, 320x240', 'Non-touch local HMI display. Touch capability may exist on modules but is not required.', 'candidate', 'Prefer SPI display with proven ESP32 library support.'),
    (3, 2, 'Generic', 'ROTARY-ENCODER-PUSH', 'Detented rotary encoder with push button', 'Menu navigation, value editing, and confirm action on the local HMI.', 'candidate', NULL),
    (4, 2, 'Industrial 22 mm', 'PB-GREEN-NO-22MM', 'Green 22 mm momentary NO Start pushbutton', 'Local Start request to ESP32 command handler.', 'candidate', 'ESP32 input only; not part of hard safety chain.'),
    (5, 2, 'Industrial 22 mm', 'PB-RED-NO-22MM', 'Red 22 mm momentary NO Stop pushbutton', 'Controlled Stop request to ESP32 command handler.', 'candidate', 'Separate from emergency stop.'),
    (6, 2, 'Industrial 22 mm', 'PB-WHITE-NO-22MM', 'White 22 mm momentary NO Back/Cancel pushbutton', 'Back/cancel navigation on local HMI.', 'candidate', NULL),
    (7, 3, 'Schneider Electric or equivalent', 'ESTOP-22MM-2NC', '22 mm latching mushroom emergency stop, 2 NC contacts', 'Hardwired emergency stop in heater-enable/safety chain.', 'candidate', 'Must be latching and mechanically suitable; preferably 2 NC plus optional NO feedback.'),
    (8, 3, 'Omron or equivalent', 'D4N-ROLLER-NC', 'Positive-opening roller lever limit switch', 'Door/lid interlock to remove heater enable and report state to ESP32.', 'candidate', 'Select actuator style after mechanical door/lid design is fixed.'),
    (9, 3, 'Thermal limit manufacturer TBD', 'OVERTEMP-LIMIT', 'Independent over-temperature limit device', 'Independent heater shutdown path not relying on ESP32 firmware.', 'needs-selection', 'Final part depends on mounting method and trip temperature.'),
    (10, 4, 'Omron or equivalent', 'G3NA-25B-UTU-DC5-24', 'Zero-cross AC solid-state relay, 25 A class, DC input', 'Time-proportional heater coil switching from ESP32-controlled isolated output stage.', 'candidate', 'Final current rating needs heater current plus derating and heatsink calculation.'),
    (11, 4, 'Schneider/Eaton/Noark equivalent', 'CONTACTOR-25A-24VDC', '25 A class contactor with 24 V DC coil', 'Hardware heater-enable power removal path upstream of SSR.', 'candidate', 'Coil voltage assumes a 24 V control supply. Final rating depends on heater supply/current.'),
    (12, 5, 'Analog Devices / Maxim', 'MAX31856', 'MAX31856 thermocouple interface module', 'Thermocouple signal conditioning with cold-junction compensation and fault detection.', 'candidate', 'Prefer over MAX6675/MAX31855 for better type support and fault handling. Need two channels/modules.'),
    (13, 5, 'TBD', 'TYPE-K-HIGHTEMP-1200C', 'High-temperature Type K thermocouple probe', 'TC1/TC2 furnace temperature measurement.', 'needs-selection', 'Type K is a starting assumption; Type N/R/S may be more appropriate near 1200 C depending accuracy and lifetime.'),
    (14, 6, 'Mean Well or equivalent', 'HDR-30-24', '24 V DC DIN-rail power supply, 30 W class', 'Control supply for contactor coil, interlock loop, relays, and field I/O.', 'candidate', 'Power budget must be confirmed.'),
    (15, 6, 'Generic', 'BUCK-24V-5V', '24 V to 5 V DC-DC buck converter', 'Derive 5 V rail for ESP32 dev board/display from 24 V control supply.', 'candidate', 'Use isolated or suitably rated converter if required by final safety architecture.'),
    (16, 7, 'Industrial 22 mm', 'LED-RUN-22MM', '22 mm panel indicator LED, green', 'Run status indication on local panel.', 'candidate', NULL),
    (17, 7, 'Industrial 22 mm', 'LED-FAULT-22MM', '22 mm panel indicator LED, red', 'Fault indication on local panel.', 'candidate', NULL),
    (18, 7, 'Generic', 'PIEZO-BUZZER-5V', '5 V panel buzzer', 'Audible alarm/fault notification.', 'optional', NULL);

INSERT INTO bom_items (bom_item_id, assembly_id, item_number, subnumber, part_id, quantity, unit, use_case, required_for, design_notes) VALUES
    (1, 1, 1, 1, 1, 1, 'ea', 'Autonomous controller for profile execution, PID, telemetry, and local HMI.', 'prototype', 'Use existing ESP32-WROOM-32E board first.'),
    (2, 1, 1, 2, 2, 1, 'ea', 'Local non-touch display for runtime, setpoint, TC1/TC2, output, and faults.', 'prototype', 'Touch layer, if present, is not used.'),
    (3, 1, 1, 3, 3, 1, 'ea', 'Local menu navigation and value editing.', 'prototype', NULL),
    (4, 1, 1, 4, 4, 1, 'ea', 'Physical Start command.', 'prototype', NULL),
    (5, 1, 1, 5, 5, 1, 'ea', 'Physical controlled Stop command.', 'prototype', NULL),
    (6, 1, 1, 6, 6, 1, 'ea', 'Back/cancel navigation.', 'prototype', 'White is easier to label as Back/Cancel than black on a dark panel.'),
    (7, 1, 2, 1, 7, 1, 'ea', 'Hardwired emergency stop in the heater-enable chain.', 'final', 'Can be simulated on bench but must be real for heater tests.'),
    (8, 1, 2, 2, 8, 1, 'ea', 'Door/lid interlock for heater shutdown and status feedback.', 'final', 'Mechanical selection pending.'),
    (9, 1, 2, 3, 9, 1, 'ea', 'Independent over-temperature shutdown.', 'final', 'Needs furnace thermal design input.'),
    (10, 1, 3, 1, 10, 1, 'ea', 'Zero-cross SSR for heater coil time-proportional control.', 'final', 'Bench prototype should use LED/low-voltage load first.'),
    (11, 1, 3, 2, 11, 1, 'ea', 'Contactor or hard heater-enable path upstream of SSR.', 'final', 'Do not rely on SSR alone for safety.'),
    (12, 1, 4, 1, 12, 2, 'ea', 'Thermocouple measurement channels TC1 and TC2.', 'prototype', 'Two modules or a two-channel design.'),
    (13, 1, 4, 2, 13, 2, 'ea', 'High-temperature furnace thermocouples.', 'prototype', 'Probe selection depends on furnace build and temperature accuracy requirements.'),
    (14, 1, 5, 1, 14, 1, 'ea', '24 V control supply.', 'prototype', 'Bench supply can be substituted during early testing.'),
    (15, 1, 5, 2, 15, 1, 'ea', '5 V rail for ESP32 dev board and display.', 'prototype', NULL),
    (16, 1, 6, 1, 16, 1, 'ea', 'Panel Run indicator.', 'prototype', NULL),
    (17, 1, 6, 2, 17, 1, 'ea', 'Panel Fault indicator.', 'prototype', NULL),
    (18, 1, 6, 3, 18, 1, 'ea', 'Optional audible alert.', 'optional', NULL);

INSERT INTO supplier_offers (offer_id, part_id, supplier_id, supplier_sku, price_amount, currency, price_basis, availability, link, checked_at, notes) VALUES
    (1, 1, 2, 'ESP32DEVKITCVIE', 150.00, 'ZAR', 'listed price, verify before order', NULL, 'https://www.diyelectronics.co.za/store/wireless-wifi/4216-esp32-development-board-wifi-bluetooth-ultra-low-power-consumption.html', '2026-07-25', 'Replacement/reference only; user already owns ESP32-WROOM-32E.'),
    (2, 1, 1, 'ESP32-DEV', NULL, 'ZAR', 'price not captured', NULL, 'https://www.robotics.org.za/ESP32-DEV', '2026-07-25', 'Alternative local ESP32 dev board source; verify module revision and price.'),
    (3, 2, 1, 'TFT-32-SPI-ILI9341', 287.50, 'ZAR', 'listed price, verify before order', NULL, 'https://www.robotics.org.za/TFT-32-SPI-ILI9341', '2026-07-25', '3.2 inch ILI9341 SPI module; touch feature, if included, can be ignored.'),
    (4, 2, 2, 'LCD-1387', NULL, 'ZAR', 'price not captured', NULL, 'https://www.diyelectronics.co.za/store/displays/1387-28-inch-320240-tft-touchscreen-display.html', '2026-07-25', '2.8 inch TFT module candidate; verify controller and 3.3 V compatibility.'),
    (5, 3, 1, 'ROT-ENC-TH', 23.00, 'ZAR', 'listed price, verify before order', NULL, 'https://www.robotics.org.za/ROT-ENC-TH', '2026-07-25', 'Through-hole incremental rotary encoder candidate.'),
    (6, 3, 3, 'ROTARY-ENCODER', 15.65, 'ZAR', 'listed price, verify before order', 'search result indicated out of stock', 'https://www.communica.co.za/products/rotary-encoder', '2026-07-25', 'Verify availability.'),
    (7, 4, 4, 'XB4BP31', 448.16, 'ZAR', 'listed price from search result, verify before order', NULL, 'https://za.rs-online.com/web/p/push-button-heads/6097566', '2026-07-25', 'Schneider Harmony green pushbutton head; may require compatible contact block.'),
    (8, 4, 3, 'PB300MG', NULL, 'ZAR', 'price not captured', NULL, 'https://www.communica.co.za/products/pb300mg', '2026-07-25', '22 mm green momentary 1NO+1NC candidate; verify current price and stock.'),
    (9, 5, 3, 'AVP22F-M3SCR12', 332.35, 'ZAR', 'listed inc VAT, verify before order', 'in stock at time of search result', 'https://www.communica.co.za/products/avp22f-m3scr12', '2026-07-25', '22 mm red momentary IP67 ring LED candidate for controlled Stop; do not confuse with E-stop.'),
    (10, 6, 3, 'PB300MW', 129.38, 'ZAR', 'listed inc VAT, verify before order', 'in stock at time of search result', 'https://www.communica.co.za/products/pb300mw', '2026-07-25', '22 mm white momentary 1NO+1NC candidate for Back/Cancel.'),
    (11, 7, 4, 'XB5AS84449', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/p/emergency-stop-push-buttons/3308637', '2026-07-25', 'Schneider Harmony emergency stop candidate; confirm contact arrangement and enclosure rating.'),
    (12, 7, 3, '3SU1150-1HB20-1PA0', NULL, 'ZAR', 'price not captured', NULL, 'https://www.communica.co.za/collections/electromechanical/pushbutton-switches', '2026-07-25', 'Communica search result showed Siemens SIRIUS ACT 22 mm emergency stop; verify exact contacts and price.'),
    (13, 8, 4, 'D4N-series', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/c/automation-control-gear/sensors-transducers/limit-switches/', '2026-07-25', 'Select exact actuator after mechanical door/lid layout.'),
    (14, 8, 3, 'WLCA2-2Y-N', NULL, 'ZAR', 'price not captured', NULL, 'https://www.communica.co.za/products/wlca2-2y-n', '2026-07-25', 'Omron roller lever limit switch candidate; page described IP67 and NO/NC support.'),
    (15, 10, 4, 'G3NA-225B-UTU DC5-24', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/p/solid-state-relays/1846730', '2026-07-25', 'Omron zero-cross SSR candidate; confirm heater current and heatsink.'),
    (16, 10, 1, 'SSR-25DA', NULL, 'ZAR', 'price not captured', NULL, 'https://www.robotics.org.za/SSR-25DA', '2026-07-25', 'Low-cost SSR candidate for prototype only; verify authenticity, derating, and heatsink.'),
    (17, 11, 4, 'CONTACTOR-25A-24VDC', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/c/switches/contactors/contractors/', '2026-07-25', 'Select exact contactor after heater voltage/current and control supply are fixed.'),
    (18, 12, 6, 'MAX31856', NULL, 'ZAR', 'price not captured', NULL, 'https://www.mouser.co.za/c/semiconductors/sensor-ics/temperature-sensor-ics/?q=MAX31856', '2026-07-25', 'IC/source candidate if designing custom thermocouple board.'),
    (19, 12, 2, 'ADA-3263', NULL, 'ZAR', 'price not captured', NULL, 'https://www.diyelectronics.co.za/store/sensors/3263-adafruit-universal-thermocouple-amplifier-max31856-breakout.html', '2026-07-25', 'Adafruit MAX31856 breakout candidate; verify availability.'),
    (20, 13, 4, 'TYPE-K-1200C', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/c/test-measurement/temperature-measurement/thermocouples/', '2026-07-25', 'Select probe/sheath after furnace geometry and thermocouple type are fixed.'),
    (21, 14, 4, 'HDR-30-24', NULL, 'ZAR', 'price not captured', NULL, 'https://za.rs-online.com/web/p/din-rail-power-supplies/1869564', '2026-07-25', 'Mean Well HDR-30-24 class candidate; verify control power budget.'),
    (22, 14, 3, 'MEANWELL-24V-DIN', NULL, 'ZAR', 'price not captured', NULL, 'https://www.communica.co.za/', '2026-07-25', 'Alternative local source for Mean Well 24 V DIN supply.'),
    (23, 15, 1, 'BUCK-5A-XL4015', NULL, 'ZAR', 'price not captured', NULL, 'https://www.robotics.org.za/DC-DC-STEP-DOWN', '2026-07-25', 'Prototype buck converter candidate; final product may need industrial converter.'),
    (24, 18, 1, 'BUZZER-5V', NULL, 'ZAR', 'price not captured', NULL, 'https://www.robotics.org.za/index.php?route=product/search&search=buzzer%205v', '2026-07-25', 'Optional buzzer candidate.');
