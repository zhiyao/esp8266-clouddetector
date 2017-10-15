#include <FS.h>
#include <WebSocketsServer.h>
#include "Helper.h"

void setup() {
  Serial.begin(115200);
  Wire.begin(SDAPIN, SCLPIN); // SDA, SCL

  Serial.println("Configuring Sensors");
  mlx.begin();
  bmp.begin();

  delay(1000);
  SPIFFS.begin();
  Serial.println("Configuring Access Point");

  setupWiFi();
  IPAddress myIP = WiFi.localIP();
  Serial.print("AP IP address: "); Serial.println(myIP);

  server.on("/", HTTP_GET, []() {
    handleFileRead("/");
  });

  server.onNotFound([]() {                          // Handle when user requests a file that does not exist
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  webSocket.begin();                                // start webSocket server
  webSocket.onEvent(webSocketEvent);                // callback function

  server.begin();
  Serial.println("HTTP server started");
  yield();
}

void loop() {
  static unsigned long l = 0;                     // only initialized once
  unsigned long t;                                // local var: type declaration at compile time
    
  t = millis();

  if((t - l) > 5000) {                            // update temp every 5 seconds
    readSensors();
    printSensors();
    yield();
    webSocket.sendTXT(socketNumber, "dht,Temperature, " + String(dhtTemperature));
    webSocket.sendTXT(socketNumber, "dht,Humidity, " + String(dhtHumidity));
    webSocket.sendTXT(socketNumber, "mlx,Ambient, " + String(mlxAmbient));
    webSocket.sendTXT(socketNumber, "mlx,Object, " + String(mlxObject));
    webSocket.sendTXT(socketNumber, "bmp,Temperature, " + String(bmpTemperature));
    webSocket.sendTXT(socketNumber, "bmp,Pressure, " + String(bmpPressure));
    webSocket.sendTXT(socketNumber, "bmp,Altitude, " + String(bmpAltitude));
    webSocket.sendTXT(socketNumber, "light,Light, " + String(light));
    webSocket.sendTXT(socketNumber, "sky,Sky, " + String(sky));
    l = t;
    yield();
  }

  server.handleClient();
  webSocket.loop(); 
}
