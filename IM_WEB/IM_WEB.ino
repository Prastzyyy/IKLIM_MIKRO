#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
//#include WebServer???     //esp32
#include <Ticker.h>
//#include <LiquidCrystal_I2C.h>
//#include <DHT.h>
#include <ESP8266WiFi.h>
//#include <WiFi.h>     //esp32
#include <ESP8266HTTPClient.h>  
//#include <HTTPClient.h>  //esp32
#include <WiFiClientSecure.h>
//#include <RTClib.h>
#include <EEPROM.h>
#include "web.h"


ESP8266WebServer server(80);

// Password untuk autentikasi OTA
const char* ota_password = "123";
void setupOTA();
String OTA_msg = "Update #2";

// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbz3S04CGjpaPvaXhfWPEMwXwXurooe9cCbCTPkuVKQfBuWraHT5OzGsCln2pL3PFu-r/exec";
void spreadsheet();
int dataSpreadsheet;
String warning;

char ssid[] = "Prastzy.net"; 
char pass[] = "123456781";  
//const char* ssid ="bebas";
//const char* password = "akunulisaja";

//#define DHTPIN 4 
//#define DHTTYPE DHT22 
#define relay_kipas D6//19
#define relay_sprayer D5//18
#define relay_lampu D4//17
//#define LDR_PIN 34 //analog pin

//LiquidCrystal_I2C lcd(0x27, 20, 4);  
//DHT dht(DHTPIN, DHTTYPE);

//RTC_DS1307 rtc;
int setWaktu1 [] = {4,0,0}; //jam, menit, detik
int setWaktu2 [] = {5,0,0};
int setWaktu3 [] = {18,0,0};

int lightValue;
float temperature, humidity;
float set_temperature = 30;
float set_humidity = 80;
float set_light = 1000;
long lama_cahaya, lama_lampu, waktu_lampu;
#define EEPROM_SIZE 12 
String kondisi_kipas = "Padam", kondisi_sprayer = "Padam", kondisi_lampu = "Padam";
bool jam4, jam5_18, jam18_4, status_kipas, status_sprayer, status_lampu;

void baca_RTC();
void setup_wifi();
void kontrol_sprayer();
void handleRoot();
void handleNotFound();
void handleData();

Ticker task1, task2, task3;
void monitoring();
void kontrol_kipas();
void kontrol_lampu();

