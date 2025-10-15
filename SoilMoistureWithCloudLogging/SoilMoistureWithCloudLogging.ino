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
const char *ssid = "BePositive";
const char *password = "Positive24";

// AWS IoT
const char *awsEndpoint = "acmu50w6950e9-ats.iot.eu-north-1.amazonaws.com";
const int awsPort = 8883;
const char *deviceId = "esp8266-test-1";
const char *topic = "soil/readings";

// Certificates
static const char rootCA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char clientCert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUQjV0thEmofVH+PwQ9hEWhgPmHD0wDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTAxNTEwMTUz
M1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALC1gco1SMuXyyYcJIen
1jZbCn5Pcu3wV+SGgR1wlVEeBaOEL9VYt8ysB3XGOZJb05MteCgQeOjUjJWvKj0W
JUdvPGUb1jx1749Y98vLZs0pleJw7svQa2c7YIF4jVi2mMA+sRTxSfXGxU6OkNTB
g5e6C5v8vej917QN16/dKvhKIkqcKGBJ7Rb9t1Ye4Uq7dMqBXmz608j50yXcHKMD
Of5C3seqjIEtv0yA+EBAoX/WMnFygRsbAp41Xj4t/rJuf6JCN5YYnkLjLZZWzots
NiiHbinaG9p+hFIfks/01dxmenEQcXE1OOKkVfTbI+Rz/UB7jzNOb4WsR7GGjMf8
LwUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUtfySQ8Ya0PloFxbT2mI/eveRY+MwHQYD
VR0OBBYEFI8D3gefDdAGevAUyy5mY5eRaJLfMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQA/LOXIMqBUMAVzrImwhAp3JJH1
onKvdg0uI+AdH1uvbGl59Z2r2XuhE+TJfy/70dcUm4IGgY7DTyRruo6uf0E6UzZC
lWYUzzoLkqsDxu1RfNs6bWAK1I8LpOigxpyzXuLgrPAFQLIbcdeMVh811DOXfgFi
ijsS+7qyqX0EUYdlwOgEU6tF+ox0n14Q3aw7it0RS+dlvLt2AXYUpk9XYCeQEdKw
BYATChYQHhzJEpEgJHxFD/H018OzaLQ7T6b0NOQyUacu40iOBD7ouplqg74Hqh/Z
UL0uduP1s7OdK2GRiAbl2qyeYkiVtRHd8pGMbr9qUj5X9HRWJ3meI7OcL+ck
-----END CERTIFICATE-----
)KEY";

static const char clientKey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAsLWByjVIy5fLJhwkh6fWNlsKfk9y7fBX5IaBHXCVUR4Fo4Qv
1Vi3zKwHdcY5klvTky14KBB46NSMla8qPRYlR288ZRvWPHXvj1j3y8tmzSmV4nDu
y9BrZztggXiNWLaYwD6xFPFJ9cbFTo6Q1MGDl7oLm/y96P3XtA3Xr90q+EoiSpwo
YEntFv23Vh7hSrt0yoFebPrTyPnTJdwcowM5/kLex6qMgS2/TID4QEChf9YycXKB
GxsCnjVePi3+sm5/okI3lhieQuMtllbOi2w2KIduKdob2n6EUh+Sz/TV3GZ6cRBx
cTU44qRV9Nsj5HP9QHuPM05vhaxHsYaMx/wvBQIDAQABAoIBAQCgS+ZywadJWGcm
KP1SOYPuRuPb9MIy6YUxfnn9dLVeojdHP5AMnrAMcDlMMwbip33j0qle3qJvwu/M
kNtmEoK6rENDDN9odStdA721FswobALVKNDV/jiPqqM+j1bZFLq4GaaVctQ+qZMu
UdE7zldK+a+yTrEPe8WCRdRUafKiy0pruiIJV+KbPY/JApZPlp06rovAE+Hf/C0m
GtNMJgQjyylUztUj2UFsKwL+85xbIPZjV4w0VARxdDUZWCqoBpnJ5SKTQtlzXj7e
lTAR0iFjhakG/gszYobADH8im2EKhNVpAjsuA9UD+8697rdee5PLLCkgALle8Qv/
GGm95MkBAoGBAOKjX/aZIsfuKqXtYdruqrdvST8rlydRBO90EuVClZ3o1W3OkNba
gxowVhpKgTQtvr7Ya+xu8I1FqU3fWQz8dZjKvYOnLpREWzeO8D2yiiOTzYMi0FiT
EJcHBArHUBQJiiASFHIAUxSz9JSj9qvIGPSbdUu7rg8tbq3V6AhWtEflAoGBAMea
MZo0PreZCrC3c/hE6V6Slqpz+ImDDIrf9N/hXZS8/bNkHavj8PVwcgIQEqi5mM9g
pDIu2zI9xMycZ3cCx5+BFOEGg6ug6rv4T8rJSQ/GXOcxNTX/PhFijaxmDwrzcz2O
FiKhMaMIjnLF6x7xUxeRBLHgQfijFpDtJLgJj5ihAoGAYzKZFSlv1MyE9D1ToGzo
YgOerryYm0fV/izy/E/JhuVbTYWlVbUsvvC7LyS/KqR/Ia8+ha0FMwBzmzcQFiQG
IMtUaeRp+dF1j3rL4mVcGzhdMnGxeaNdFo2mkkUs4qhOQb1IH6HOu3gMrGab2BJs
AdxtWambBN+g2wuOVOAGZpUCgYAuKa4xIMYkhiOfzT3n/GAQvExWLa4cd4+OCs/C
e4e+H5t5Qr1OBNC0owXrDjaSiSIbTBmNdaLCUuDSBZY0QxNSA3OFu5D2P6gqQvqJ
RtY77L+NxKauz1JWiPmFFrGB8d4ELXlXFvq43wC68KU5Kczy1VpFnT0EuQs+B2qH
mcZ5IQKBgQDIduhJxX7FNFfKWg7NmyBatqcYf5sZ/BPHSwMucSYlhbk1KFGLmhCN
zY3auD+f+P4OMHZJs9KLgO+4HGDs7NPONDsoGJzYl+NkP+cnaRTwLAPRP6P8j2C9
2EIPMQMPwhA8wbI1r9EMgVnrMhqqCqd63kwULaF7MuhWQRf+O2VC7w==
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
