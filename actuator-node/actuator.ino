#include <WiFi.h>
#include <PubSubClient.h>

// ======================================================
// Wi-Fi
// ======================================================


#define WIFI_SSID " "
#define WIFI_PASSWORD " "


// ======================================================
// Team Name and Zone ID
// ======================================================

#define TEAM_NAME "team10"
#define ZONE_ID   "zone1"


// ======================================================
// ACTUATORS
// ======================================================

// LEDs
#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 21

// Buzzer
#define BUZZER 23

// Relay / Pump
#define RELAY_PIN 22


// ======================================================
// MQTT
// ======================================================


// local host / server 
const char* mqtt_server = "broker.emqx.io";


const int mqtt_port = 1883;

const char* mqtt_user = "user";
const char* mqtt_pass = "passward";


// ======================================================
// MQTT TOPICS
// ======================================================

String base =
    String(TEAM_NAME) +
    "/flood/" +
    String(ZONE_ID) +
    "/";

// NODE-RED → ACTUATOR

String topic_actuator_led =
    base + "actuator/led";

String topic_actuator_pump =
    base + "actuator/pump";

// Actuator status

String topic_actuator_status =
    base + "actuator/status";


// ======================================================
// MQTT CLIENT
// ======================================================

WiFiClient espClient;
PubSubClient client(espClient);


// ======================================================
// Wi-Fi
// ======================================================

void connectWiFi() {

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}


// ======================================================
// LED CONTROL
// ======================================================

void setLEDs(
    bool green,
    bool yellow,
    bool red
) {

  digitalWrite(GREEN_LED, green);
  digitalWrite(YELLOW_LED, yellow);
  digitalWrite(RED_LED, red);
}


// ======================================================
// MQTT CALLBACK
// ======================================================

void callback(
    char* topic,
    byte* payload,
    unsigned int length
) {

  String message = "";

  for (
      unsigned int i = 0;
      i < length;
      i++
  ) {

    message +=
        (char)payload[i];
  }

  message.trim();

  String topicStr =
      String(topic);


  Serial.println();
  Serial.println("===== MQTT COMMAND =====");

  Serial.print("Topic: ");
  Serial.println(topicStr);

  Serial.print("Command: ");
  Serial.println(message);


  // ==================================================
  // LED COMMAND
  // ==================================================

  if (topicStr == topic_actuator_led) {

    // Turn everything OFF first

    setLEDs(
      false,
      false,
      false
    );

    // Buzzer OFF first

    digitalWrite(
      BUZZER,
      LOW
    );


    if (message == "GREEN") {

      setLEDs(
        true,
        false,
        false
      );

      Serial.println(
        "GREEN LED ON"
      );
    }


    else if (message == "YELLOW") {

      setLEDs(
        false,
        true,
        false
      );

      Serial.println(
        "YELLOW LED ON"
      );
    }


    else if (message == "RED") {

      setLEDs(
        false,
        false,
        true
      );

      digitalWrite(
        BUZZER,
        HIGH
      );

      Serial.println(
        "RED LED ON"
      );

      Serial.println(
        "BUZZER ON"
      );
    }
  }


  // ==================================================
  // PUMP COMMAND
  // ==================================================

  else if (
      topicStr == topic_actuator_pump
  ) {

    if (message == "ON") {

      digitalWrite(
        RELAY_PIN,
        HIGH
      );

      Serial.println(
        "PUMP / RELAY ON"
      );
    }


    else if (message == "OFF") {

      digitalWrite(
        RELAY_PIN,
        LOW
      );

      Serial.println(
        "PUMP / RELAY OFF"
      );
    }
  }
}


// ======================================================
// MQTT CONNECTION
// ======================================================

void connectMQTT() {

  while (!client.connected()) {

    Serial.print(
      "Connecting to MQTT... "
    );


    // Generate a unique client ID using MAC address
    String clientId = "ESP32Client-";
    clientId += WiFi.macAddress();


    if (client.connect(

          clientId.c_str(),

          mqtt_user,
          mqtt_pass,

          // Last Will
          topic_actuator_status.c_str(),

          1,
          true,

          "offline"
        )) {

      Serial.println(
        "Connected!"
      );

      Serial.print(
        "MQTT Client ID: "
      );

      Serial.println(
        clientId
      );


      // =================================================
      // SUBSCRIBE TO NODE-RED COMMANDS
      // =================================================

      client.subscribe(
        topic_actuator_led.c_str()
      );

      client.subscribe(
        topic_actuator_pump.c_str()
      );


      Serial.print(
        "Subscribed to: "
      );

      Serial.println(
        topic_actuator_led
      );


      Serial.print(
        "Subscribed to: "
      );

      Serial.println(
        topic_actuator_pump
      );


      // =================================================
      // ONLINE STATUS
      // =================================================

      client.publish(
        topic_actuator_status.c_str(),
        "online",
        true
      );
    }


    else {

      Serial.print(
        "Failed, state="
      );

      Serial.print(
        client.state()
      );

      Serial.println(
        " | retrying in 2 seconds"
      );

      delay(2000);
    }
  }
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(9600);

  delay(1000);


  Serial.println();
  Serial.println("================================");
  Serial.println(" SMART DRAIN MONITORING SYSTEM ");
  Serial.println(" ESP32 ACTUATOR NODE - " + String(ZONE_ID));
  Serial.println("================================");


  // ====================================================
  // GPIO
  // ====================================================

  pinMode(
    GREEN_LED,
    OUTPUT
  );

  pinMode(
    YELLOW_LED,
    OUTPUT
  );

  pinMode(
    RED_LED,
    OUTPUT
  );


  pinMode(
    BUZZER,
    OUTPUT
  );


  pinMode(
    RELAY_PIN,
    OUTPUT
  );


  // ====================================================
  // SAFE INITIAL STATE
  // ====================================================

  setLEDs(
    false,
    false,
    false
  );

  digitalWrite(
    BUZZER,
    LOW
  );

  digitalWrite(
    RELAY_PIN,
    LOW
  );


  // ====================================================
  // CONNECTIONS
  // ====================================================

  connectWiFi();

  client.setServer(
    mqtt_server,
    mqtt_port
  );

  client.setCallback(
    callback
  );
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  if (!client.connected()) {

    connectMQTT();
  }

  client.loop();
}