void setup() {
  Serial.begin(115200);
  pinMode(relay_kipas, OUTPUT);
  pinMode(relay_sprayer, OUTPUT);
  pinMode(relay_lampu, OUTPUT);
  //pinMode(LDR_PIN, INPUT);

  setup_wifi();
  //dht.begin();
  //lcd.init();   
  /*      
  lcd.backlight();    
  lcd.setCursor(3,1);
  lcd.print("START  PROGRAM");
  lcd.setCursor(3,2);
  lcd.print("==============");
  delay(1000);
  lcd.clear();
  */

  // Inisialisasi EEPROM
  EEPROM.begin(EEPROM_SIZE);
  // Membaca nilai terakhir dari EEPROM
  lama_cahaya = readLong(0); // Offset 0 
  waktu_lampu = readLong(4); // Offset 4 
  lama_lampu = readLong(8); // Offset 8 
  // Menampilkan nilai awal dari EEPROM
  Serial.println("Nilai Lama Cahaya dari EEPROM: " + String(lama_cahaya));
  Serial.println("Nilai Lama Lampu dari EEPROM: " + String(lama_lampu));
  Serial.println("Nilai Waktu Lampu dari EEPROM: " + String(waktu_lampu));
  

  /*
   if (! rtc.begin()) {
    Serial.println("RTC tidak ditemukan");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC tidak berjalan, setting waktu...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set waktu sesuai waktu kompilasi
  }
  */
  ArduinoOTA.setHostname("esp8266 - Iklim Mikro");
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin(); 
  task1.attach(3, monitoring);
  task2.attach(3, kontrol_kipas);
  task3.attach(3, kontrol_lampu);

  server.on("/", [](){
    handleRoot();
  });
  //kipas
  server.on("/kipas_on", [](){
    Serial.println("");
    Serial.print("/kipas_on"); 
    Serial.println("");
    status_kipas = true;
    handleRoot();
  });
  server.on("/kipas_off", [](){
    Serial.println("");
    Serial.print("/kipas_off"); 
    Serial.println("");
    status_kipas = false;
    handleRoot();
  });
  //sprayer
  server.on("/sprayer_on", [](){
    Serial.println("");
    Serial.print("/sprayer_on"); 
    Serial.println("");
    status_sprayer = true;
    handleRoot();
  });
  server.on("/sprayer_off", [](){
    Serial.println("");
    Serial.print("/sprayer_off"); 
    Serial.println("");
    status_sprayer = false;
    handleRoot();
  });
  //lampu
  server.on("/lampu_on", [](){
    Serial.println("");
    Serial.print("/lampu_on"); 
    Serial.println("");
    status_lampu = true;
    handleRoot();
  });
  server.on("/lampu_off", [](){
    Serial.println("");
    Serial.print("/lampu_off"); 
    Serial.println("");
    status_lampu = false;
    handleRoot();
  });
  
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() { 
  kontrol_sprayer();
  
  ArduinoOTA.handle();
  if (status_kipas){
    spreadsheet();
    delay(5000);
  }
  delay(1000);
}

//Task monitoring
void monitoring() {
/*
    //baca sensor
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    lightValue = analogRead(LDR_PIN);
    baca_RTC();
  
    //serial monitor
    Serial.print("Suhu : "); Serial.println(temperature);
    Serial.print("Kelembaban : "); Serial.println(humidity);
    Serial.print("Cahaya : "); Serial.println(lightValue);
    Serial.println("");
*/
    server.handleClient();
    temperature = random(25, 36);
    humidity = random(70, 91);
    lightValue = random(500, 1000);
    //Print LCD
    /*
    lcd.setCursor(0, 1);
    lcd.print("Temp: "); lcd.print(temperature); lcd.print(" C");
    lcd.setCursor(0, 2);
    lcd.print("Hum: "); lcd.print(humidity); lcd.print(" %");
    lcd.setCursor(0, 3);
    lcd.print("LDR: "); lcd.print(lightValue);
    */
}

//Task pengkondisian kipas
void kontrol_kipas(){
    if (temperature > set_temperature){
      digitalWrite (relay_kipas, HIGH);
      kondisi_kipas = "Menyala";
    }
    else if (temperature <= set_temperature) {
      if (status_kipas){
        digitalWrite (relay_kipas, HIGH);
        kondisi_kipas = "Menyala";
      }else {
        digitalWrite (relay_kipas, LOW);
        kondisi_kipas = "Padam";
      }
    }
}

//Task pengkondisian sprayer
void kontrol_sprayer(){
    if (humidity < set_humidity){
      digitalWrite (relay_sprayer, HIGH);
      kondisi_sprayer = "Menyala";
      delay(5000);
      digitalWrite (relay_sprayer, LOW);
      kondisi_sprayer = "Padam";
    }
    else if (humidity >= set_humidity) {
      if (status_sprayer){
        digitalWrite (relay_sprayer, HIGH);
        kondisi_sprayer = "Menyala";
        delay(5000);
        digitalWrite (relay_sprayer, LOW);
        kondisi_sprayer = "Padam";
      }else {
        digitalWrite (relay_sprayer, LOW);
        kondisi_sprayer = "Padam";
      }
    }
}

//task baca cahaya
void kontrol_lampu () {
    //DateTime now = rtc.now(); 
    //jam 4 pagi, reset
    //reset dibuat antara jam 04:00:00 sampai 04:00:10
    //if (now.hour() == setWaktu1[0] && now.minute() == setWaktu[1] && now.second() == setWaktu[2]){
    if (jam4 == true) {
      //Serial.println("reset lama_cahaya menjadi 0");
      lama_cahaya = 0;
      //Serial.println("reset lama_lampu menjadi 0");
      waktu_lampu = 0;
      lama_lampu = 100;
    }
    //jam 5 sampai jam 18, menghitung lama cahaya
    //if (now.hour() > setWaktu2[0] && now.hour() < setWaktu3[0]){
    if (jam5_18 == true) {
        if (lightValue >= set_light) {
          if (lama_cahaya >= 99){
            Serial.print("lama cahaya :"); Serial.println(lama_cahaya);
          }else {
            lama_cahaya += 3;
            Serial.print("lama cahaya :"); Serial.println(lama_cahaya);
          }
        }
        else {
           Serial.print("lama cahaya :"); Serial.println(lama_cahaya);
        }
        lama_lampu = (100 - lama_cahaya);
        Serial.print("lama lampu : "); Serial.println(lama_lampu);
    }
    //jam 18 sampai jam 3 , menyalakan lampu
    //if (now.hour() > setWaktu3[0] && now.hour() < setWaktu1[0]){
    if (jam18_4 == true) {
      if (waktu_lampu >= 0 && waktu_lampu < lama_lampu) {
        digitalWrite (relay_lampu, HIGH);
        kondisi_lampu = "Menyala";
        waktu_lampu += 3;
        Serial.print("lampu menyala : "); Serial.println(waktu_lampu);
      }
      else if (waktu_lampu >= lama_lampu){
        if (status_lampu){
          digitalWrite (relay_lampu, HIGH);
          kondisi_lampu = "Menyala";
        }else {
          Serial.println("lampu padam");
          digitalWrite (relay_lampu, LOW);
          kondisi_lampu = "Padam";
        }
      }
    }
    // Simpan nilai terbaru ke EEPROM
    writeLong(0, lama_cahaya); // Simpan di offset 0
    writeLong(4, waktu_lampu); // Simpan di offset 4
    writeLong(8, lama_lampu); // Simpan di offset 8
    EEPROM.commit(); // Pastikan data tersimpan ke memori
}

// Fungsi untuk membaca long dari EEPROM
long readLong(int address) {
  long value = 0;
  for (int i = 0; i < 4; i++) {
    value |= (EEPROM.read(address + i) << (8 * i));
  }
  return value;
}

// Fungsi untuk menulis long ke EEPROM
void writeLong(int address, long value) {
  for (int i = 0; i < 4; i++) {
    EEPROM.write(address + i, (value >> (8 * i)) & 0xFF);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Terhubung ke WiFi dengan IP: ");
  Serial.println(WiFi.localIP());
}

void baca_RTC(){
  //Serial monitor
  /*
  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(" - ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  //LCD
  lcd.setCursor(0,0);
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print(" - ");
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);
  */
}

void spreadsheet(){
  // This condition is the condition for sending or writing data to Google Sheets.
  if (WiFi.status() == WL_CONNECTED) {
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&lightValue=" + String (lightValue);
    Send_Data_URL += "&temperature=" + String(temperature);
    Send_Data_URL += "&humidity=" + String(humidity);
    Send_Data_URL += "&kipas=" + kondisi_kipas;
    Send_Data_URL += "&sprayer=" + kondisi_sprayer;
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);

      WiFiClientSecure client;
      client.setInsecure(); // Nonaktifkan validasi SSL
      HTTPClient http;
      http.begin(client, Send_Data_URL);  
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int httpCode = http.GET(); 
      http.end();

      String Read_Data_URL = Web_App_URL + "?sts=read";
      http.begin(client, Read_Data_URL); 
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      httpCode = http.GET();
      Serial.print("HTTP Status Code : ");
      Serial.println(httpCode);
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("Data ke : " + payload);    
        dataSpreadsheet = payload.toInt();
      }
      http.end();
      Serial.println("-------------");
  }
}


void handleRoot() {
  server.send(200, "text/html", SendHTML(temperature, humidity, lightValue, kondisi_kipas, kondisi_sprayer, kondisi_lampu));
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
  message += " " + server.argName(i) + ": " + server.arg(i)+ "\n";
  }
  server.send(404, "text/plain", message);
}

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"lightValue\":" + String(lightValue) + ",";
  json += "\"kondisi_kipas\":\"" + kondisi_kipas + "\",";
  json += "\"kondisi_sprayer\":\"" + kondisi_sprayer + "\",";
  json += "\"kondisi_lampu\":\"" + kondisi_lampu + "\"";
  json += "}";
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "application/json", json);
}