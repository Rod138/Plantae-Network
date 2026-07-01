#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURACIÓN DE TU RED WI-FI ---
const char* ssid = "Gruñon 2.0";       
const char* password = "Alan1004"; 

BH1750 lightMeter;
WebServer server(80);

const int soilPin = 34; 

// Variables para el temporizador de reconexión Wi-Fi
unsigned long previousMillis = 0;
const long interval = 10000; // Intenta reconectar cada 10 segundos (10000 milisegundos)

void handleRoot() {
  // 1. Leer el sensor de luz
  float lux = lightMeter.readLightLevel(); 
  String lightStatus = "";
  
  if (lux < 0) {
    lightStatus = "<span style='color: red;'>⚠️ Sensor Desconectado o Error</span>";
  } else {
    lightStatus = "<span class='data luz'>" + String(lux) + " lx</span>";
  }
  
  // 2. Leer el sensor de humedad
  int soilValue = analogRead(soilPin);
  String soilStatus = "";
  
  if (soilValue < 100 || soilValue >= 4090) {
    soilStatus = "<span style='color: red;'>⚠️ Sensor Desconectado o Error</span>";
  } else {
    soilStatus = "<span style='color: green;'>Conectado</span> - Lectura: " + String(soilValue);
  }

  // 3. Crear el HTML de la página web
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; background-color: #f4f4f9; }";
  html += ".card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); display: inline-block; margin: 10px; width: 80%; max-width: 300px; vertical-align: top; }";
  html += "h1 { color: #333; }";
  html += ".data { font-size: 30px; font-weight: bold; margin: 10px 0; display: block; }";
  html += ".luz { color: #f39c12; }";
  html += "</style></head><body>";
  
  html += "<h1>Monitor de Sensores ESP-32</h1>";
  
  // Tarjeta de Luz
  html += "<div class='card'>";
  html += "<h3>Luz Ambiental (BH1750)</h3>";
  html += "<p>Estado: " + lightStatus + "</p>";
  html += "</div>";

  // Tarjeta de Humedad
  html += "<div class='card'>";
  html += "<h3>Humedad de Suelo</h3>";
  html += "<p>Estado: " + soilStatus + "</p>";
  if (soilValue >= 100 && soilValue < 4090) {
      html += "<progress value='" + String(soilValue) + "' max='4095' style='width: 100%;'></progress>";
  }
  html += "</div>";

  html += "<p style='margin-top: 30px;'><i>Actualiza la página para leer de nuevo</i></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 

  // Iniciar sensor de luz
  if (lightMeter.begin()) {
    Serial.println("Sensor BH1750 iniciado.");
  } else {
    Serial.println("Error con BH1750.");
  }

  // Configurar la resolución del ADC
  analogReadResolution(12);

  // Conexión Wi-Fi inicial (con límite de intentos)
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int retries = 0;
  // Solo intentará 20 veces (unos 10 segundos) y luego continuará
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n¡Conexión exitosa!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar al inicio. Se intentará en segundo plano.");
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Revisa la conexión a Internet periódicamente sin detener el programa
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.println("Wi-Fi desconectado. Intentando reconectar...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis; // Actualiza el temporizador
  }

  // Solo si está conectado, atiende las peticiones web
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
}
