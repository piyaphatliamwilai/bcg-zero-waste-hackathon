#include <AirQualityClass.h>
#include <Arduino_MKRIoTCarrier.h>
#include <Arduino_MKRIoTCarrier_Buzzer.h>
#include <Arduino_MKRIoTCarrier_Qtouch.h>
#include <Arduino_MKRIoTCarrier_Relay.h>
#include <EnvClass.h>
#include <IMUClass.h>
#include <MKRIoTCarrierDefines.h>
#include <PressureClass.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Keypad.h>
// #include "thingProperties.h"
#include <Servo.h>

char ssid[] = "iot"; // ssid
char pass[] = "12345678"; // password

MKRIoTCarrier carrier;

int status = WL_IDLE_STATUS;
char server[] = "script.google.com"; // google script website
const char* lineNotifyToken = "sU7NCOY2agvrcT1UkFe8BNUyZobNU4Y9F3iDDRVunZG";
const float CARBON_FOOTPRINT_PLASTIC = 85.04;
const float CARBON_FOOTPRINT_ALUMINUM = 96.8;
String currentState = "NONE";
String key = "";
uint32_t red = carrier.leds.Color(100, 0, 0);
uint32_t green = carrier.leds.Color(0, 100, 0);
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, server, 443);
Servo servo;
int current = 100;

//
void setup() {
  pinMode(5, INPUT);
  pinMode(A5, INPUT);
  servo.attach(4);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  updateWifiConnection();
  Serial.println("Connected to WiFi");
  // initProperties();
  // ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  carrier.begin();
  currentState = "READY";
  // setDebugMessageLevel(2);
  // ArduinoCloud.printDebugInfo();
  servo.write(0);
  delay(1000);
}

void loop() {
  updateWifiConnection(); // Always!
  // ArduinoCloud.update();
  if (currentState == "READY") {
    main_code();
  }
  delay(100);
}

void main_code() {
  if (isDetected()) {
    Serial.println("Detected bottle...");
    Serial.println(analogRead(A5));
    delay(3000);
    Serial.println("giving money - plastic");
    giveMoney(2);
    updateGoogleSheet("plastic bottle", CARBON_FOOTPRINT_PLASTIC, 2, current-2);
    current = current - 2;
    updateLineNotify("Someone just used the machine, the waste type are plastic bottle. Carbon footprint of the waste: " + String(CARBON_FOOTPRINT_PLASTIC) + ", Current money in the machine: " + String(current));
    servo.write(0);
    delay(1000);
  } else {
    Serial.println(servo.read());
  }
}

bool isDetected() {
  return analogRead(A5) < 200 && analogRead(A5) > 190;
}

void giveMoney(int amount) {
  if (amount % 2 != 0) {
    amount = amount + 1;
  }
  if (amount > current) {
    updateLineNotify("Can't give the money due to the amount being higher than the current money in the machine!");
  } else {
    for (int i = 0; i < (amount / 2); i++) {
      Serial.println("Servo dispensing money");
      servo.write(90); // Move servo to 90 degrees to dispense
      delay(1000);
      Serial.println("Stopping");
      servo.write(0);  // Return servo to 0 degrees
      delay(1000);     // Add delay to allow the servo to return
    }
  }
}


void updateWifiConnection() {
  if (!(WiFi.status() == WL_CONNECTED)) {
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, pass);
      delay(1000);
      carrier.leds.fill(red, 0, 5);
      carrier.leds.show();
    }
  }
  carrier.leds.fill(green, 0, 5);
  carrier.leds.show();
}

// ไออัท มึงเขียนเหี้ยไรอีกแล้ว
void updateLineNotify(const String& message) {
  if (status == WL_CONNECTED) {
    if (!wifi.connect("notify-api.line.me", 443)) {
      Serial.println("Connection to LINE Notify failed!");
      return;
    }

    // URL encode the message
    String encodedMessage = urlEncode(message);

    // Prepare the POST request
    String url = "/api/notify";
    String payload = "message=" + encodedMessage;
    String request = String("POST ") + url + " HTTP/1.1\r\n" +
                    "Host: " + "notify-api.line.me" + "\r\n" +
                    "Authorization: Bearer " + lineNotifyToken + "\r\n" +
                    "Content-Type: application/x-www-form-urlencoded\r\n" +
                    "Content-Length: " + payload.length() + "\r\n" +
                    "Connection: close\r\n\r\n" +
                    payload;

    // Send the request
    client.print(request);
    Serial.println("Request sent!");

    // Read and print the response
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break; // Headers ended
      Serial.println(line);
    }

    // Close the connection
    client.stop();
    }
}

// ไออัท มึงเขียนเหี้ยไรเนี้ย
void updateGoogleSheet(String type, float carbon, int cost, int current) {
  if (status == WL_CONNECTED) {
    client = HttpClient(wifi, server, 443);
    Serial.println("Attempting the GET request");
    type.replace(" ", "%20");
    String googleSheet = "/macros/s/AKfycbysfAW_F1Bbo3R51llPnMHId5eRpgaQqyP8qOzrcVENurVhx891kUjy4XiBq_2bN4Iy/exec";
    String payload = "type=" + type + "&carbon=" + String(carbon) + "&cost=" + String(cost) + "&current=" + String(current);
    client.beginRequest();
    client.get(googleSheet + "?" + payload);
    client.endRequest();
    int code = client.responseStatusCode();
    Serial.print("Status code: ");
    Serial.println(code);
    // ควย Redirect
    if (code == 302 || code == 303) {
      handleRedirect();
    }
  } else {
    String response = client.responseBody();
    Serial.println("Response:");
    Serial.println(response);
  }
}

void handleRedirect() {
  String location;
  Serial.println("Redirected");
  
  while (client.headerAvailable()) {
    String headerName = client.readHeaderName();
    String headerValue = client.readHeaderValue();
    if (headerName == "Location") {
      location = headerValue;
      break;
    }
  }

  if (location.length() > 0) {
    Serial.print("Redirecting to: ");
    Serial.println(location);

    if (location.startsWith("https://")) {
      location = location.substring(8); // remove "https://"
    }
    
    int pathIndex = location.indexOf('/');
    String newServer = location.substring(0, pathIndex);
    String newPath = location.substring(pathIndex);

    client = HttpClient(wifi, newServer, 443);
    client.beginRequest();
    client.get(newPath);
    client.endRequest();

    Serial.print("Status code: ");
    Serial.println(client.responseStatusCode());

    String response = client.responseBody();
    Serial.println("Response: " + response);
  }
}


String urlEncode(const String &msg) {
  String encodedMsg = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < msg.length(); i++) {
    c = msg.charAt(i);
    if (c == ' ') {
      encodedMsg += '+';
    } else if (isalnum(c)) {
      encodedMsg += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedMsg += '%';
      encodedMsg += code0;
      encodedMsg += code1;
    }
  }
  return encodedMsg;
}

void onCurrentChange() {
  
}
