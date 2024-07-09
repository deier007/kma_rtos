#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

//Define WiFi
#define ssid "MERCUSYS_3130"
#define password "72898789"

#define mqtt_server "1ca9ad83643c4e22b4a2f8e80836f03c.s1.eu.hivemq.cloud"
#define mqtt_username "kit2512"
#define mqtt_password "Ohio@2022"
#define mqtt_port 8883
#define command1_topic "mqtt"
#define commandcontrol_topic "control_house"
#define MSG_BUFFER_SIZE (400)

#define GAS_MIN 30
#define TEMP_MIN 35

// define previous value
int prevHumidity = -1;
int prevTemperature = -1;
int prevGas = -1;
int prevDeviceStates[5] = { -1, -1, -1, -1, -1 };

const char* Home_UID = "7f852382-6960-4a39-8aa6-b5d338432622";

const char* Pump_UID = "P3Y4AMvaybNoDsP9rCXu9M";
// relay 1 - fan
#define Devide_01_PIN 12
// relay 2
const char* Led1_UID = "RPTzKmzobMciYX7Vstk7m6";
#define Devide_02_PIN 14

const char* Led2_UID = "YSLYa9hxyjbrAKW2NMFTW5";
#define Devide_03_PIN 27

const char* Led3_UID = "5BAWsxvuUcUQKcaZ6FS6Tt";
#define Devide_04_PIN 26

const char* Fan_UID = "YtPUWBquvbvc4ahaDv2P4w";
#define Devide_05_PIN 25

const int devidePin[] = { 12, 14, 27, 26, 25 };
const int numdevide = sizeof(devidePin) / sizeof(devidePin[0]);

/* DHT11 ==> Humi and Temperature  */
#define DHTPIN 15      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT 22 (AM2302)
/* Gas Sensor Pin */
#define AO_PIN 35
#define BUZZ_PIN 32

DHT dht(DHTPIN, DHTTYPE);

//===========================
static const char* root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


//==========================================
// DUAL CORE SETUP
TaskHandle_t Task1;
TaskHandle_t Task2;


// Declare Variable
WiFiClientSecure espClient;
PubSubClient client(espClient);
char msg[MSG_BUFFER_SIZE];
unsigned long count = 0;

/*=======================================================================================*/

void callback(char* topic, byte* message, unsigned int length);
void publishMessage(const char* topic, String payload, boolean retained);
void reconnect();

/*=======================================================================================*/

void setup_wifi();
void setup_devide();
void setupcore();




void setup() {
  Serial.begin(115200);
  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.subscribe(commandcontrol_topic);
  setupcore();
  setup_devide();
  dht.begin();

  pinMode(BUZZ_PIN, OUTPUT_OPEN_DRAIN);

  digitalWrite(BUZZ_PIN, 1);
}

void loop() {
}

/*=========================================================================================*/
//Task1code:
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    if (!client.connected()) reconnect();
    client.loop();
    delay(500);
  }
}

//Task2code:
void Task2code(void* pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    int humidity = readHumidity();
    int temperature = readTemperature();
    int gas = convertToPercentage();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }

    // Create a JSON document
    StaticJsonDocument<400> doc;
    doc["h"] = humidity;
    doc["t"] = temperature;
    doc["g"] = gas;
    doc["h_id"] = Home_UID;
    doc[Pump_UID] = digitalRead(Devide_01_PIN) == 1 ? 0 : 1;
    doc[Led1_UID] = digitalRead(Devide_02_PIN) == 1 ? 0 : 1;
    doc[Led2_UID] = digitalRead(Devide_03_PIN) == 1 ? 0 : 1;
    doc[Led3_UID] = digitalRead(Devide_04_PIN) == 1 ? 0 : 1;
    doc[Fan_UID] = digitalRead(Devide_05_PIN) == 1 ? 0 : 1;

    int deviceStates[5];
    deviceStates[0] = doc[Pump_UID];
    deviceStates[1] = doc[Led1_UID];
    deviceStates[2] = doc[Led2_UID];
    deviceStates[3] = doc[Led3_UID];
    deviceStates[4] = doc[Fan_UID];

    // Check if values have changed
    bool hasChanged = (humidity != prevHumidity) || (temperature != prevTemperature) || (gas != prevGas);
    for (int i = 0; i < 5; i++) {
      if (deviceStates[i] != prevDeviceStates[i]) {
        hasChanged = true;
        break;
      }
    }

    if (hasChanged) {
      prevHumidity = humidity;
      prevTemperature = temperature;
      prevGas = gas;
      for (int i = 0; i < 5; i++) {
        prevDeviceStates[i] = deviceStates[i];
      }
      // Serialize JSON to string and print it
      
      serializeJson(doc, msg);
      Serial.println("SEND DATA");
      Serial.println(msg);
      publishMessage(command1_topic, msg, true);
    }
    delay(1000);
  }
}
/*=========================================================================================*/

void setupcore() {
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
  vTaskDelay(500);

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
  vTaskDelay(500);
}


/*=========================================================================================*/
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

/*=========================================================================================*/
void setup_devide() {
  for (int i = 0; i < numdevide; i++) {
    pinMode(devidePin[i], OUTPUT);
    digitalWrite(devidePin[i], HIGH);
  }
}
/*=========================================================================================*/
void callback(char* topic, byte* message, unsigned int length) {
  if (strcmp(topic, commandcontrol_topic) == 0) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
    StaticJsonDocument<400> doc;
    DeserializationError error = deserializeJson(doc, messageTemp);
    // Test if parsing succeeds
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    // Extract values and print them
    digitalWrite(Devide_01_PIN, int(doc[Pump_UID]));
    digitalWrite(Devide_02_PIN, int(doc[Led1_UID]));
    digitalWrite(Devide_03_PIN, int(doc[Led2_UID]));
    digitalWrite(Devide_04_PIN, int(doc[Led3_UID]));
    digitalWrite(Devide_05_PIN, int(doc[Fan_UID]));
  }
}

/*=========================================================================================*/
void publishMessage(const char* topic, String payload, boolean retained) {
  client.publish(topic, payload.c_str(), true);

  // Serial.println("Message publised [" + String(topic) + "]: " + payload);
}
/*=========================================================================================*/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-";  // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe(command1_topic);        // subscribe the topics here
      client.subscribe(commandcontrol_topic);  // subscribe the topics here

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");  // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}




// Function to read humidity
float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println(F("Failed to read humidity from DHT sensor!"));
    return NAN;
  }
  return h;
}

// Function to read temperature in Celsius
int readTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("Failed to read temperature from DHT sensor!"));
    return NAN;
  }
  if (t > TEMP_MIN) {
    digitalWrite(BUZZ_PIN, 0);
  } else {
    digitalWrite(BUZZ_PIN, 1);
  }
  return (int)t;
}

float convertToPercentage() {

  int gasValue = analogRead(AO_PIN);
  float voltage = gasValue * (5.0 / 4095.0);  // Convert ADC reading to voltage (assuming 5V range)
  int percentage;

  // Example conversion (adjust as per your sensor datasheet)

  percentage = map(gasValue, 0, 4095, 0, 100);  // Map ADC reading to 0-100%
  if (percentage > GAS_MIN) {
    digitalWrite(BUZZ_PIN, 0);
  } else {
    digitalWrite(BUZZ_PIN, 1);
  }
  return percentage;
}