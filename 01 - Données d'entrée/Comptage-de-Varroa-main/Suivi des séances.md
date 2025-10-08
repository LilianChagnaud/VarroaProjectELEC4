### 1ère séance

Au cours de la première séance, nous avons choisi de travailler sur le projet "Comptage de Varroa".
Nous avons entamé la rédaction du cahier des charges pour définir les éléments essentiels du projet.

Afin de visualiser les varroas sur une feuille, nous avons a notre disposition l'ESP32-CAM.
Nous avons alors tenté de visualiser une photo capturée par la caméra de l’ESP32-CAM.
Pour cela, nous avons utilisé le logiciel Arduino IDE pour nous connecter à l’ESP32-CAM.
Nous avons installé Arduino IDE afin de configurer et accéder à la caméra.
Un programme Arduino a été développé pour permettre cette interaction.
La caméra intègre un serveur web, ce qui nous permet de visualiser les photos directement en ligne grâce à l'adresse IP.

Voici le code pour visualiser les photos sur un serveur web : 


```c++
#include "esp_camera.h"
#include <WiFi.h>
#include "WebServer.h"
#include "esp_http_server.h" 

const char* ssid = "Pixel_3581";
const char* password = "12345678910";

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
void setup() {
Serial.begin(115200);
Serial.setDebugOutput(true);
Serial.println(); 

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

if(psramFound()){
	config.frame_size = FRAMESIZE_UXGA;
	config.jpeg_quality = 10;
	config.fb_count = 2;
} else {
	config.frame_size = FRAMESIZE_SVGA;
	config.jpeg_quality = 12;
	config.fb_count = 1;
}

// Initialisation de la caméra
esp_err_t err = esp_camera_init(&config);
if (err != ESP_OK) {
	Serial.printf("Échec d'initialisation de la caméra, erreur 0x%x", err);
return;
}

// Connexion Wi-Fi
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
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
	delay(10000);
}

// Définition de la fonction startCameraServer et stream_handler
httpd_handle_t stream_httpd = NULL;
static esp_err_t stream_handler(httpd_req_t *req){
camera_fb_t * fb = NULL;
esp_err_t res = ESP_OK;
size_t _jpg_buf_len = 0;
uint8_t * _jpg_buf = NULL;
fb = esp_camera_fb_get();

if (!fb) {
	Serial.println("Échec de capture de la frame");
	httpd_resp_send_500(req);
	return ESP_FAIL;
}

if(fb->format != PIXFORMAT_JPEG){
	bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
if(!jpeg_converted){
	Serial.println("Échec de la compression JPEG");
	esp_camera_fb_return(fb);
	httpd_resp_send_500(req);
	return ESP_FAIL;
}

} else {
	_jpg_buf_len = fb->len;
	_jpg_buf = fb->buf;
}

httpd_resp_set_type(req, "image/jpeg");
res = httpd_resp_send(req, (const char *)_jpg_buf, _jpg_buf_len);
if(fb->format != PIXFORMAT_JPEG){
	free(_jpg_buf);
}
	esp_camera_fb_return(fb);
	return res;
}

void startCameraServer(){
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
```


### 2ème séance

Au cours de la deuxième séance, nous avons tenté de faire fonctionner le moteur.
Celui-ci est contrôlé par un microcontrôleur LoRa WiFi et un driver TMC2226.

Nous avons effectué le câblage nécessaire pour assurer la commande du moteur :

-------------------------------------------------------------------------------------------------------------

