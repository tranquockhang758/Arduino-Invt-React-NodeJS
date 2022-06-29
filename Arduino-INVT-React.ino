//D0  GPIO16
//D1  GPIO5
//D2  GPIO4
//D3,D15  GPIO0
//D4,D14  GPIO2
//D5,D13  GPIO14
//D6,D12  GPIO12
//D7,D11  GPIO13
//D8  GPIO15
//D9  GPIO3
//D10 GPIO1
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
ESP8266WiFiMulti WiFiMulti;

//======================================Connecting to Server
const char* ssid = "hieunhanml@CIA.hoangnhatjav";
const char* password ="maybienap123456";
String url = "http://192.168.137.10:8000/api/ViewDataArduino";
String json_string;
int Status_Code = 0;
int Freqyency = 0;
//======================================Connecting to Server

//======================================Modbus RTU
ModbusMaster node;
#define TSEN 0 // Khai Báo Chân Điều Hướng D3 GPIO0
#define IDSLAVE1 1  // Khai Báo ID Giao Tiếp Của Biến Tần iG5A (I60=1)
void preTransmission(); //=> Cho phép truyền nhận
void postTransmission();
//======================================Modbus RTU

void setup() {
  Serial.begin(19200,SERIAL_8N1);
  //Đèn báo truyền nhận dữ liệu
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);

  //Chân điều hướng
  pinMode(TSEN,OUTPUT);
  digitalWrite(TSEN, 0);

  //=================================Config Truyền thông
  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  //=================================Config Truyền thông

  //=================================Config Wifi 
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //=================================Config Wifi
}


void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    int id = 1;
    int Feedback = 100;
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    String newUrl = url + "?id=" + String(id) + "&Feedback=" + String(Feedback); 

    http.begin(client,newUrl);
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();
    if (httpCode > 0 ) {
      //Nếu nhận được dữ liệu
       if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        digitalWrite(12,HIGH);
        //Sau khi load được dữ liệu từ Server về ta chuyển đổi dữ liệu để điều khiển
                  String payload = http.getString();
                  JSONVar my_obj = JSON.parse(payload);
                    if (JSON.typeof(my_obj) == "undefined") {
                    Serial.println("Parsing input failed!");
                    return;
                  }
//{"Feedback":"100", "data":{"id":1,"Stop":true,"Reverse":false,"Forward":false,"Setpoint":1199,"Feedback":100,"Current":0,"Voltage":0,"Frequency":0},
//"message":"Data Fetch in DB","errCode":0}
                  bool Stop = my_obj["data"]["Stop"];
                  bool Reverse = my_obj["data"]["Reverse"];
                  bool Forward = my_obj["data"]["Forward"];
                  int Setpoint = my_obj["data"]["Setpoint"];
                  int Feedback = my_obj["data"]["Feedback"];
                  int Current = my_obj["data"]["Current"];
                  int Voltage = my_obj["data"]["Voltage"];
                  int Frequency = my_obj["data"]["Frequency"];
                  if(Stop == true){
                    Status_Code = 1;
                    node.writeSingleRegister(4, 0);
                    node.writeSingleRegister(5, Status_Code);
                  }
                  if(Reverse == true){
                    Status_Code = 4;
                    node.writeSingleRegister(4, 3000);
                    node.writeSingleRegister(5, Status_Code);
                  }
                  if(Forward == true){
                    Status_Code = 2;
                    node.writeSingleRegister(4, 3000);
                    node.writeSingleRegister(5, Status_Code);
                  }
                  digitalWrite(12,LOW);
                  //Ki tu 0 la quy dinh cua C
                  //int I00 = payload[  payload.indexOf("I00")+6] -'0';
              }
      
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    } 
    http.end();
  }
}
//=================================Hàm truyền/nhận
void preTransmission()
{
  digitalWrite(TSEN, 1);
}

void postTransmission()
{
  digitalWrite(TSEN, 0);
}
