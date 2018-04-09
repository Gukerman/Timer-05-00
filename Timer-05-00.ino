/*
Подключения:
NodeMCU    -> Matrix
MOSI-D7-GPIO13  -> DIN
CLK-D5-GPIO14   -> Clk
GPIO0-D3        -> LOAD

key=5; //define key D1 & GND
Rx - in amplifaer 
*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ArduinoJson.h>

#include <Arduino.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"


// To run, set your ESP8266 build to 160MHz, and include a SPIFFS of 512KB or greater.
// Use the "Tools->ESP8266/ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
// Then upload the sketch normally.  

// pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;


// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();


}



#define t 1;  // сколько минут таймер

String weatherMain = "";
String weatherDescription = "";
String weatherLocation = "";
String country;
int humidity;
int pressure;
float temp;
float tempMin, tempMax;
int clouds;
float windSpeed;
String date;
String currencyRates;
String weatherString;

long period;
int offset=1,refresh=0;
int pinCS = 0; // Подключение пина CS
int numberOfHorizontalDisplays = 4; // Количество светодиодных матриц по Горизонтали
int numberOfVerticalDisplays = 1; // Количество светодиодных матриц по Вертикали

String decodedMsg;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
//matrix.cp437(true);

int spacer = 2;
int width = 5 + spacer; // Регулируем расстояние между символами

int key=5; //define key D1
int buzzer = 4; //D2 объявляем переменную с номером пина, на который мы подключили пьезоэлемент
bool flagkey = 0;


void setup(void) {

  Serial.begin(115200);                           // Дебаг
  WiFi.mode(WIFI_OFF); 
  delay(1000);

  SPIFFS.begin();
  Serial.printf("Sample MP3 playback begins...\n");

  file = new AudioFileSourceSPIFFS("/pno-cs.mp3");
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
              
  
  pinMode(key,INPUT_PULLUP);
  pinMode(buzzer, OUTPUT); //объявляем пин как выход
  
matrix.setIntensity(0); // Яркость матрицы от 0 до 15


// начальные координаты матриц 8*8
  matrix.setRotation(0, 1);        // 1 матрица
  matrix.setRotation(1, 1);        // 2 матрица
  matrix.setRotation(2, 1);        // 3 матрица
  matrix.setRotation(3, 1);        // 4 матрица

}

// =======================================================================
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int m = t;
int s = 0;


// =======================================================================
void loop(void) {

DisplayTime();

    if (mp3->isRunning())   
      {
         if (!mp3->loop()) 
            {
        OpenMuz();
            }
      } 

  if (digitalRead(key)==LOW) 
  {
    flagkey=1;
    if (mp3->isRunning())   
      {
        OpenMuz();
      }
  }
  
  if (flagkey) 
    {
      if(millis()-dotTime > 20) {
        dotTime = millis();
        if (s > 0)
          {
            s--;
          }
        else 
          if (m > 0)
            {
              m--;
              s = 59;
            }
          else 
            {
              flagkey=0;
              m = t;
              s = 0;
              mp3->begin(id3, out);
            }
        }
    }
}

// =======================================================================

void DisplayTime()
{
    matrix.fillScreen(LOW);
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали

    
    if(s & 1){matrix.drawChar(14, y, (String(" "))[0], HIGH, LOW, 1);} //каждую четную секунду печатаем двоеточие по центру (чтобы мигало)
    else{matrix.drawChar(14, y, (String(":"))[0], HIGH, LOW, 1);}
    
    String min1 = String (m/10);
    String min2 = String (m%10);
    String sec1 = String (s/10);
    String sec2 = String (s%10);
    int xm = 2;
    int xs = 19;

    matrix.drawChar(xm, y, min1[0], HIGH, LOW, 1);
    matrix.drawChar(xm+6, y, min2[0], HIGH, LOW, 1);
    matrix.drawChar(xs, y, sec1[0], HIGH, LOW, 1);
    matrix.drawChar(xs+6, y, sec2[0], HIGH, LOW, 1);  

  
    matrix.write(); // Вывод на дисплей
}

// =======================================================================

void OpenMuz()
{

  Serial.print(" Stop              FreeHeap - " );
  Serial.println(ESP.getFreeHeap() );
                
            mp3->stop();
            id3->close();
            file->close();
            delete mp3;
            delete id3;
            delete file;
            delete out;
            mp3 = NULL;
            id3 = NULL;
            file = NULL;
            out = NULL;
      
            file = new AudioFileSourceSPIFFS("/pno-cs.mp3");
            id3 = new AudioFileSourceID3(file);
            id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
            out = new AudioOutputI2SNoDAC();
            mp3 = new AudioGeneratorMP3(); 
        //    ESP.restart();                                // перезагружаем модуль

  Serial.print("               FreeHeap - " );
  Serial.println(ESP.getFreeHeap() );

}

