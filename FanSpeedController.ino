/*
Temperature Range : 25 - 35 C
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT11.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

#define POT A3
#define LDR A2
#define FAN 3
#define DHT 2
#define MODESW 4
#define LED_BLUE 10
#define LED_GREEN 9

class DisplayControl {
  public:
    void printText(char*, int, int);
    void printDegree(int, int);
    void printLevel(int);
    void printDHTData(int, int);
    void changeMode(bool);
    void init();
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT11 dht11(DHT);
DisplayControl oled;

int temperature_reading = 0, humidity_reading = 0, previous_temp = 0, previous_humidity = 0;
float light_intensity = 0;
bool state = true;

void setup() {
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  pinMode(POT, INPUT);
  pinMode(LDR, INPUT);
  pinMode(FAN, OUTPUT);
  pinMode(MODESW, INPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  oled.init();
}

void loop() {
  if(digitalRead(MODESW) == 1) {
    state = !state;
    oled.changeMode(state);
    oled.printLevel(0);
    delay(1000);
  }

  light_intensity = analogRead(LDR);
  if(light_intensity > 300) {
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
  } else {
    analogWrite(LED_GREEN, int(calculatePerNum(675 - light_intensity, 675, 255)));
    analogWrite(LED_BLUE, int(calculatePerNum(675 - light_intensity, 675, 255)));
  }

  if(!state){
    oled.printLevel(int(calculatePerNum(analogRead(POT), 685, 52)));
    analogWrite(FAN, int(calculatePerNum(analogRead(POT), 687, 255)));
    Serial.println(analogRead(POT));
  }

  int result = dht11.readTemperatureHumidity(temperature_reading, humidity_reading);

  if (result == 0 && (previous_temp != temperature_reading || previous_humidity != humidity_reading)) {
    oled.printDHTData(temperature_reading, humidity_reading);
    if(state) {
      oled.printLevel(int(calculatePerNum(temperature_reading - 25, 10, 52))); //minTemp = 25, maxTemp: 35 (25 - 25: 10), default
      analogWrite(FAN, int(calculatePerNum(temperature_reading - 25, 10, 255)));
    }
    previous_humidity = humidity_reading;
    previous_temp = temperature_reading;
  } else if(result != 0) {
      Serial.println(DHT11::getErrorString(result));
  }
}

float calculatePerNum(int what, int per, int of){
  float temp = what;
  temp /= per;
  temp = temp * of;
  return temp;
}

void DisplayControl::printText(char* text, int x, int y) {

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(x, y);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.write(' ');
  for(int i = 0; i < strlen(text); i++){
    display.write(text[i]);
  }

  display.display();
}

void DisplayControl::printDegree(int x, int y) {
  display.drawCircle(x, y, 2, SSD1306_WHITE);
  display.display();
}

void DisplayControl::printLevel(int level){
  static int prevlevel = level;
  if(prevlevel == level){
    return;
  }
  display.fillRect(71, 1, prevlevel, 5, SSD1306_INVERSE);
  display.fillRect(71, 1, level, 5, SSD1306_INVERSE);
  display.display();
  prevlevel = level;
}

void DisplayControl::init(){
  display.clearDisplay();
  oled.printText("FAN SPEED:", 0, 0);
  display.drawRect(70, 0, 54, 7, SSD1306_WHITE);
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  oled.printText("TEMPERATURE:   .0  C", 0, 15);
  oled.printDegree(113, 16);
  oled.printText("HUMIDITY   :   .0 %", 0, 25);
  oled.printText("MODE : AUTOMATIC", 0, 35);
  oled.printLevel(0);
}

void DisplayControl::printDHTData(int temperature, int humidity){
  display.fillRect(83, 15, 12, 20, SSD1306_BLACK);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(90, 15);
  display.write((char)((temperature % 10) + 48));
  temperature /= 10;
  display.setCursor(84, 15);
  display.write((char)((temperature % 10) + 48));
  display.setCursor(90, 25);
  display.write((char)((humidity % 10) + 48));
  humidity /= 10;
  display.setCursor(84, 25);
  display.write((char)((humidity % 10) + 48));
  display.display();
}

void DisplayControl::changeMode(bool state){
  display.fillRect(40, 35, 80, 20, SSD1306_BLACK);
  if(state){
    oled.printText("AUTOMATIC", 42, 35);
  } else {
    oled.printText("MANUAL", 42, 35);
  }
}