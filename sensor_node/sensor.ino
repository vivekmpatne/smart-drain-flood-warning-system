#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// Wi-Fi
// ======================================================

#define WIFI_SSID " "
#define WIFI_PASSWORD " "


// ======================================================
// Team Name , Zone ID
// ======================================================

#define TEAM_NAME "team10"
#define ZONE_ID   "zone1"


// ======================================================
// DHT11
// ======================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


// ======================================================
// HC-SR04 Ultrasonic Sensor
// ======================================================

#define TRIG_PIN 25
#define ECHO_PIN 26


// ======================================================
// Water Level Calibration
// ======================================================

#define EMPTY_DISTANCE_CM 20
#define FULL_DISTANCE_CM  10

#if FULL_DISTANCE_CM >= EMPTY_DISTANCE_CM
#error "FULL_DISTANCE_CM must be less than EMPTY_DISTANCE_CM"
#endif


// ======================================================
// FLOAT SWITCH
// ======================================================
// NOT CONNECTED CURRENTLY
// Keep commented until physically wired.

#define FLOAT_PIN 27


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

String base = String(TEAM_NAME) + "/flood/" + String(ZONE_ID) + "/";

// SENSOR → NODE-RED

String topic_temp        = base + "temperature";
String topic_humidity    = base + "humidity";
String topic_level       = base + "level";
String topic_overflow    = base + "overflow";
String topic_node_status = base + "node_status";


// ======================================================
// MQTT CLIENT
// ======================================================

WiFiClient espClient;
PubSubClient client(espClient);


// ======================================================
// Wi-Fi CONNECTION
// ======================================================

void connectWiFi() {

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
// MQTT CONNECTION
// ======================================================

void connectMQTT() {

  while (!client.connected()) {

    Serial.print("Connecting to MQTT... ");

    // Generate a unique client ID using MAC address
    String clientId = "ESP32Client-";
    clientId += WiFi.macAddress();

    if (client.connect(
          clientId.c_str(),
          mqtt_user,
          mqtt_pass,

          // Last Will
          topic_node_status.c_str(),
          1,
          true,
          "offline"
        )) {

      Serial.println("Connected!");

      Serial.print("MQTT Client ID: ");
      Serial.println(clientId);

      // Tell Node-RED sensor node is alive
      client.publish(
        topic_node_status.c_str(),
        "online",
        true
      );
    }

    else {

      Serial.print("Failed, state=");
      Serial.print(client.state());

      Serial.println(" | retrying in 2 seconds");

      delay(2000);
    }
  }
}


// ======================================================
// ULTRASONIC WATER LEVEL
// ======================================================

float getWaterLevel() {

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);


  long duration =
      pulseIn(ECHO_PIN, HIGH, 30000);


  if (duration == 0) {

    Serial.println("WARNING: Ultrasonic timeout");

    return -1;
  }


  float distance =
      duration * 0.0343 / 2;


  float level =
    (
      (EMPTY_DISTANCE_CM - distance) /
      (EMPTY_DISTANCE_CM - FULL_DISTANCE_CM)
    ) * 100.0;


  if (level < 0)
    level = 0;

  if (level > 100)
    level = 100;


  Serial.print("Distance: ");
  Serial.print(distance);

  Serial.print(" cm | Water Level: ");
  Serial.print(level);

  Serial.println(" %");


  return level;
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
  Serial.println(" ESP32 SENSOR NODE - " + String(ZONE_ID));
  Serial.println("================================");


  dht.begin();


  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  // ====================================================
  // FLOAT SWITCH
  // ====================================================
  // NOT CONNECTED
  // Future:

  pinMode(FLOAT_PIN, INPUT_PULLUP);


  connectWiFi();


  client.setServer(
    mqtt_server,
    mqtt_port
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


  // ====================================================
  // READ SENSORS
  // ====================================================

  float temperature =
      dht.readTemperature();

  float humidity =
      dht.readHumidity();

  float waterLevel =
      getWaterLevel();


  // ====================================================
  // FLOAT SWITCH
  // ====================================================
  // NOT CONNECTED CURRENTLY
  //
  // Future:

  int floatState =
      digitalRead(FLOAT_PIN);

  int overflow =
      (floatState == LOW) ? 1 : 0;


  client.publish(
    topic_overflow.c_str(),
    String(overflow).c_str()
  );


  // ====================================================
  // TEMPERATURE
  // ====================================================

  if (!isnan(temperature)) {

    client.publish(
      topic_temp.c_str(),
      String(temperature, 2).c_str()
    );
  }


  // ====================================================
  // HUMIDITY
  // ====================================================

  if (!isnan(humidity)) {

    client.publish(
      topic_humidity.c_str(),
      String(humidity, 2).c_str()
    );
  }


  // ====================================================
  // WATER LEVEL
  // ====================================================

  if (waterLevel >= 0) {

    client.publish(
      topic_level.c_str(),
      String(waterLevel, 2).c_str()
    );
  }


  // ====================================================
  // SERIAL MONITOR
  // ====================================================

  Serial.println();
  Serial.println("---------- SENSOR DATA ----------");


  Serial.print("Temperature: ");

  Serial.println(
    isnan(temperature)
      ? "ERROR"
      : String(temperature) + " C"
  );


  Serial.print("Humidity: ");

  Serial.println(
    isnan(humidity)
      ? "ERROR"
      : String(humidity) + " %"
  );


  Serial.print("Water Level: ");

  Serial.println(
    waterLevel < 0
      ? "SENSOR ERROR"
      : String(waterLevel) + " %"
  );


  Serial.println("--------------------------------");


  delay(3000);
}
