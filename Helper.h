
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define SDAPIN D5
#define SCLPIN D4
#define DHTTYPE DHT22   // There are multiple kinds of DHT sensors

//https://indiduino.wordpress.com/2013/02/02/meteostation/
#define K1 33
#define K2 0
#define K3 4
#define K4 100
#define K5 100
#define CLOUD_THRESHOLD 30
#define CLOUD_TEMP_CLEAR -8
#define CLOUD_TEMP_OVERCAST 0

float Tcorrection() {
  return  (K1 / 100) * (CLOUD_THRESHOLD - K2 / 10) + (K3 / 100) * pow((exp (K4 / 1000 * CLOUD_THRESHOLD)) , (K5 / 100));
}

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

const char*         WIFI_PASS = "password";        // "put your password here before compiling"
const char*         WIFI_SSID = "ssid";
uint8_t            socketNumber;

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

float mlxAmbient = 0.0;
float mlxObject = 0.0;
float dhtHumidity = 0.0;
float dhtTemperature = 0.0;
float bmpTemperature = 0.0;
float bmpPressure = 0.0;
float bmpAltitude = 0.0;
float light = 0;
float sky = 0;

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) { 
    case WStype_DISCONNECTED:
      // Reset the control for sending samples of ADC to idle to allow for web server to respond.
      Serial.printf("[%u] Disconnected!\n", num);
      yield();
      break;
    case WStype_CONNECTED: {                  // Braces required http://stackoverflow.com/questions/5685471/error-jump-to-case-label
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      yield();
      socketNumber = num;
      break;
      }
    case WStype_TEXT:
      if (payload[0] == '#') {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        yield();
      }
      break;
    case WStype_ERROR:
      Serial.printf("Error [%u] , %s\n", num, payload);
      yield();
  }
}

float temperatureSky(float mlxIR, float ambienceTemp) {
  return (mlxIR - ambienceTemp) - Tcorrection();
}

void readSensors() {
  float temp;
  temp = mlx.readAmbientTempC();
  if(temp) {
    mlxAmbient = temp;
  }
  temp = mlx.readObjectTempC();
  if(temp) {
    mlxObject = temp;
  }

  temp = dht.readHumidity();
  if(!isnan(temp)) {
    dhtHumidity = temp;
  }
  temp = dht.readTemperature();
  if(!isnan(temp)) {
    dhtTemperature = temp;
  }
  
  bmpTemperature = bmp.readTemperature();
  bmpPressure = bmp.readPressure();
  bmpAltitude = bmp.readAltitude();
  
  light = analogRead(0);
  sky = temperatureSky(mlxObject, bmpTemperature);
}

void printSensors() { 
  Serial.print("MLX Ambient = "); Serial.print(mlxAmbient);
  Serial.print("*C\tObject = "); Serial.print(mlxObject); Serial.println("*C");

  Serial.print("DHT humidity = "); Serial.print(dhtHumidity);
  Serial.print("\tTemperature = "); Serial.print(dhtTemperature); Serial.println("*C");
  
  Serial.print("BMP Temperature = ");
  Serial.print(bmpTemperature);
  Serial.print(" *C");

  Serial.print("\tPressure = ");
  Serial.print(bmpPressure);
  Serial.print(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("\tAltitude = ");
  Serial.print(bmpAltitude);
  Serial.println(" meters");

//  Serial.print("Pressure at sealevel (calculated) = ");
//  Serial.print(bmp.readSealevelPressure());
//  Serial.println(" Pa");

  Serial.print("Light = "); Serial.println(light);
  Serial.print("Sky = "); Serial.println(sky);
  Serial.print("Tcorrection = "); Serial.println(Tcorrection());
}

String getContentType(String filename) {
  yield();
  if (server.hasArg("download"))      return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html"))return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js"))  return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz"))  return "application/x-gzip";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

void setupWiFi()
{
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (WiFi.status() != WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please check SSID and PASSWORD");
    }
  }
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);

  if (path.endsWith("/")) {
    path += "counter.html";
  }

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  Serial.println("PathFile: " + pathWithGz);

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  yield();
  return false;
}

