/*
Подключения:
NodeMCU    -> Matrix
MOSI-D7-GPIO13  -> DIN
CLK-D5-GPIO14   -> Clk
GPIO0-D3        -> LOAD
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ArduinoJson.h>

#define t 1;  // сколько минут минуты

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

int key=2; //define key D4
int buzzer = 4; //D2 объявляем переменную с номером пина, на который мы подключили пьезоэлемент
bool flag = 0;


void setup(void) {

  pinMode(key,INPUT_PULLUP);
  pinMode(buzzer, OUTPUT); //объявляем пин как выход
  
matrix.setIntensity(0); // Яркость матрицы от 0 до 15




// начальные координаты матриц 8*8
  matrix.setRotation(0, 1);        // 1 матрица
  matrix.setRotation(1, 1);        // 2 матрица
  matrix.setRotation(2, 1);        // 3 матрица
  matrix.setRotation(3, 1);        // 4 матрица


  Serial.begin(115200);                           // Дебаг



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

  if (digitalRead(key)==LOW) {flag=1;}
  if (flag) 
    {
      if(millis()-dotTime > 100) {
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
              flag=0;
              m = t;
              s = 0;
              tone(buzzer, 500); //включаем на 500 Гц
              delay(1000); //ждем 1000 Мс
              tone(buzzer, 0); //включаем на 500 Гц
            }
        }
    }
}

// =======================================================================
void DisplayTime(){
    //updateTime();
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
void DisplayText(String text){
    matrix.fillScreen(LOW);
    for (int i=0; i<text.length(); i++){
    
    int letter =(matrix.width())- i * (width-1);
    int x = (matrix.width() +1) -letter;
    int y = (matrix.height() - 8) / 2; // Центрируем текст по Вертикали
    matrix.drawChar(x, y, text[i], HIGH, LOW, 1);
    matrix.write(); // Вывод на дисплей
    
    }

}





String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30-1;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70-1;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
