#include <Wire.h>                // Bibliothèque pour I2C
#include "esp_camera.h"
#include <WiFi.h>
#include "WebServer.h"
#include "esp_http_server.h"
#include <HTTPClient.h>
#include "HuskyLens.h"           // Bibliothèque pour HuskyLens

// Pins pour Heltec WiFi LoRa 32 (ajuster selon la configuration)
#define SDA_PIN 21
#define SCL_PIN 22

const char* ssid = "Pixel_3581";
const char* password = "1234567891011";

bool captureEnabled = true; // Indique si la capture d'images est activée

// Initialisation de HuskyLens via I2C
HuskyLens huskyLens;

void setup() {
  Serial.begin(115200);
  Serial.println("\n===== DÉMARRAGE ESP32 =====");
  
  // Initialisation I2C
  Wire.begin(SDA_PIN, SCL_PIN); // Configure l'I2C avec les bonnes pins
  huskyLens.begin(SDA_PIN, SCL_PIN);  // Initialisation de HuskyLens
  
  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnexion Wi-Fi établie");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Initialisation de la caméra
  camera_config_t config;
  // Configuration de la caméra (garde ta configuration d'origine pour l'ESP32)
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erreur d'initialisation de la caméra : 0x%x", err);
    return;
  }
  
  // Lancement du serveur pour diffuser les images
  startCameraServer();
  Serial.println("Serveur caméra démarré");
}

void loop() {
  // Lire et traiter les images via la caméra
  if (captureEnabled) {
    sendToServer();
    delay(10000); // Capture toutes les 10 secondes
  }

  // Traitement des données HuskyLens
  if (huskyLens.request()) {
    Serial.println("Données HuskyLens reçues");
    // Ajouter ton code pour traiter les données de HuskyLens ici
  }
}

void sendToServer() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Erreur de capture de l'image");
    return;
  }

  HTTPClient http;
  http.begin("http://10.212.100.215:5000/upload"); // Remplace par l'URL de ton serveur
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  if (httpResponseCode > 0) {
    Serial.printf("Réponse serveur : %d\n", httpResponseCode);
  } else {
    Serial.printf("Erreur HTTP : %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  esp_camera_fb_return(fb); // Libération de la mémoire
}

httpd_handle_t stream_httpd = NULL;
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Erreur de capture");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  
  size_t _jpg_buf_len = fb->len;
  uint8_t * _jpg_buf = fb->buf;
  
  httpd_resp_set_type(req, "image/jpeg");
  esp_err_t res = httpd_resp_send(req, (const char *)_jpg_buf, _jpg_buf_len);
  esp_camera_fb_return(fb);
  return res;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t stream_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}