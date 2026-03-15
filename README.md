# Smart Alarm Clock
Smart Iot based Desk Alarm clock which have a web based synced simulation control and it also notify you on screen when you get an message or you got pinged on slack its keeps you updated and its also have animation Screens to make it your desk showcase 
## WEB UI DEMO - https://smart-alarm-demo.vercel.app
# Componet Used -
- Aruduino UNO (to control)
- ESP32 (to connect with Internet / Firebase )
- LCD 16x2 I2C Display (to show content)
- BUZZER 
- PUSH BUTTONS

# Circuit 
 <img src="https://raw.githubusercontent.com/tejasmmali/smart-iot-alarm/refs/heads/main/images/circuit_image.png"/>
 circuit deign link - https://app.cirkitdesigner.com/project/50af74ac-e6c1-45c3-b1f8-8858fe94862f

# Programing 
**For Web Ui**
- HTML
- CSS
- JAVA SCRIPT

**For UNO and ESP32**
- C++

**Other**
- Pipedream (slack notification)
- Slack App API
- NTP API (Fetch Real Time)
- Firebase Real time DB (to store input and Sync)

## How its Works 
so Esp32 is connect to through TX2 to RX arduino and all other component connected to Arduino UNO ESP32 only fetch data from internet and send it to arduino uno like shown below

        ┌───────────────┐
        │     User      │
        │  Web Browser  │
        └───────┬───────┘
                │
                │ Enter Alarm / Data
                ▼
        ┌────────────────────┐
        │   Web Application  │
        │  (HTML / JS / API) │
        └─────────┬──────────┘
                  │
                  │ Send HTTP Request
                  ▼
        ┌─────────────────────────┐
        │ Firebase Realtime DB    │
        │  Stores Alarm / Time    │
        └─────────┬───────────────┘
                  │
                  │ ESP32 HTTP GET
                  ▼
        ┌──────────────────────┐
        │        ESP32         │
        │ Connects to WiFi    │
        │ Fetch JSON Data     │
        └─────────┬───────────┘
                  │
                  │ Parse JSON
                  ▼
        ┌──────────────────────┐
        │  Alarm Logic Check   │
        │  Compare with Time   │
        └─────────┬───────────┘
                  │
                  │ Trigger Action
                  ▼
        ┌──────────────────────┐
        │ LCD / Display / LED  │
        │ Show Time / Alarm    │
        │ Buzzer 
        └──────────────────────┘

### All the Problem i faced and how did i solved it thats given in devlog please checout!
# NOTE -
<p> i gonna make this arlarm a real product by making cad and 3d printing its body and upgrading some features .</p>
<p>i am pausing this project development because of examination after exams i will make a plan to ugrade this in full product .</p>
<p>in future i am making this project web control open to all anyone can make there alarnm clcok and use this web ui in by connecting it through server which have there UID</p>
<p>and i am gonna make full guide website for how to make this alarm step by step guide </p>

# THANK YOU ! 

