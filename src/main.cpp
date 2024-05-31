#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
#include <Stepper.h>


const int trigPin = 2; // khoang cach
const int echoPin = 4; // khoang cach
const int DHT_PIN = 15; // DHT
const int servoPin = 5; // servo
const int pirPin = 14;  // Chân kết nối cảm biến PIR
const int ledPin = 12;  // Chân kết nối đèn LED
const int ledPin_13 = 13;  // Chân kết nối đèn LED
const int stepPin = 25; // motor
const int dirPin = 33; // motor
const int stepsPerRevolution = 200; // motor
#define SOUND_SPEED 0.034

WiFiClient espClient;
#define FIREBASE_HOST "https://flutter-firebase-app-90c9a-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCgC_bd-p6bGBDeLFjq9lTxLJQOFn6_7dM"

Stepper myStepper(stepsPerRevolution, stepPin, dirPin);
DHTesp dhtSensor;
Servo servo;
FirebaseData firebaseData;
FirebaseConfig firebaseConfig; // Khai báo đối tượng FirebaseConfig
FirebaseAuth firebaseAuth; // Khai báo đối tượng FirebaseAuth

long duration;
float distanceCm;

//-----------------------------------------

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
    servo.write(0);  // Góc 0 độ
  }
}

//-----------------------------------

void dht() {
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  float temperature = (data.temperature);
  float humidity = (data.humidity);
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


  if (temperature >= 40) {
    digitalWrite(19, HIGH);
    Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_02", 1);
  } else {
    digitalWrite(19, LOW);
    Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_02", 0);
  }
  if (humidity >= 30 && humidity <= 60) {
    digitalWrite(18, HIGH);
    Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_03", 1);
  } else {
    digitalWrite(18, LOW);
    Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_03", 0);
  }
  delay(1000);
}

//-------------------------------------

void PIR() {
  int motion = digitalRead(pirPin);
  if (motion == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("có người bật đèn");
    Firebase.setString(firebaseData, "/PTIOT_DHT/LED_04", pirPin);
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("không có người tắt đèn");
    Firebase.setInt(firebaseData, "/PTIOT_DHT/LED_04", pirPin);
  }
}

//------------------------------------

void QUAT() {
  if (Firebase.getInt(firebaseData, "/PTIOT_DHT/QUAT") == true) {
    int quatValue = firebaseData.to<int>();
    if (quatValue == 1) {
      // Bật động cơ
      myStepper.step(stepsPerRevolution * 20); // Quay 20 vòng
    } else {
      // Tắt động cơ
      myStepper.step(0); // Không quay (tắt động cơ)
    }
  }
}
void BAT_TAT_LED() {
  if (Firebase.getString(firebaseData, "/PTIOT_DHT/LED_01")) {
    int ledStatus = firebaseData.intData();
    if (ledStatus == 1) {
      digitalWrite(ledPin_13, HIGH);
    } else if (ledStatus == 0) {
      digitalWrite(ledPin_13, LOW);
    }
  }
}

//-------------------------------------

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);

  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin_13, OUTPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  servo.attach(servoPin, 500, 2400);

  myStepper.setSpeed(60); // RPM

  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    
  }

  Serial.println();
  Serial.print("Connected to Wi-Fi, IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo Firebase
  firebaseConfig.host = FIREBASE_HOST; // Thiết lập host
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH; // Thiết lập legacy token

  // Initialize Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth); // Thay đổi hàm gọi Firebase
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop() {
  BAT_TAT_LED();
  hc_sr04();
  dht();
  PIR();
  QUAT();
  delay(1000);
}