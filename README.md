# esphome-hcpbridge

[![GitHub](https://img.shields.io/github/license/thomasdanz/esphome-hcpbridge)](https://github.com/thomasdanz/esphome-hcpbridge/blob/main/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/thomasdanz/esphome-hcpbridge)](https://github.com/thomasdanz/esphome-hcpbridge)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/mapero)](https://github.com/sponsors/mapero)
![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/thomasdanz/esphome-hcpbridge/build.yaml)


This is a esphome-based adaption of the HCPBridge. thanks to [mapero](https://github.com/14yannick/esphome-hcpbridge) for the initial esphome port. Credits for the initial development of the HCPBridge go to [Gifford47](https://github.com/Gifford47/HCPBridgeMqtt), [hkiam](https://github.com/hkiam/HCPBridge) and all the other guys contributed.

## Usage

### Example esphome configuration

```YAML
substitutions:
  name: "hcpbridge"
  friendly_name: "Garage Door"
esphome:
  name: "${name}"
  friendly_name: "${friendly_name}"
  libraries:
    # The registry name alone no longer resolves on current ESPHome; it needs a
    # name=URL form. This fork also fixes a dead bounds check in the upstream
    # library (see https://github.com/thomasdanz/modbus-esp8266).
    - modbus-esp8266=https://github.com/thomasdanz/modbus-esp8266

external_components:
  - source: github://thomasdanz/esphome-hcpbridge
    refresh: always # Ensure you always get the latest version

esp32:
  board: #adafruit_feather_esp32s3 #set your board
  framework:
    type: arduino

hcpbridge:
  id: hcpbridge_id
  rx_pin: 18 # optional, default=18
  tx_pin: 17 # optional, default=17
  #rts_pin : 1 # optional RTS pin to use if hardware automatic control flow is not available.

cover:
  - platform: hcpbridge
    name: ${friendly_name}
    device_class: garage
    id: garagedoor_cover
```

### Home Assistant

![Home Assistant Device Overview](docs/device_overview.png)

### Cover

The component provides a cover component to control the garage door.

### Light

The component provides a Light component to turn the light off and on.
The Output is needed to control the light.
```YAML
output:
  - platform: hcpbridge
    id: output_light

light:
  - platform: hcpbridge
    id: gd_light
    output: output_light
    name: Garage Door Light
```
### Binary_Sensor

The component provides you two sensor.

- `is_connected`: Who indicated if there is a valid connection with the door.
- `relay_state`: Give the status of the option relay (Menu 30) of the HCP.
```YAML
binary_sensor:
  - platform: hcpbridge
    is_connected:
      name: "HCPBridge Connected"
      id: sensor_connected
    relay_state:
      name: "Garage Door Relay state"
      id: sensor_relay
      #on_state:
      #create your automation based on Garage Door Relay state
```
### Text_sensor

This component provide you a detailed current state of the door. This text can be changed using the substitute functionality.
```YAML
text_sensor:
  - platform: hcpbridge
    id: sensor_templ_state
    name: "Garage Door State"
```
### sensor

This component provide you the position of the door in %. Where 100% is fully open.
```YAML
sensor:
  - platform: hcpbridge
    id: sensor_position
    name: "Garage Door Position"
```
### Button

This component allows you to add three buttons to sond commands to the door.
```YAML
button:
  - platform: hcpbridge
    vent_button:
      id: button_vent
      name: "Garage Door Vent"
    impulse_button:
      id: button_impulse
      name: "Garage Door Impulse"
    half_button:
      id: button_half
      name: "Half"
```

### Switch

This component allows you to add two switch to sond commands to the door.
```YAML
switch:
  - platform: hcpbridge
    vent_switch:
      id: switch_vent
      name: "Venting"
      restore_mode: disabled
    half_switch:
      id: half_switch
      name: "Open Half"
      restore_mode: disabled
```

### Services

Additionally, when using the cover component, you can expose the following services to the API:

- `esphome.hcpbridge_go_to_close`: To close the garage door
- `esphome.hcpbridge_go_to_half`: To move the garage door to half position
- `esphome.hcpbridge_go_to_vent`: To move the garage door to the vent position
- `esphome.hcpbridge_go_to_open`: To open the garage door
- `esphome.hcpbridge_toggle`: Send an Impulse command to the door

There are in the YAML and not directly in the Cover to remove the API dependency there. This give the possibility to use the Cover without the API Component for exemple only with the web_server or mqtt.
```YAML
api:
  encryption:
    key: !secret api_key
  actions:
    - action: go_to_open
      then:
        - lambda: |-
            id(garagedoor_cover).on_go_to_open();
    - action: go_to_close
      then:
        - lambda: |-
            id(garagedoor_cover).on_go_to_close();
    - action: go_to_half
      then:
        - lambda: |-
            id(garagedoor_cover).on_go_to_half();
    - action: go_to_vent
      then:
        - lambda: |-
            id(garagedoor_cover).on_go_to_vent();
    - action: toggle
      then:
        - cover.toggle: garagedoor_cover
```

### Example YAML

Check out the [example_hcpbridge.yaml](./example_hcpbridge.yaml) for a complete yaml with all hcpbridge components.

# Known issues

### Door goes into an error state after an OTA update or ESP restart

Restarting the ESP (including OTA updates) briefly interrupts the HCP bus
connection. On some operators (confirmed on a Hörmann Promatic 4 / SupraMatic
4) this is enough to put the operator into an error state that only clears
with a power cycle of the operator itself - reconnecting the ESP alone does
not fix it.

`example_hcpbridge.yaml` includes an optional automation for this: if the
operator does not resume polling within 10s of an ESP boot, it turns off a
Home Assistant switch that powers the operator (and, if the ESP is powered
from the same bus, the ESP itself), relying on that switch's own auto-on
timer to bring both back. It retries up to 3 times and gives up after that,
to avoid endlessly power-cycling a genuinely broken operator. See the
comments above the `globals`/`script` section in the example for the exact
requirements (a smart plug with its own auto-on timer, and the "Allow the
device to perform Home Assistant actions" permission in Home Assistant).

# Design notes

### Why not ESPHome's native `modbus:` component

ESPHome's `modbus:` component gained slave/server mode support at some point,
but this project still implements its own Modbus RTU slave with the
`modbus-esp8266` library instead. The reason is timing: Hörmann's controller
expects a response within ~12ms, so this runs in its own dedicated,
highest-priority FreeRTOS task rather than a `Component::loop()` tick shared
with WiFi/API/etc. ESPHome's native Modbus server has no such task, and we
haven't verified the shared main loop is fast enough for this deadline.
Running it from our own task instead isn't a clean option either, since its
internal buffers were never designed for concurrent access from a second
task. See the comment on the `mb` member in
[hoermann.h](components/hcpbridge/hoermann.h) for more detail.

# Project

- HCPBridge from `Tysonpower` on an `Hörmann Promatic 4`

You can find more information on the project here: [Hörmann garage door via MQTT](https://community.home-assistant.io/t/hormann-garage-door-via-mqtt/279938/340)
Known working hardware are the ESP32 and S3 dual core chip.

# Contribute

I am open for contribution. Just get in contact with me.

# License

```
MIT License

Copyright (c) 2023 Jochen Scheib

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
