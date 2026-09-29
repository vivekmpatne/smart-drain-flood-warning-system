# Intelligent Water Clogging and Flood Prevention Using Real-Time Data

**Smart Drain Monitoring System: Flood and Road Hazard Early Warning for Urban Drains (Bengaluru PoC)**

Samsung Innovation Campus (IoT) capstone project, RNS Institute of Technology, September 2026.

---

## Demo Videos

Videos are hosted on Google Drive because of file size limits on GitHub.

**[Watch demo videos (Google Drive folder)](https://drive.google.com/drive/folders/1869mVIRYLwwES12wQjYP1Cnl_X0ko-AE?usp=sharing)**

| Video | What it shows |
|-------|---------------|
| 01-dashboard-overview | Walkthrough of the live control-room dashboard |
| 02-project-demo-no-voiceover-subtitles | Full working demo with subtitles (no voiceover) |
| 03-final-presentation-to-instructor | Final presentation and explanation given on 17 Sept 2026 |

---

## Problem

Bengaluru floods within minutes of heavy rain. Storm-water drains overflow or get blocked, roads get waterlogged, and authorities usually learn about it only after the flooding has started. There is no real-time visibility of drain conditions.

## Solution

An IoT-based early warning system that:

- Monitors drain water level continuously
- Calculates the **rate of rise**, not just a fixed threshold
- Combines local sensor data with live weather data
- Classifies risk and alerts the control room before the situation becomes critical

A bucket is used as a small-scale model of a drain for the prototype.

---

## How It Works

```
Sensor Node (ESP32)
   HC-SR04 ultrasonic + float switch
          |
          v
   MQTT (EMQX broker)
          |
          v
   Node-RED (processing + risk logic)  <--  OpenWeatherMap API
          |
   +------+------------------+
   v      v                   v
Dashboard  Email alerts   Actuator Node (ESP32)
(FlowFuse) (control room)  LED + buzzer (on-site)
```

**Risk logic**

1. Raw ultrasonic readings are smoothed using an Exponential Moving Average (30% current reading, 70% previous value).
2. Rate of rise of the water level is calculated.
3. Water level, rate of rise and weather forecast are combined into a risk score.
4. Risk is classified as **Normal, Watch, Alert or Critical**.
5. If the water level jumps abnormally fast, the system flags Critical even when the absolute level is still low.
6. A float switch acts as a redundant sensor. If it triggers, the system goes to Critical directly.
7. If an Alert is not acknowledged within 60 seconds, it escalates to Critical automatically.

---

## Features

- Real-time water level monitoring
- EMA smoothing and rate-of-rise calculation
- Four alert states: Normal, Watch, Alert, Critical
- Weather-aware risk scoring (OpenWeatherMap: rain probability, humidity, temperature, clouds)
- Redundant sensing (ultrasonic + float switch)
- Auto-escalation after 60 seconds without acknowledgement
- Email alerts to the control room for Alert and Critical states
- Sensor and actuator node online/offline status
- Alert log and water level history graph
- Citizen dashboard to report blocked drains, overflow, damaged covers or garbage, with photos
- On-site LED indicators and buzzer

---

## Tech Stack

**Hardware**
- ESP32 (sensor node and actuator node)
- HC-SR04 ultrasonic sensor
- Float switch
- Red, yellow and green LEDs
- Buzzer

**Software and protocols**
- MQTT (EMQX broker)
- Node-RED
- FlowFuse Dashboard 2.0
- OpenWeatherMap API
- Email (SMTP) alerts
- Arduino C++ (ESP32 firmware)

---

## Repository Structure

```
smart-drain-flood-warning-system/
├── sensor_node/
│   └── sensor.ino            # ESP32 sensor node firmware (ultrasonic + float switch, publishes over MQTT)
├── actuator-node/
│   └── actuator.ino          # ESP32 actuator node firmware (LED + buzzer, subscribes over MQTT)
├── node-red/
│   └── flows.json            # Exported Node-RED flow (import this into Node-RED)
├── dashboard/
│   └── screenshots/          # Dashboard and citizen dashboard screenshots
├── docs/
│   ├── poster.jpeg           # Project poster
│   └── smart drain_20260916_232237_0000.pptx   # Presentation slides
├── LICENSE
└── README.md
```

---

## How to Run

1. **Flash the ESP32 boards**
   - Open `sensor_node/sensor.ino` and `actuator-node/actuator.ino` in Arduino IDE.
   - Set your Wi-Fi credentials and MQTT broker details.
   - Calibrate the empty and full drain heights in the sensor code.
2. **Set up Node-RED**
   - Install Node-RED and the FlowFuse Dashboard 2.0 nodes.
   - Import `node-red/flows.json`.
   - Add your OpenWeatherMap API key and SMTP email settings.
3. **Deploy the flow** and open the dashboard.

Note: API keys and credentials are not included in this repository.

---

## Future Enhancements

- Automated pump or valve control when Critical
- City-wide, multi-zone scaling on a single dashboard
- Drain blockage detection
- Waterproof sensors (for example JSN-SR04) for real deployment
- Mobile app and SMS alerts
- Integration with municipal control rooms

---

## Team : 

- [Vivek Patne](https://www.linkedin.com/in/vivekpatnem/)
- [Christo Savio George](https://www.linkedin.com/in/christosaviogeorge/)
- [Aditya Mathad](https://www.linkedin.com/in/aditya-mathad-796bb2378/)
- [Sandeep Patil](https://www.linkedin.com/in/sandeep-patil-a8bb493b2/)

Mentor: Tushar Das, Samsung Innovation Campus IoT, RNSIT

---

## License

See [LICENSE](LICENSE).
