// Enter your WiFi ssid and password
const char* ssid     = "Stop Pinching";   //your network SSID
const char* password = "keyroom8";   //your network password

String Script = "/macros/s/AKfycbzEkzg8nOeuuxQI_XlK6hTme-ocjNhP9TTcCPPRrWgE6O7oujNH/exec";    //Create your Google Apps Script and replace the "Script" path.
String Line_Token = "myToken=**********";    //Line Notify Token. You can set the value of xxxxxxxxxx empty if you don't want to send picture to Linenotify.
String Folder_Name = "&Folder_Name=ESP32-CAM";
String File_Name = "&File_Name=ESP32-CAM.jpg";
String Image = "&File=";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

#include "esp_camera.h"

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO     32
#define RESET_GPIO    -1
#define XCLK_GPIO      0
#define SIOD_GPIO     26
#define SIOC_GPIO     27

#define Y9_GPIO       35
#define Y8_GPIO       34
#define Y7_GPIO       39
#define Y6_GPIO       36
#define Y5_GPIO       21
#define Y4_GPIO       19
#define Y3_GPIO       18
#define Y2_GPIO        5
#define VSYNC_GPIO    25
#define HREF_GPIO     23
#define PCLK_GPIO     22

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  
  long int Starting_Time = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if ((Starting_Time+10000) < millis()) break;
  } 

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO;
  config.pin_d1 = Y3_GPIO;
  config.pin_d2 = Y4_GPIO;
  config.pin_d3 = Y5_GPIO;
  config.pin_d4 = Y6_GPIO;
  config.pin_d5 = Y7_GPIO;
  config.pin_d6 = Y8_GPIO;
  config.pin_d7 = Y9_GPIO;
  config.pin_xclk = XCLK_GPIO;
  config.pin_pclk = PCLK_GPIO;
  config.pin_vsync = VSYNC_GPIO;
  config.pin_href = HREF_GPIO;
  config.pin_sscb_sda = SIOD_GPIO;
  config.pin_sscb_scl = SIOC_GPIO;
  config.pin_pwdn = PWDN_GPIO;
  config.pin_reset = RESET_GPIO;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void loop()
{
  Send_Captured_Images();
  delay(60000);
}

String Send_Captured_Images() {
  const char* myDomain = "script.google.com";
  String getAll="", getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = Line_Token+Folder_Name+File_Name+Image;
    
    client_tcp.println("POST " + Script + " HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Connection: keep-alive");
    client_tcp.println();
    
    client_tcp.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
      client_tcp.print(imageFile.substring(Index, Index+1000));
    }
    esp_camera_fb_return(fb);
    
    int Waiting_Time = 10000;   // timeout 10 seconds
    long Starting_Time = millis();
    boolean state = false;
    
    while ((Starting_Time + Waiting_Time) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          Starting_Time = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  
  return getBody;
}


String urlencode(String str)
{
    String Encoded_String="";
    char c;
    char code_0;
    char code_1;
    char code_2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        Encoded_String+= '+';
      } else if (isalnum(c)){
        Encoded_String+=c;
      } else{
        code_1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code_1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code_0=c+'0';
        if (c > 9){
            code_0=c - 10 + 'A';
        }
        code_2='\0';
        Encoded_String+='%';
        Encoded_String+=code_0;
        Encoded_String+=code_1;
        //Encoded_String+=code_2;
      }
      yield();
    }
    return Encoded_String;
}
