#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SOIL_PIN A0 // Analog pin
int soilValue = 0;
int moisturePercent = 0;

// Values of trail and error
int dryValue = 1024; // sensor value in dry soil
int wetValue = 400;  // sensor value in wet soil

LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Credentials
const char *ssid = "ENTER_YOUR_SSID";
const char *password = "ENTER_YOUR_PASSWORD";

// AWS IoT
const char *awsEndpoint = "ENTER YOUR AWS ENDPOINT";
const int awsPort = 8883; // DEFAULT PORT FOR MQTT PROTOCOL
const char *deviceId = "esp8266-test-1";
const char *topic = "ENTER YOUR TOPIC (Eg: soil/logging)";

// Certificates
static const char rootCA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
ADD CERTIFICATE HERE
-----END CERTIFICATE-----
)EOF";

static const char clientCert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
ADD CERTIFICATE HERE
-----END CERTIFICATE-----
)KEY";

static const char clientKey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
ADD CERTIFICATE HERE
-----END RSA PRIVATE KEY-----
)KEY";

// Create BearSSL objects for certs
static BearSSL::X509List caCert(rootCA);
static BearSSL::X509List clientCertificate(clientCert);
static BearSSL::PrivateKey privateKey(clientKey);

// Global BearSSL client
BearSSL::WiFiClientSecure net;
PubSubClient client(net);

// --------------------------------------------------------------------
void connectWiFi()
{
  Serial.print("Connecting to WiFi: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// --------------------------------------------------------------------
void connectAWS()
{
  // code to sync the clock
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1609459200)
  {
    delay(100);
  }

  // Attach certificates (ESP8266 style)
  net.setTrustAnchors(&caCert);
  net.setClientRSACert(&clientCertificate, &privateKey);

  client.setServer(awsEndpoint, awsPort);

  while (!client.connected())
  {
    Serial.print("Connecting to AWS IoT...");
    if (client.connect(deviceId))
    {
      Serial.println("Connected!");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// --------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.begin(115200);
  Serial.println("Soil Moisture Sensor Test");
  lcd.init();      // Initialize LCD
  lcd.backlight(); // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Hello, ESP8266!");
  lcd.setCursor(0, 1);
  lcd.print("I2C LCD Working!");

  connectWiFi();
  connectAWS();

  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Raw: ");

  delay(100);

  lcd.setCursor(0, 1);
  lcd.print("Moisture: ");
  delay(2000);
}

void loop()
{

  soilValue = analogRead(SOIL_PIN);
  // Map raw value to percentage
  moisturePercent = map(soilValue, dryValue, wetValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Raw: ");
  Serial.print(soilValue);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  lcd.setCursor(0, 0);
  lcd.print("Raw: ");
  lcd.print(soilValue);

  delay(100);

  lcd.setCursor(0, 1);
  if (moisturePercent < 10)
    lcd.print("0");

  lcd.print("Moisture: ");
  lcd.print(moisturePercent);

  if (!client.connected())
    connectAWS();
  client.loop();

  String payload = "{\"deviceId\":\"esp8266-test-1\",\"moisture\":\"";
  payload += String(moisturePercent);
  payload += "\",\"raw\":\"";
  payload += String(soilValue);
  payload += "\"}";

  Serial.println(payload);

  if (client.publish(topic, payload.c_str()))
  {
    Serial.println("Published: " + payload);
  }
  else
  {
    Serial.println("Publish failed!");
  }
  delay(5000);
}
