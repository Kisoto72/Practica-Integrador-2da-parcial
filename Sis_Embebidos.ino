#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "secrets.h"  // Aseguramos que las credenciales sean importadas desde secrets.h

// Configuración de sensores y pantalla OLED
#define DHTPIN 4
#define DHTTYPE DHT22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BMP_ADDRESS 0x76
#define OLED_RESET    -1
#define THINGSPEAK_SERVER "http://api.thingspeak.com/update"

char ssid[] = SECRET_SSID;   // Nombre de la red WiFi
char pass[] = SECRET_PASS;   // Contraseña WiFi
String apiKey = SECRET_WRITE_APIKEY;  // Clave API de ThingSpeak
WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Conectar a WiFi
  conectarWiFi();

  // Inicializar sensores
  dht.begin();
  if (!bmp.begin(BMP_ADDRESS)) {
    Serial.println("Error: No se encontró el BMP280.");
    while (1);
  }

  // Inicializar pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error: Fallo al inicializar OLED.");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure() / 100.0F; // Convertir a hPa

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer DHT22.");
    return;
  }

  // Mostrar datos en OLED
  mostrarEnPantalla(temperature, humidity, pressure);

  // Enviar datos a ThingSpeak
  enviarThingSpeak(temperature, humidity, pressure);

  delay(20000); // Espera 20 segundos
}

// Función para conectar a WiFi
void conectarWiFi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, pass);  // Corregimos el error aquí

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado.");
  } else {
    Serial.println("\nError: No se pudo conectar a WiFi.");
  }
}

// Función para mostrar datos en pantalla OLED
void mostrarEnPantalla(float temp, float hum, float pres) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.print(" C");
  display.setCursor(0, 10);
  display.print("Hum: ");
  display.print(hum);
  display.print(" %");
  display.setCursor(0, 20);
  display.print("Pres: ");
  display.print(pres);
  display.print(" hPa");
  display.display();
}

// Función para enviar datos a ThingSpeak
void enviarThingSpeak(float temp, float hum, float pres) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi desconectado. Intentando reconectar...");
    conectarWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(THINGSPEAK_SERVER) + "?api_key=" + apiKey +
                 "&field1=" + String(temp) +
                 "&field2=" + String(hum) +
                 "&field3=" + String(pres);

    Serial.println("Enviando datos a ThingSpeak...");
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.print("Respuesta HTTP: ");
      Serial.println(httpCode);
      if (httpCode == 200) {
        Serial.println("Datos enviados correctamente.");
      } else {
        Serial.println("Error en ThingSpeak.");
      }
    } else {
      Serial.println("Error al conectar con ThingSpeak.");
    }

    http.end();
  }
}