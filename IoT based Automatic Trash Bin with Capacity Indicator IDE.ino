
//Proyek Akhir Sistem Benam-1
//Judul Proyek : Automatic Trash Bin with Capacity Indicator

//memanggil library ultrasonic
#include <ESP32Servo.h>             //memanggil library servo
#include <LiquidCrystal_I2C.h> //memanggil library LCD
#include <Wire.h>
#include <time.h>
#include <WiFi.h>
#define BLYNK_TEMPLATE_ID "TMPL54NmBVxS"
#define BLYNK_DEVICE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "Qddgp4t7NGb6GUgU1Md5qqfuOqjzoEff"
#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

// WiFi network info.
char ssid[] = "RedmiNote9Pro";
char wifiPassword[] = "12345678";

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;

const char* ntpServer = "id.pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

const int TRIGGER_PIN_SENSOR_SAMPAH = 27;  //pin trigger pada pin 8 esp
const int ECHO_PIN_SENSOR_SAMPAH = 26;    //pin echo pada pin 7 esp

//sensor kapasitas
const int TRIGGER_PIN_KAPASITAS = 33;     //pin trigger pada pin 10 esp
const int ECHO_PIN_KAPASITAS = 32;         //pin echo pada pin 9 esp


int maks = 29; //jarak maks sensor, tinggi maksimal dari kotak sampah dalam cm
int servo = 13; //inisialisasi pin servo

float pers, diff;             //inisialisasi variabel kapasitas tempat sampah
float durasi_sensor_sampah;   //inisialiasi durasi pengiriman + penerimaan sinyal echo
float jarak_sensor_sampah;    //inisialisasi jarak sensor dengan objek orang  
float durasi_kapasitas;       //inisialiasi durasi pengiriman + penerimaan sinyal echo
float jarak_kapasitas;        //inisialisasi jarak sensor dengan objek sampah

LiquidCrystal_I2C lcd(0x27, 16, 2); //inisialisasi nomor pin LCD 
Servo myservo; //membuat servo sebagai objek yang dikontrol

void setup() {
  Serial.begin(115200); //mulai serial komunikasi digital output
  
  xTaskCreate(
    taskblynk, //nama fungsinya
    "Cek Penuh", //nama task
    10000, //ukuran stack
    NULL, //task parameters
    1, //task priority
    NULL //task handle
    );

  xTaskCreate(
    mainprogram,
    "Program utama",
    10000,
    NULL,
    0,
    NULL
    );

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  Blynk.begin(auth, ssid, wifiPassword);
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  pinMode(TRIGGER_PIN_SENSOR_SAMPAH, OUTPUT); //atur PIN TRIGGER sebagai OUTPUT
  pinMode(ECHO_PIN_SENSOR_SAMPAH, INPUT);     //atur PIN ECHO sebagai INPUT
  pinMode(TRIGGER_PIN_KAPASITAS, OUTPUT);     //atur PIN TRIGGER sebagai OUTPUT
  pinMode(ECHO_PIN_KAPASITAS, INPUT);         //atur PIN ECHO sebagai INPUT
  Wire.begin();
  lcd.init(); // initialize LCD                    
  lcd.backlight(); // turn on LCD backlight  
  myservo.attach(servo);      //pin servo pada pin 6         
  myservo.write(0);           //set servo mulai dari 0 derajat

} 

void loop() {
  Blynk.run();
  vTaskDelay(1000);
}

void mainprogram(void * parameters){
  for(;;){
    printLocalTime();
    digitalWrite(TRIGGER_PIN_SENSOR_SAMPAH, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN_SENSOR_SAMPAH, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN_SENSOR_SAMPAH, LOW);
  
    durasi_sensor_sampah = pulseIn(ECHO_PIN_SENSOR_SAMPAH, HIGH); //membaca pin echo
    jarak_sensor_sampah = ((durasi_sensor_sampah*0.034)/2); //menghitung jarak
    
    //mengatur trigger untuk pengiriman gelombang ultrasonik
    digitalWrite(TRIGGER_PIN_KAPASITAS, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN_KAPASITAS, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN_KAPASITAS, LOW);
  
    durasi_kapasitas = pulseIn(ECHO_PIN_KAPASITAS, HIGH); //membaca pin echo
    jarak_kapasitas = (durasi_kapasitas/2)/29.1; //menghitung jarak sensor dengan objek sampah
    diff = maks - jarak_kapasitas;             //menghitung tinggi (cm) kapasitas sampah 
    pers = (diff/maks)*100;                    //menghitung persen kapasitas isi tempat sampah
     
    Serial.print("Jarak : ");
    Serial.println(jarak_sensor_sampah); //print jarak ke serial monitor
  
    if (pers >= 68){
      //tampilkan ke lcd
      lcd.clear();
      lcd.setCursor(0,0);                 //set kolom dan baris
      lcd.print("  Kotak Sampah  ");      
      lcd.setCursor(0,1);                 //set kolom dan baris
      lcd.print("   Sudah Penuh     ");   
      myservo.write(00);             //posisikan servo di 0 derajat
      delay(100);                    //delay selama 100 mikrodetik
    }
    else if(jarak_sensor_sampah <= 10 && pers < 68){
       lcd.clear();
       myservo.write(90);            //posisikan servo di 90 derajat
       delay (100);                  //delay selama 100 mikrodetik
       myservo.write(180);           //posisikan servo di 180 derajat
       delay (5000);                 //delay selama 5000 mikrodetik
       myservo.write(90);            //posisikan servo di 90 derajat
       delay (100);                  //delay selama 100 mikrodetik
       myservo.write(00);            //posisikan servo di 0 derajat
         //tampilkan ke lcd
      lcd.setCursor(0, 0);                  //set kolom dan baris
      lcd.print("  Kotak Sampah ");
      lcd.setCursor(0, 1);                  //set kolom dan baris
      lcd.print(" Terisi : ");
      lcd.print(pers);     //print hasil perhitungan persen kapasitas isi tempat sampah
      lcd.print("% ");
    }
    else {
      lcd.clear();
       //tampilkan ke lcd
      lcd.setCursor(0, 0);                  //set kolom dan baris
      lcd.print("  Kotak Sampah ");
      lcd.setCursor(0, 1);                  //set kolom dan baris
      lcd.print(" Terisi : ");
      lcd.print(pers);     //print hasil perhitungan persen kapasitas isi tempat sampah
      lcd.print("% ");
      myservo.write(00);             //posisikan servo di 0 derajat
      delay(100);                    //delay selama 100 mikrodetik
    }
    delay(500);
  }
//  vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void taskblynk(void * parameters) {
  for( ;;) {
    if(pers >= 68){
      Blynk.virtualWrite(V5, pers);
      Blynk.virtualWrite(V3, 1);
    }
    else{
      Blynk.virtualWrite(V5, pers);
      Blynk.virtualWrite(V3, 0);
    }
  }
//  vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
}
