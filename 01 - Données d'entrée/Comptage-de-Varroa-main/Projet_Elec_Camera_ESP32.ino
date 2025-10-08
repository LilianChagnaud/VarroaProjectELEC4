#include "esp_camera.h"
#include <WiFi.h>
#include "WebServer.h"
#include "esp_http_server.h"
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char* ssid = "Pixel_3581"; //Redmi Note 10 5G //Pixel_3581
const char* password = "1234567891011"; //3ee7230b //12345678910
bool captureEnabled = true; // Indique si la capture d'images est activée

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void startCameraServer();
void setup()
{
  // Désactive le watchdog matériel (pour débogage)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.println("\n===== DÉMARRAGE ESP32-CAM =====");

  // Vérification PSRAM
  if (!psramFound()) {
    Serial.println("ERREUR : PSRAM non détectée !");
    while (1); // Bloque si pas de PSRAM
  }
  else {
    Serial.println("PSRAM détectée.");
  }

  Serial.setDebugOutput(true);
  Serial.println();

  // Instructions pour le contrôle
  Serial.println("Tapez 'start' pour démarrer et 'stop' pour arrêter la capture.");

  // Configuration de la caméra
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound())
  {
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  }
  
  else
  {
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  }

  // Initialisation de la caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
  Serial.printf("Échec d'initialisation de la caméra, erreur 0x%x", err);
  return;
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.printf("Connexion à %s...\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");

  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
  delay(500);
  Serial.print(WiFi.status());
  }

  Serial.println("");
  Serial.println("Connexion Wi-Fi établie");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Démarrage du serveur de caméra
  startCameraServer();
  Serial.println("Serveur caméra démarré");
}
  
void loop() {
    // Vérifiez les commandes dans le Moniteur série
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n'); // Lire jusqu'à la fin de ligne
        command.trim(); // Supprime les espaces et sauts de ligne

        if (command == "start") {
            captureEnabled = true;
            Serial.println("Capture démarrée !");
        } else if (command == "stop") {
            captureEnabled = false;
            Serial.println("Capture arrêtée !");
        } else {
            Serial.println("Commande non reconnue. Tapez 'start' ou 'stop'.");
        }
    }

    // Capture et envoi d'images uniquement si activé
    if (captureEnabled) {
        sendToServer();
        delay(10000); // Temps entre deux captures
    } else {
        delay(100); // Petite pause pour ne pas surcharger la boucle
    }
}

// Définition de la fonction startCameraServer et stream_handler
httpd_handle_t stream_httpd = NULL;
static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Échec de capture de la frame");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  if(fb->format != PIXFORMAT_JPEG)
  {
    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
    if(!jpeg_converted)
    {
      Serial.println("Échec de la compression JPEG");
      esp_camera_fb_return(fb);
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
  } 

  else 
  {
    _jpg_buf_len = fb->len;
    _jpg_buf = fb->buf;
  }

  httpd_resp_set_type(req, "image/jpeg");
  res = httpd_resp_send(req, (const char *)_jpg_buf, _jpg_buf_len);
  if(fb->format != PIXFORMAT_JPEG)
  {
    free(_jpg_buf);
  }
  esp_camera_fb_return(fb);
  return res;
}

void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t stream_uri = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = stream_handler,
  .user_ctx = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void sendToServer()
{
    // Capture de l'image
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Échec de capture de la frame");
        return;
    }

    // Configurer le client HTTP
    HTTPClient http;
    http.begin("http://10.212.100.215:5000/upload"); // Remplacez par l'URL de votre serveur
    http.addHeader("Content-Type", "image/jpeg");

    // Envoyer l'image via HTTP POST
    int httpResponseCode = http.POST(fb->buf, fb->len);
    if (httpResponseCode > 0) {
        Serial.printf("Réponse serveur : %d\n", httpResponseCode);
    } else {
        Serial.printf("Erreur HTTP : %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    esp_camera_fb_return(fb); // Libérer la mémoire
}

