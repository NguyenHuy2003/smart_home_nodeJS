#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>

const int trigPin = 2; // Distance sensor
const int echoPin = 4; // Distance sensor
const int DHT_PIN = 15; // DHT sensor
const int servoPin = 5; // Servo motor
const int ledPins[] = {12, 13, 25, 14}; // LED pins
const int stepsPerRevolution = 200; // Stepper motor
#define SOUND_SPEED 0.034

WiFiClient espClient;
#define FIREBASE_HOST "https://flutter-firebase-app-90c9a-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCgC_bd-p6bGBDeLFjq9lTxLJQOFn6_7dM"

DHTesp dhtSensor;
Servo servo;
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

long duration;
float distanceCm;

// Function to control LEDs
void controlLED(int pin, const String &path) {
  if (Firebase.getString(firebaseData, path)) {
    int ledStatus = firebaseData.intData();
    digitalWrite(pin, ledStatus == 1 ? HIGH : LOW);
  }
}

void hc_sr04() {
  int pos = 0;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Firebase.setInt(firebaseData, "/PTIOT_DHT/Dis", distanceCm);
  if (distanceCm >= 100) {
    Serial.println("Không có người");
    servo.write(180);
  } else {
    Serial.println("Có người");
    servo.write(0);
  }
}

void dht() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  Firebase.setInt(firebaseData, "/PTIOT_DHT/Temp", temperature);
  Firebase.setInt(firebaseData, "/PTIOT_DHT/hum", humidity);

  digitalWrite(19, temperature >= 40 ? HIGH : LOW);
  Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_02", temperature >= 40 ? 1 : 0);

  digitalWrite(18, humidity >= 30 && humidity <= 60 ? HIGH : LOW);
  Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_03", humidity >= 30 && humidity <= 60 ? 1 : 0);

  delay(1000);
}

void WiFiStatus() {
  int32_t rssi = WiFi.RSSI();
  String ssid = WiFi.SSID();
  String ip = WiFi.localIP().toString();
  bool isConnected = WiFi.isConnected();

  Serial.print("WiFi SSID: ");
  Serial.println(ssid);
  Serial.print("RSSI: ");
  Serial.println(rssi);
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("Connected: ");
  Serial.println(isConnected ? "Yes" : "No");

  Firebase.setString(firebaseData, "/PTIOT_DHT/WiFi_SSID", ssid);
  Firebase.setInt(firebaseData, "/PTIOT_DHT/WiFi_RSSI", rssi);
  Firebase.setString(firebaseData, "/PTIOT_DHT/WiFi_IP", ip);
  Firebase.setBool(firebaseData, "/PTIOT_DHT/WiFi_Connected", isConnected);
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  servo.attach(servoPin, 500, 2400);

  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to Wi-Fi, IP address: ");
  Serial.println(WiFi.localIP());

  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop() {
  WiFiStatus();
  controlLED(ledPins[0], "/PTIOT_DHT/LED_01"); //led_04
  controlLED(ledPins[1], "/PTIOT_DHT/LED_04"); //led_01
  controlLED(ledPins[2], "/PTIOT_DHT/LED_05"); // led_06
  controlLED(ledPins[3], "/PTIOT_DHT/LED_06"); // led_05
  hc_sr04();
  dht();
  delay(1000);
}
