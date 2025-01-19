#include <ArduinoOTA.h>
#include <Ticker.h>
//#include <LiquidCrystal_I2C.h>
//#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <ESP8266WiFi.h>
//#include <WiFi.h>     //esp32
#include <ESP8266HTTPClient.h>  
//#include <HTTPClient.h>  //esp32
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
//#include <RTClib.h>
#include <EEPROM.h>

// Password untuk autentikasi OTA
const char* ota_password = "123";
String OTA_msg = "Update #2";

// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbynTuRXx2u7v6wUIQCribduwnS6NT2rEuyMl5PokGOYDC9ymPUWh29QlGjzI1p97Ryi/exec";
void spreadsheet();
int dataSpreadsheet;
String warning;

//MQTT
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "ee.unsoed.ac.id";
const int mqtt_port = 1883;

const char* topic_temperature = "greenhouse/sensor/temperature";
const char* topic_humidity = "greenhouse/sensor/humidity";
const char* topic_ldr = "greenhouse/sensor/light";

const char* topic_kontrol_kipas = "greenhouse/kontrol/kipas";
const char* topic_kontrol_sprayer = "greenhouse/kontrol/sprayer";
const char* topic_kontrol_lampu = "greenhouse/kontrol/lampu";
const char* topic_test = "greenhouse/kontrol/test";  //kontrol waktu

const char* topic_set_temperature = "greenhouse/set/temperature";
const char* topic_set_humidity = "greenhouse/set/humidity";
const char* topic_set_light = "greenhouse/set/light";

const char* topic_kondisi_kipas = "greenhouse/kondisi/kipas";
const char* topic_kondisi_sprayer = "greenhouse/kondisi/sprayer";
const char* topic_kondisi_lampu = "greenhouse/kondisi/lampu";

const char* topic_warning = "greenhouse/info/warning";
const char* topic_ota = "greenhouse/info/ota";

const char* ssid = "Prastzy.net"; 
const char* pass = "123456781";  
//const char* ssid ="bebas";
//const char* pass = "akunulisaja";

//#define DHTPIN 4 
//#define DHTTYPE DHT22 
#define relay_kipas D6//19
#define relay_sprayer D5//18
#define relay_lampu D4//17
//#define LDR_PIN 34 //analog pin

//LiquidCrystal_I2C lcd(0x27, 20, 4);  
//DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter;
//RTC_DS1307 rtc;
int setWaktu1 [] = {4,0,0}; //jam, menit, detik
int setWaktu2 [] = {5,0,0};
int setWaktu3 [] = {18,0,0};

const int delay_sprayer = 5000; 
int lightValue;
float temperature, humidity;
float set_temperature = 30;
float set_humidity = 80;
float set_light = 1000;
long lama_cahaya, lama_lampu, waktu_lampu;
#define EEPROM_SIZE 12 
String kondisi_kipas = "Padam", kondisi_sprayer = "Padam", kondisi_lampu = "Padam";
bool jam4, jam5_18, jam18_4, status_kipas, status_sprayer, status_lampu;
bool modeotomatis = false;
String statusSistem = "Manual";
float kalibrasi_suhu = 0.67, kalibrasi_kelembaban = -13.1, kalibrasi_lux = 3.96 ;
String msglampu1, msglampu2;

void baca_RTC();
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void kontrol_sprayer();

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
  /*
  dht.begin();
  Wire.begin();
  if (!lightMeter.begin()) {
    Serial.println(F("Gagal menginisialisasi sensor BH1750!"));
    Serial.println(F("Periksa koneksi kabel."));
    while (1); // Berhenti jika inisialisasi gagal
  }
  */
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
  ArduinoOTA.setHostname("esp8266 - Iklim Mikro");
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

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
  task1.attach(3, monitoring);
  task2.attach(3, kontrol_kipas);
  task3.attach(3, kontrol_lampu);
}

void loop() { 
  if(!client.connected()) {
     reconnect();
  }
  client.loop();
  kontrol_sprayer();
  
  ArduinoOTA.handle();
}

//Task monitoring
void monitoring() {
/*
    //baca sensor
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Gagal membaca dari sensor DHT22!"));
      return;
    }
    temperature += 0.67; // Tambahkan 0.67 ke suhu
    humidity -= 13.1;     // Kurangi 5.7 dari kelembapan
    float lightValue = lightMeter.readLightLevel();
    baca_RTC();
  
    //serial monitor
    Serial.print("Suhu : "); Serial.println(temperature);
    Serial.print("Kelembaban : "); Serial.println(humidity);
    Serial.print("Cahaya : "); Serial.println(lightValue);
    Serial.println("");
*/
    //=======================================
    temperature = random(25, 36);
    humidity = random(70, 91);
    lightValue = random(500, 1000);
    
    temperature +=  kalibrasi_suhu; //0.67; 
    humidity += kalibrasi_kelembaban; //-13.1; 
    lightValue = lightValue * kalibrasi_lux;    
    //=======================================
    //Print LCD
    /*
    lcd.setCursor(0, 1);
    lcd.print("Temp: "); lcd.print(temperature); lcd.print(" C");
    lcd.setCursor(0, 2);
    lcd.print("Hum: "); lcd.print(humidity); lcd.print(" %");
    lcd.setCursor(0, 3);
    lcd.print("LDR: "); lcd.print(lightValue);
    */
    //Print MQTT
    client.publish(topic_temperature, String(temperature).c_str());
    client.publish(topic_humidity, String(humidity).c_str());
    client.publish(topic_ldr, String(lightValue).c_str());

    client.publish(topic_kondisi_kipas, kondisi_kipas.c_str());
    client.publish(topic_kondisi_sprayer, kondisi_sprayer.c_str());
    client.publish(topic_kondisi_lampu, kondisi_lampu.c_str());
}

