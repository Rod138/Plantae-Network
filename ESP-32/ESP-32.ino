#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h> 
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Gruñon 2.0";       
const char* password = "Alan1004"; 

BH1750 lightMeter;
Adafruit_BMP280 bmp; 
WebServer server(80);

const int soilPin = 34; 

unsigned long previousMillis = 0;
const long interval = 10000; 

bool bmpStatus = false;

void handleRoot() {
  float lux = lightMeter.readLightLevel(); 
  String lightHtml = (lux < 0) ? "<span style='color: red;'>⚠️ Error/Desconectado</span>" : "<span class='data luz'>" + String(lux) + " lx</span>";
  
  int soilValue = analogRead(soilPin);
  String soilHtml = (soilValue < 100 || soilValue >= 4090) ? "<span style='color: red;'>⚠️ Error/Desconectado</span>" : "<span style='color: green;'>Conectado</span><br>Lectura: " + String(soilValue);

  String tempHtml = "<span style='color: red;'>⚠️ Error</span>";
  String presHtml = "<span style='color: red;'>⚠️ Error</span>";
  
  if (bmpStatus) {
    float temp = bmp.readTemperature();
    float pres = bmp.readPressure() / 100.0F; 
    
    if (isnan(temp) || isnan(pres)) {
      tempHtml = "<span style='color: red;'>⚠️ Fallo de lectura</span>";
      presHtml = "<span style='color: red;'>⚠️ Fallo de lectura</span>";
    } else {
      tempHtml = "<span class='data temp'>" + String(temp) + " °C</span>";
      presHtml = "<span class='data pres'>" + String(pres) + " hPa</span>";
    }
  }

  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; background-color: #eef2f3; margin: 0; padding: 20px; }";
  html += ".container { display: flex; flex-wrap: wrap; justify-content: center; max-width: 800px; margin: auto; }";
  html += ".card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 6px 15px rgba(0,0,0,0.1); margin: 10px; width: 100%; max-width: 320px; box-sizing: border-box; }";
  html += "h1 { color: #2c3e50; margin-bottom: 30px; }";
  html += "h3 { color: #7f8c8d; margin-top: 0; }";
  html += ".data { font-size: 32px; font-weight: bold; margin: 15px 0; display: block; }";
  html += ".luz { color: #f1c40f; } .temp { color: #e74c3c; } .pres { color: #16a085; }";
  html += "</style></head><body>";
  
  html += "<h1>Sistema de Monitoreo Ambiental</h1>";
  html += "<div class='container'>";
  
  html += "<div class='card'><h3>Ambiente (BMP280)</h3>";
  html += "<p>Temperatura:</p>" + tempHtml;
  html += "<p>Presión Atmosférica:</p>" + presHtml;
  html += "</div>";

  html += "<div class='card'><h3>Luz Solar (BH1750)</h3>";
  html += "<p>Intensidad:</p>" + lightHtml;
  html += "</div>";

  html += "<div class='card'><h3>Tierra (Pin 34)</h3>";
  html += "<p>Estado: " + soilHtml + "</p>";
  if (soilValue >= 100 && soilValue < 4090) {
      html += "<progress value='" + String(soilValue) + "' max='4095' style='width: 100%; height: 20px;'></progress>";
  }
  html += "</div>";

  html += "</div>";
  html += "<p style='color: #95a5a6; margin-top: 30px;'><i>Actualiza la página para obtener nuevas lecturas</i></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 

  if (lightMeter.begin()) {
    Serial.println("BH1750 OK.");
  } else {
    Serial.println("Error BH1750.");
  }

  if (bmp.begin(0x76)) {
    Serial.println("BMP280 OK.");
    bmpStatus = true;
  } else {
    Serial.println("No se encontró el sensor BMP280. Revisa el código.");
    bmpStatus = false;
  }

  analogReadResolution(12);

  Serial.print("Conectando a Wi-Fi.");
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500); Serial.print("."); retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nIP del ESP-32: ");
    Serial.println(WiFi.localIP());
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.println("Intentando reconectar Wi-Fi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
}
