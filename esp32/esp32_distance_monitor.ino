#include <WiFi.h>
#include <HTTPClient.h>

// Set these locally. Do not commit real Wi-Fi credentials to a public repo.
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL = "http://192.168.1.170/esp_project/insert.php";

unsigned long lastPostTime = 0;
const unsigned long POST_INTERVAL = 5000;

const int leds[] = {23, 22, 21, 19, 18, 15, 13, 12, 4, 2};
const int NUM_LEDS = 10;

const int RED_BUTTON = 14;
const int BLUE_BUTTON = 27;
const int WHITE_BUTTON = 26;

const int TRIG_PIN = 25;
const int ECHO_PIN = 33;

#define SOUND_SPEED 0.0343
const float MAX_DISTANCE = 50.0;
const float DANGER_DISTANCE = 10.0;
const int SPEED = 100;

void allOff();
void leftToRight();
void rightToLeft();
void centerToBoth();
void ultrasonicSensor();
float getFilteredDistance();
void sendDistanceToServer(float distanceCm, String statusStr, bool forceSend = false);

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(WHITE_BUTTON, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  allOff();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (digitalRead(RED_BUTTON) == LOW) {
    delay(30);
    if (digitalRead(RED_BUTTON) == LOW) {
      leftToRight();
      while (digitalRead(RED_BUTTON) == LOW) delay(10);
      return;
    }
  }

  if (digitalRead(BLUE_BUTTON) == LOW) {
    delay(30);
    if (digitalRead(BLUE_BUTTON) == LOW) {
      rightToLeft();
      while (digitalRead(BLUE_BUTTON) == LOW) delay(10);
      return;
    }
  }

  if (digitalRead(WHITE_BUTTON) == LOW) {
    delay(30);
    if (digitalRead(WHITE_BUTTON) == LOW) {
      centerToBoth();
      while (digitalRead(WHITE_BUTTON) == LOW) delay(10);
      return;
    }
  }

  ultrasonicSensor();
  delay(100);
}

void allOff() {
  for (int i = 0; i < NUM_LEDS; i++) digitalWrite(leds[i], LOW);
}

float getFilteredDistance() {
  float total = 0;
  int validReadings = 0;

  for (int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 25000);
    if (duration > 0) {
      float cm = (duration * SOUND_SPEED) / 2.0;
      if (cm > 0 && cm <= 400) {
        total += cm;
        validReadings++;
      }
    }
    delay(5);
  }

  return validReadings > 0 ? total / validReadings : 0;
}

void ultrasonicSensor() {
  float distanceCm = getFilteredDistance();

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  if (distanceCm <= 0) return;

  String statusStr = "Normal";

  if (distanceCm <= DANGER_DISTANCE) {
    statusStr = "ALERT: Object <= 10cm!";
    sendDistanceToServer(distanceCm, statusStr, true);

    for (int i = 0; i < NUM_LEDS; i++) digitalWrite(leds[i], HIGH);
    delay(100);
    allOff();
    delay(100);
    return;
  }

  allOff();

  if (distanceCm > MAX_DISTANCE) {
    statusStr = "Out of Range (>50cm)";
    sendDistanceToServer(MAX_DISTANCE, statusStr, false);
    return;
  }

  statusStr = "Normal (" + String(distanceCm, 1) + "cm)";
  sendDistanceToServer(distanceCm, statusStr, false);

  int ledIndex = (int)((distanceCm / MAX_DISTANCE) * NUM_LEDS);
  if (ledIndex >= NUM_LEDS) ledIndex = NUM_LEDS - 1;
  digitalWrite(leds[ledIndex], HIGH);
}

void sendDistanceToServer(float distanceCm, String statusStr, bool forceSend) {
  unsigned long now = millis();
  if (!forceSend && (now - lastPostTime < POST_INTERVAL)) return;
  lastPostTime = now;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.reconnect();
    return;
  }

  HTTPClient http;
  http.setTimeout(3000);

  if (!http.begin(SERVER_URL)) {
    Serial.println("Unable to initialize HTTP request.");
    return;
  }

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // URL-encode the status value so spaces and punctuation are safe in form data.
  statusStr.replace("%", "%25");
  statusStr.replace(" ", "%20");
  statusStr.replace("!", "%21");
  statusStr.replace("(", "%28");
  statusStr.replace(")", "%29");
  statusStr.replace("<", "%3C");
  statusStr.replace(">", "%3E");
  String postData = "distance=" + String(distanceCm, 2) + "&status=" + statusStr;

  Serial.print("Sending HTTP POST: ");
  Serial.println(postData);

  int responseCode = http.POST(postData);
  if (responseCode > 0) {
    Serial.print("HTTP response code: ");
    Serial.println(responseCode);
    Serial.print("Server response: ");
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP request failed: ");
    Serial.println(http.errorToString(responseCode));
  }

  http.end();
}

void leftToRight() {
  allOff();
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(leds[i], HIGH);
    delay(SPEED);
    digitalWrite(leds[i], LOW);
  }
  allOff();
}

void rightToLeft() {
  allOff();
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    digitalWrite(leds[i], HIGH);
    delay(SPEED);
    digitalWrite(leds[i], LOW);
  }
  allOff();
}

void centerToBoth() {
  allOff();
  for (int i = 0; i < 5; i++) {
    digitalWrite(leds[4 - i], HIGH);
    digitalWrite(leds[5 + i], HIGH);
    delay(SPEED);
    digitalWrite(leds[4 - i], LOW);
    digitalWrite(leds[5 + i], LOW);
  }
  allOff();
}