//Task pengkondisian kipas
void kontrol_kipas(){
  if(modeotomatis){
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
}

//Task pengkondisian sprayer
void kontrol_sprayer() {
  if(modeotomatis){
    static unsigned long previousMillisSprayer = 0;  // Waktu sebelumnya
    static bool sprayerActive = false;              // Status sprayer   
    static bool firstActivation = true; 
    
    // Kondisi ketika kelembapan lebih kecil dari setpoint
    if (humidity < set_humidity) {
        if (!sprayerActive && firstActivation) {
            digitalWrite(relay_sprayer, HIGH);
            kondisi_sprayer = "Menyala";
            previousMillisSprayer = millis();  // Catat waktu saat sprayer diaktifkan
            sprayerActive = true;             // Tandai sprayer sebagai aktif
            firstActivation = false;
        }

        // Matikan sprayer setelah durasi tertentu
        if (millis() - previousMillisSprayer >= delay_sprayer) {
            digitalWrite(relay_sprayer, LOW);
            kondisi_sprayer = "Padam";
            sprayerActive = false;            // Tandai sprayer sebagai tidak aktif
            firstActivation = false;
        }
    }
    // Kondisi ketika kelembapan >= setpoint
    else if (humidity >= set_humidity) {
        if (status_sprayer) {
            // Nyalakan sprayer jika status_sprayer aktif
            digitalWrite(relay_sprayer, HIGH);
            kondisi_sprayer = "Menyala";
            firstActivation = true;
        } else {
            // Matikan sprayer jika status_sprayer tidak aktif
            digitalWrite(relay_sprayer, LOW);
            kondisi_sprayer = "Padam";
            sprayerActive = false;  // Pastikan status sprayer juga tidak aktif
            firstActivation = true;
        }
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
            Serial.print("lama cahaya  :"); Serial.println(lama_cahaya);
          }
        }
        else {
           Serial.print("lama cahaya :"); Serial.println(lama_cahaya);
        }
        lama_lampu = (100 - lama_cahaya);
        Serial.print("lama lampu : "); Serial.println(lama_lampu);
    }
  if(modeotomatis){
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
  }
  msglampu1 = "=> " + String(lama_lampu) + " detik";
  msglampu2 = "Lampu sudah menyala selama : " + String(waktu_lampu) + " detik";
  client.publish("greenhouse/info/lamalampu", msglampu1.c_str());
  client.publish("greenhouse/info/waktulampu", msglampu2.c_str());
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

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Receive Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';  
  Serial.println(msg);

  // Saklar Mode Otomatis 
  if (!strcmp(topic, "greenhouse/kontrol/modeotomatis")) {
    if (!strncmp(msg, "on", length)) {
      modeotomatis = true;
      statusSistem = "Otomatis";
      client.publish("greenhouse/info/modeotomatis", statusSistem.c_str()); 
    } else if (!strncmp(msg, "off", length)) {
      modeotomatis = false;
      statusSistem = "Manual";
      client.publish("greenhouse/info/modeotomatis", statusSistem.c_str());
    }
  }

  // kontrol kipas
  if (!strcmp(topic, topic_kontrol_kipas)) {
    if (!strncmp(msg, "on", length)) {
      status_kipas = true;
      Serial.println("Saklar Kipas ON");
      digitalWrite (relay_kipas, HIGH);
      kondisi_kipas = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      status_kipas = false;
      Serial.println("Saklar Kipas OFF");
      digitalWrite (relay_kipas, LOW);
      kondisi_kipas = "Padam";
    }
  }

  //kontrol sprayer
  if (!strcmp(topic, topic_kontrol_sprayer)) {
    if (!strncmp(msg, "on", length)) {
      status_sprayer = true;
      Serial.println("Saklar Sprayer ON");
      digitalWrite (relay_sprayer, HIGH);
      kondisi_sprayer = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      status_sprayer = false;
      Serial.println("Saklar Sprayer OFF");
      digitalWrite (relay_sprayer, LOW);
      kondisi_sprayer = "Padam";
    }
  }

  //kontrol lampu
  if (!strcmp(topic, topic_kontrol_lampu)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_lampu, HIGH);
      kondisi_lampu = "Menyala";
      status_lampu = true;
      Serial.println("Saklar Lampu ON");
      digitalWrite(relay_lampu, HIGH);
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_lampu, LOW);
      kondisi_lampu = "Padam";
      status_lampu = false;
      Serial.println("Saklar Lampu OFF");
      digitalWrite(relay_lampu, LOW);
    }
  }
  //===================================================
  // Ambil data Spreadsheet
  if (!strcmp(topic, "greenhouse/kontrol/ambildata")) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengambil data untuk spreadsheet");
      spreadsheet();
    }
  }
  // KALIBRASI SUHU
  if (!strcmp(topic, "greenhouse/kontrol/kalibrasisuhu")) {
    kalibrasi_suhu = atof(msg); 
    Serial.print("Set kalibrasi Suhu menjadi : ");
    Serial.println(kalibrasi_suhu);
    client.publish("greenhouse/info/kalibrasisuhu", String(kalibrasi_suhu).c_str());
  }
  // KALIBRASI Kelembaban
  if (!strcmp(topic, "greenhouse/kontrol/kalibrasikelembaban")) {
    kalibrasi_kelembaban = atof(msg); 
    Serial.print("Set kalibrasi Kelembaban menjadi : ");
    Serial.println(kalibrasi_kelembaban);
    client.publish("greenhouse/info/kalibrasikelembaban", String(kalibrasi_kelembaban).c_str());
  }
  // KALIBRASI Lux
  if (!strcmp(topic, "greenhouse/kontrol/kalibrasilux")) {
    kalibrasi_lux = atof(msg); 
    Serial.print("Set kalibrasi Lux menjadi : ");
    Serial.println(kalibrasi_lux);
    client.publish("greenhouse/info/kalibrasilux", String(kalibrasi_lux).c_str());
  }
  //=================================================================
  // Set suhu 
  if (!strcmp(topic, topic_set_temperature)) {
    set_temperature = atof(msg); 
    Serial.print("Set suhu menjadi : ");
    Serial.println(set_temperature);
  }

  // Set kelembaban
  if (!strcmp(topic, topic_set_humidity)) {
    set_humidity = atof(msg); 
    Serial.print("Set kelembaban menjadi : ");
    Serial.println(set_humidity);
  }

  // Set Light
  if (!strcmp(topic, topic_set_light)) {
    set_light = atoi(msg); 
    Serial.print("Set Light menjadi : ");
    Serial.println(set_light);
  }
  //=============================================================
  // jam 4
  if (!strcmp(topic, topic_test)) {
    if (!strncmp(msg, "on1", length)) {
      jam4 = true;
      Serial.println("jam 4 on");
      Serial.println("reset lama_cahaya menjadi 0");
      lama_cahaya = 0;
      Serial.println("reset lama_lampu menjadi 0");
      waktu_lampu = 0;
      lama_lampu = 100;
    } else if (!strncmp(msg, "off1", length)) {
      jam4 = false;
      Serial.println("jam 4 off");
    }
  }

  // jam 5 - 18
  if (!strcmp(topic, topic_test)) {
    if (!strncmp(msg, "on2", length)) {
      jam5_18 = true;
      Serial.println("jam 5 - 18 on");
    } else if (!strncmp(msg, "off2", length)) {
      jam5_18 = false;
      Serial.println("jam 5 - 18 off");
    }
  }

  // jam 18 - 4
  if (!strcmp(topic, topic_test)) {
    if (!strncmp(msg, "on3", length)) {
      jam18_4 = true;
      Serial.println("jam 18 on");
    } else if (!strncmp(msg, "off3", length)) {
      jam18_4 = false;
      Serial.println("jam 18 off");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "esp32-clientId-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung");
      Serial.print("Status OTA : ");
      Serial.println(OTA_msg);
      client.publish(topic_ota, OTA_msg.c_str());
      // Suscribe ke topik
      client.subscribe(topic_kontrol_kipas);
      client.subscribe(topic_kontrol_sprayer);
      client.subscribe(topic_kontrol_lampu);
      client.subscribe(topic_set_temperature);
      client.subscribe(topic_set_humidity);
      client.subscribe(topic_set_light);
      client.subscribe("greenhouse/kontrol/modeotomatis");
      client.subscribe("greenhouse/kontrol/ambildata");
      client.subscribe("greenhouse/kontrol/kalibrasisuhu");
      client.subscribe("greenhouse/kontrol/kalibrasikelembaban");
      client.subscribe("greenhouse/kontrol/kalibrasilux");
      client.subscribe(topic_test);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
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
  if (WiFi.status() == WL_CONNECTED) {
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&lightValue=" + String (lightValue);
    Send_Data_URL += "&temperature=" + String(temperature);
    Send_Data_URL += "&humidity=" + String(humidity);
    Send_Data_URL += "&kipas=" + kondisi_kipas;
    Send_Data_URL += "&sprayer=" + kondisi_sprayer;
    Send_Data_URL += "&lampu=" + kondisi_lampu;
    Send_Data_URL += "&lamacahaya=" + String(lama_cahaya);
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);

    WiFiClientSecure client;
    client.setInsecure(); 
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
  // Sistem Peringatan data SpreadSheet
  if (dataSpreadsheet > 1000){
    warning = "Data sudah terlalu banyak";
  }else{
    warning = String (dataSpreadsheet);  
  }
  client.publish(topic_warning, warning.c_str());
}