![Câblage Moteur](https://github.com/user-attachments/assets/22e9685b-adc7-4d2a-897c-e988c143a9de)

-------------------------------------------------------------------------------------------------------------

Ainsi que le programme Arduino :

```c++
const int Pas = 18;
const int Dir = 19;

void setup() {
	Serial.begin(115200);
	pinMode(Pas, OUTPUT);
	pinMode(Dir, OUTPUT);
	digitalWrite(Dir, HIGH); // Vous pouvez changer à LOW si vous voulez que le moteur tourne dans l'autre sens
}
void loop() {
	for(int x=0; x<200; x++){
	digitalWrite(Pas, HIGH);
	delayMicroseconds(500); // Ajustez le délai pour contrôler la vitesse
	digitalWrite(Pas, LOW);
	delay(4);
}
delay(1000);
}
```

### 3ème séance

Au cours de la troisième séance, nous travaillons toujours sur le fonctionnement du moteur.
Après avoir constaté un problème avec le driver initial, nous l’avons remplacé par un autre.
Nous avons également opté pour un moteur 12V afin de poursuivre nos essais.
Des tests ont été réalisés avec le driver A4988, mais ils se sont révélés infructueux.
Nous sommes donc revenus à l’utilisation du TMC2226 pour continuer nos essais.

Voici le code pour faire tourner le moteur dans les deux sens :

```c++ 
const int Pas = 3;
const int Dir = 2;
void setup() {
  Serial.begin(115200);
  pinMode(Pas, OUTPUT);
  pinMode(Dir, OUTPUT);
  digitalWrite(Dir, HIGH);
}
void loop() {

  for (int x = 200; x > 0; x--) {
    Serial.println(x);
    digitalWrite(Dir, LOW);
    digitalWrite(Pas, HIGH);
    delayMicroseconds(500);
    digitalWrite(Pas, LOW);
    delay(4);}
 
  for (int x = 0; x < 200; x++) {
    Serial.println(x);    
    digitalWrite(Dir, HIGH);
    digitalWrite(Pas, HIGH);
    delayMicroseconds(500);
    digitalWrite(Pas, LOW);
    delay(4);
  }
  delay(2000);
}
```


### 4ème séance

Après de nombreux essais sur les moteurs, il semble que le problème provienne des alimentations plutôt que du câblage ou du programme.
Le professeur nous a alors suggéré d’utiliser une alimentation 12V via une prise secteur adaptée.

Nous avons ensuite déterminé la hauteur optimale pour positionner la caméra afin d'obtenir une image nette et claire. Cette hauteur est de 24 cm. 

-------------------------------------------------------------------------------------------------------------

<img width="1284" alt="Capture d’écran 2024-11-25 à 16 32 52" src="https://github.com/user-attachments/assets/13253b82-e238-4c2b-a853-1c3b6a13595d">

-------------------------------------------------------------------------------------------------------------

Nous utiliserons cette référence pour créer notre programme 3D.
Pour visualiser le projet, j'ai commencé à créer la conception assistée par ordinateur (CAO) :

-------------------------------------------------------------------------------------------------------------

<img width="1512" alt="Capture d’écran 2024-11-25 à 16 59 09" src="https://github.com/user-attachments/assets/91f5ed5d-f52d-4513-9c9a-bc1230af840b">

-------------------------------------------------------------------------------------------------------------

### 5ème séance

Au cours de ma cinquième séance de travail, ma priorité était la conception d'un système mécanique de déplacement de caméra en 3D, en utilisant le logiciel de conception assistée par ordinateur Fusion 360. Cela nous permettra d'avoir une vision global du projet, afin de réaliser la partie mécanique de celui-ci.

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-11-26 à 11 46 03" src="https://github.com/user-attachments/assets/042539e9-b1c1-40d1-8db9-2a3a880f74f4">

-------------------------------------------------------------------------------------------------------------

Le dispositif mécanique que j'ai développé comprend plusieurs éléments :

- Un moteur pas à pas connecté à un axe cylindrique via un coupleur
- Un roulement intégré sur l'axe pour garantir une rotation stable et guidée
- Une transmission du mouvement rotatif permettant le déplacement de la caméra
- Une structure verticale composée de montants supportant l'axe

J'ai stratégiquement positionné les axes à une hauteur de 24 cm, ce qui permet :

- De surélever la caméra au maximum
- D'améliorer la qualité de l'image
- D'élargir significativement le champ de vision

L'axe traversant la structure servira à la fois de guide et de support pour le système de déplacement.

-------------------------------------------------------------------------------------------------------------

<img width="1098" alt="Capture d’écran 2024-12-08 à 14 12 38" src="https://github.com/user-attachments/assets/9078dcff-6cdc-4f95-ae4a-07eb45baa81e">

-------------------------------------------------------------------------------------------------------------

Le dispositif mécanique que j'ai développé comprend plusieurs éléments :

- Un moteur pas à pas connecté à un axe cylindrique via un coupleur
- Un roulement intégré sur l'axe pour garantir une rotation stable et guidée
- Une transmission du mouvement rotatif permettant le déplacement de la caméra
- Une structure verticale composée de montants supportant l'axe

J'ai stratégiquement positionné les axes à une hauteur de 24 cm, ce qui permet :

- De surélever la caméra au maximum
- D'améliorer la qualité de l'image
- D'élargir significativement le champ de vision

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-12-08 à 14 14 22" src="https://github.com/user-attachments/assets/a04da049-09d6-4142-9ac6-648da85117ad">

-------------------------------------------------------------------------------------------------------------

L'axe traversant la structure servira à la fois de guide et de support pour le système de déplacement.

-------------------------------------------------------------------------------------------------------------

<img width="1098" alt="Capture d’écran 2024-12-08 à 14 06 52" src="https://github.com/user-attachments/assets/1f9978af-4bd5-4b02-8ba3-be34ff1f9949">

-------------------------------------------------------------------------------------------------------------

Ici, nous observons la section où se situe le support de la caméra. Elle comprend deux supports montés sur des roulements à billes, permettant un mouvement horizontal, ainsi qu’un support également monté sur des roulements à billes, offrant un mouvement vertical.

-------------------------------------------------------------------------------------------------------------

<img width="1098" alt="Capture d’écran 2024-12-08 à 14 09 39" src="https://github.com/user-attachments/assets/401ec712-e62d-4517-95d0-a71d7d78e5ae">

-------------------------------------------------------------------------------------------------------------

Suite aux retours de Monsieur Peter et d’un professeur de robotique concernant ma modélisation 3D, plusieurs points d’amélioration ont été soulevés :
- Remplacer le roulement cranté par un roulement à billes.
- Positionner les moteurs du même côté pour faciliter le cheminement des câbles.
- Supprimer le roulement au niveau de la plateforme supportant la caméra et le remplacer par un système de pincement au support.
- Ajouter des guides pour le câblage.
- Réduire l’épaisseur de la plateforme afin de limiter l’inertie et alléger l’ensemble.
- Concevoir des blocs empilables pour les pylônes.
- Remplacer les pièces de glissement des plateformes par des pièces usinées.
- Intégrer des fins de course.


**Remplacer le roulement cranté par un roulement à billes :**

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-12-09 à 11 58 18" src="https://github.com/user-attachments/assets/b09996f9-6f3c-4576-a0e6-12ce2f2dbc6f">

-------------------------------------------------------------------------------------------------------------

**Moteurs du même coté :**

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-12-09 à 11 55 22" src="https://github.com/user-attachments/assets/ba12f487-5c40-4efe-adbe-c18c4c36c204">

-------------------------------------------------------------------------------------------------------------

**Système de pincement :**

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-12-09 à 11 59 41" src="https://github.com/user-attachments/assets/12bee5cc-a3b5-4c93-95fb-afcfd39aa751">

-------------------------------------------------------------------------------------------------------------

**Ajout capteur de fin de course :**

-------------------------------------------------------------------------------------------------------------

<img width="1112" alt="Capture d’écran 2024-12-09 à 15 05 34" src="https://github.com/user-attachments/assets/32e87ccf-8936-484f-83ec-ca61b9b9ad9c">

-------------------------------------------------------------------------------------------------------------


### 6ème séances

Lors de la 6ᵉ séance, j'ai pris les mesures exactes nécessaires pour modéliser les pièces en 3D en vue de leur impression. Pour le moment, les pièces imprimées serviront de prototypes afin de valider les dimensions.


#### Côtes

- [i] Flexible couplings moteur : 18,50 mm de diamètre

- [i] Moteur :
	- [>] Axe moteur : 5 mm
	- [>] Largeur moteur : 42 mm
	- [>] Longueur moteur : 39,5 mm

- [i] Axes profilés :
	- [>] Diamètre 8 mm

- [i] Roulement :
	- [>] Diamètre de 22 mm 

- [i] Support Axes :
	- [>] Longueur : 30 mm
	- [>] Hauteur : 22 mm
	- [>] Largeur : 33,9 mm

- [?] Prévoir support pour fixer le moteur  

#### Impression

- [x] Pylone à gauche du moteur
- [ ] Pylone à droite du moteur
- [ ] Pylone en parallèle du moteur
- [ ] Pylone moteur
- [ ] Support axe moteur 
- [ ] Support axe
- [ ] Support camera


### 7ème séances

Lors de la 7ᵉ séance, j’ai imprimé les prototypes afin de valider les pièces et de pouvoir enfin lancer les impressions finales pour assembler l’ensemble du projet. Les prototypes se sont avérés concluants.

![IMG_4887](https://github.com/user-attachments/assets/71a04db4-c5e8-4b8f-8355-05f6252ddd41)


![IMG_4885](https://github.com/user-attachments/assets/e856bd61-c8a8-4f56-975c-74a16bc11331)


![IMG_4886](https://github.com/user-attachments/assets/8b01794c-79df-4b08-9dc3-edfe73db34fe)

 
### 8ème séances

Lors de cette séance, j’ai assemblé les prototypes afin de préparer l’impression des pièces finales. J’ai également programmé la gestion des moteurs, en intégrant des capteurs de fin de course et un bouton poussoir permettant d’arrêter le système à tout moment. Après quelques ajustements, j’ai modifié les prototypes pour ajouter les emplacements nécessaires aux capteurs.

Images/Capture d’écran 2025-04-26 à 15.20.42.png

Images/Capture d’écran 2025-04-26 à 15.21.03.png

### 9ème séances

Pendant la 9ᵉ séance, avec Jack, nous avons découpé la plaque de bois qui servira de support pour le système. Nous avons positionné l’ensemble sur la plaque et percé les quatre coins pour installer les supports. Ensuite, nous avons pris différentes mesures entre les piliers pour déterminer les longueurs de profilés à découper. Nous avons également remarqué qu’il était nécessaire de prévoir des fixations supplémentaires sur certaines pièces pour pouvoir attacher correctement les courroies.

Images/Capture d’écran 2025-04-26 à 15.21.19.png

### 10ème séances

Lors de cette séance, j’ai réalisé toutes les modifications 3D nécessaires afin de pouvoir enfin imprimer les pièces et monter le système complet. Un nouveau projet Fusion a aussi été créé pour prendre en compte les cotes réelles du montage.

Images/Capture d’écran 2025-04-26 à 15.21.55.png


### 11ème séances

Durant la 11ᵉ séance, nous avons découpé les profilés à la bonne longueur.

/Users/kylianyacoub/Downloads/Capture d’écran 2025-04-26 à 15.22.17.png


Nous avons ensuite assemblé l’ensemble du système, en effectuant les derniers réglages mécaniques pour tout finaliser.

