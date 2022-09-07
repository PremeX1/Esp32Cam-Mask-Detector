#include "esp_camera.h"
#include <WiFi.h>
#include <WebSocketsServer.h>

#define SOUND_BIT 8
#define CHANNEL 0
#define CHANNEL_ 1
#define FREQ 2000
#define FLASH_GPIO_NUM 4
#define BUZZER_GPIO_NUM 15

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


WebSocketsServer webSocket = WebSocketsServer(81);
WiFiServer server(80);
uint8_t cam_num;
char _payload[16];
bool connected = false;

String index_html ="<html>\n \
<head>\n \
<title>Not Client !!!!</title>\n \
</head>\n \
<body>\n \
<div id='output'>Anoymouse Gateway</div>\n \
</body>\n \
</html>\n \
</script>";
void configCamera(){
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

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 9;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void liveCam(uint8_t num){
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Frame buffer could not be acquired");
      return;
  }
  webSocket.sendBIN(num, fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            cam_num = num;
            connected = true;
            break;
        case WStype_TEXT:
            strcpy(_payload, (char *)(payload));
            break;
        case WStype_BIN:
            break;
        case WStype_ERROR:
            break;
        case WStype_FRAGMENT_TEXT_START:
            break;
        case WStype_FRAGMENT_BIN_START:
            break;
        case WStype_FRAGMENT:
            break;
        case WStype_FRAGMENT_FIN:
            break;
        default:
          break;
    }
}
void setup() {
  Serial.begin(115200);
  WiFi.begin("LTL", "ProjectPassword");
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  String IP = WiFi.localIP().toString();
  Serial.print("IP address: " + IP);
  index_html.replace("server_ip", IP);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  configCamera();
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  ledcSetup(CHANNEL, FREQ, SOUND_BIT);
  ledcSetup(CHANNEL_, FREQ, SOUND_BIT);
  ledcAttachPin(15, CHANNEL);
  ledcAttachPin(FLASH_GPIO_NUM, CHANNEL_);
}
    
void http_resp(){
  WiFiClient client = server.available();
  if (client.connected() && client.available()) {                   
    client.flush();          
    client.print(index_html);
    client.stop();
  }
}
//void warning() {
//  ledcWrite(CHANNEL, 240);
//  delay(1000);
//  ledcWrite(CHANNEL, 0);
//  delay(100);
//  ledcWrite(CHANNEL, 240);
//  delay(300);
//  ledcWrite(CHANNEL, 0);
//  delay(300);
//  ledcWrite(CHANNEL, 240);
//  delay(200);
//  ledcWrite(CHANNEL, 0);
//  delay(1000);  
//}
void loop() {
  http_resp();
  webSocket.loop();
  ledcWrite(CHANNEL_, 1);
  Serial.println(_payload);
  String __payload = String(_payload);
  if(__payload == "F-ON") {
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    Serial.println("LIGHT ACTIVE");
  } else {
    digitalWrite(FLASH_GPIO_NUM, LOW);
    Serial.println("LIGHT DEACTIVE");
  }

  if(__payload == "BEEP") {
    Serial.println("SOUND ACTIVE");
    ledcWrite(CHANNEL, 240);
  }
  else {
    Serial.println("SOUND DEACTIVE");
    ledcWrite(CHANNEL, 0);
  }
  if(connected == true){
    liveCam(cam_num);
  }

  
}
