/*********************************************************************************
CODOS AKA CO2 
Un sistema de bajo coste basado en Arduino NANO, OLED SS1306, CM1106 para la detección del CO2 
y temperatura en el aula
*********************************************************************************/

char VERSION_SW[6] = "v1.10";
#include "Wire.h"                       // Librería Wire para el soporte del protocolo i2c

#include <SoftwareSerial.h>                                // Remove if using HardwareSerial or Arduino package without SoftwareSerial support

#define RX_PIN 2                                          // Rx pin which the CM1106 Tx pin is attached to
#define TX_PIN 4                                          // Tx pin which the CM1106 Rx pin is attached to
#define BAUDRATE 9600                                     // Device to CM1106 Serial baudrate (should not be changed)
#define CALIBRATION_PIN 3                                 // Long press to force calibrate (Use a PIN that supports interrupts)

#define PREHEATING_TIME 30000                             // Por especificaciones tiene un PREHEATING Time de 30 segundos

#define CALIBRATION_VALUE 415                             // Clean air CO2ppm

const int CO2_safe_level = 800;        // Se debe especificar un valor máximo de CO2 seguro
const int CO2_alarm_level = 1200;       // Se debe especificar un valor máximo de CO2 de alarma


SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to CM1106 serial

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CO2_sensor_present true         // Debe seleccionarse si este sensor está o no presente
#define OLED_present true              // Debe seleccionarse si la pantalla está o no presente
#define traffic_lights_present true     // Debe seleccionarse si se conectará el semáforo o no (el sensor de CO2 debe estar presente también en ese caso)

// Pines Digitales que he usado en el arduino NANO
#define verde 6
#define amarillo 7
#define rojo 8
#define BUILTIN_LED 13

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Imagenes generadas a partir de LCD Assistant (http://en.radzio.dxp.pl/bitmap_converter/)
// con iconos descargados de https://iconarchive.com/ 
const unsigned char PROGMEM happy_icon [] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF0, 0x0F, 0xFC, 0x00, 0x00,
0x00, 0x03, 0xF0, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00,
0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0xF0, 0x03, 0xFC, 0x3C, 0x00, 0x0F, 0xFC,
0xF0, 0x00, 0x0F, 0x3C, 0x3C, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x0C, 0x3F, 0xFC, 0x00, 0x03, 0xC3,
0xC0, 0x00, 0x3C, 0x3C, 0x3C, 0x00, 0x03, 0x03, 0xC0, 0x00, 0x30, 0x30, 0x0C, 0x00, 0x0F, 0x03,
0xC0, 0x00, 0xF0, 0x30, 0x0C, 0x00, 0x0C, 0x03, 0xF0, 0x00, 0xC0, 0xF0, 0x0F, 0x00, 0x3C, 0x0F,
0xF0, 0x03, 0xC3, 0xC0, 0x03, 0xC0, 0x30, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
0xF3, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xCF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x0C, 0x03, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x30,
0x0F, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0xF0, 0x03, 0xC0, 0x3C, 0x00, 0x00, 0x3C, 0x03, 0xC0,
0x00, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0x00, 0x00, 0x3C, 0x00, 0x3F, 0xFC, 0x00, 0x3C, 0x00,
0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x0F, 0xC0, 0x00,
0x00, 0x00, 0x3F, 0xF0, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM Angry_icon [] = {
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF0, 0x0F, 0xFC, 0x00, 0x00,
0x00, 0x03, 0xF0, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0xF3, 0xF0, 0x00, 0x00, 0x0F, 0xCF, 0x00,
0x03, 0xC0, 0x3F, 0x00, 0x00, 0xFC, 0x03, 0xC0, 0x0F, 0x00, 0x03, 0xF0, 0x0F, 0xC0, 0x00, 0xF0,
0x0C, 0x00, 0x00, 0x30, 0x0C, 0x00, 0x00, 0x30, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x3C,
0x30, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x0C, 0x30, 0x03, 0x00, 0xC0, 0x03, 0x00, 0xC0, 0x0C,
0xF0, 0x03, 0x00, 0xC0, 0x03, 0x00, 0xC0, 0x0F, 0xF0, 0x03, 0x00, 0xC0, 0x03, 0x00, 0xC0, 0x0F,
0xC0, 0x03, 0xC3, 0xC0, 0x03, 0xC3, 0xC0, 0x03, 0xC0, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x03,
0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
0x30, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x0F, 0xF0, 0x0F, 0xF0, 0x00, 0x0C,
0x3C, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x3C, 0x0C, 0x00, 0xF0, 0x00, 0x00, 0x0F, 0x00, 0x30,
0x0F, 0x03, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0,
0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00,
0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x0F, 0xC0, 0x00,
0x00, 0x00, 0x3F, 0xF0, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM Surprised_icon [] = {
0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x30, 0x3F, 0xF0, 0x0F, 0xF0, 0x3C, 0x00,
0x00, 0xC0, 0xF0, 0x30, 0x0C, 0x3F, 0x0F, 0x00, 0x03, 0xC0, 0xC0, 0x00, 0x00, 0x03, 0x03, 0xC0,
0x0F, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0xC0, 0x0C, 0x00, 0x0F, 0xC0, 0x03, 0xF0, 0x00, 0x30,
0x0C, 0x00, 0x0F, 0xC0, 0x03, 0xF0, 0x00, 0x30, 0x3C, 0x00, 0x0F, 0xC0, 0x03, 0xF0, 0x00, 0x3C,
0x30, 0x00, 0x0F, 0xC0, 0x03, 0xF0, 0x00, 0x0C, 0x30, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x0C,
0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
0x30, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x0C,
0x30, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x0C,
0x3C, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x3C, 0x0C, 0x00, 0x0F, 0x00, 0x00, 0xF0, 0x00, 0x30,
0x0F, 0x00, 0x03, 0xFF, 0xFF, 0xC0, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0xC0,
0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00,
0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
0x00, 0x03, 0xF0, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x3F, 0xC0, 0x03, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x03, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM Increibles_logo [] = {
0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xF8, 0x3F, 0xC0, 0x00, 0x00,
0x00, 0x00, 0x7F, 0xF0, 0x1E, 0x1E, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xC0, 0x1E, 0x00, 0xC0, 0x00,
0x00, 0x1F, 0xFF, 0xC0, 0x1E, 0x00, 0x38, 0x00, 0x00, 0x7F, 0xFF, 0xF0, 0x3E, 0x00, 0x0F, 0x00,
0x01, 0xFF, 0xFF, 0xF8, 0xFC, 0x00, 0x01, 0x80, 0x03, 0xFF, 0xF8, 0x7F, 0xF0, 0x00, 0x00, 0xC0,
0x0F, 0xFF, 0x80, 0x1F, 0xE0, 0x00, 0x00, 0xF0, 0x1F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78,
0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E,
0x3F, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x1F,
0xFE, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x1F, 0xFE, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x7F,
0xFC, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0xFF,
0x3C, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0xFE, 0x3C, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x01, 0xFE,
0x3E, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x0F, 0xFE, 0x1E, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x3F, 0xF8,
0x0F, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0xFF, 0xF0, 0x03, 0xC0, 0x00, 0x1F, 0xFC, 0x3F, 0xFF, 0xC0,
0x01, 0xE0, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x78, 0x00, 0x1F, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0x1E, 0x00, 0x1F, 0xFF, 0xFF, 0xF8, 0x00, 0x00, 0x03, 0xC0, 0x1F, 0xFF, 0xFF, 0xC0, 0x00,
0x00, 0x00, 0x7C, 0x1F, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

unsigned long getDataTimer = 0;

const int OFFSET_CALIBRATION_TEMP = -6; // Valor obtenido de comparar con un termometro mas fiable que tengo en casa.. NO ES MUY FIABLE
const int OFFSET_CALIBRATION_PPM = 0; // Esta calibracion esta inventada (PENDIENTE) porque supongo que debiera ser unas tablas en funcion de temp y humedad asi que tomo un valor conservador para intentar no generar falsa alarma en el aula.
int p1 = 0; // Variable donde calculo el porcentaje sobre el limite establecido como ROJO
char leyend[15] = "12345678901234"; // Variable donde pongo la leyenda que se pinta en la barra de progreso en funcion del estado
char leyendGREEN[15] = "AULA OK"; 
char leyendRED[15] = "VENTILAR"; 
char leyendYELLOW[15] = "LIMITE KO"; 
char leyendRECOVERY[15] = "RECOVERY"; 
bool doCalibration = false;
volatile unsigned long buttonPressTimestamp = 0;

void(* resetFunc) (void) = 0;//declare reset function at address 

void setup() {
  Serial.begin(115200);
  bool status;

  speaker();
  
  Serial.println("Sensor CM1106 y OLED 0.96");

  Wire.begin(); //Inialize I2C Hardware
  
  if (OLED_present){
      // SSD1306_SWITCHCAPVCC = generate display voltage
      if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
      }     
      // Clear the buffer
      display.clearDisplay();

      // Draw a single pixel in white
      display.drawPixel(10, 10, SSD1306_WHITE);
      display.setCursor(0,0);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.println(VERSION_SW);

      // Show the display buffer on the screen. You MUST call display() after
      // drawing commands to make them visible on screen!
      display.display();
      delay(2000);
      Serial.println("OLED Display begin executed");    
  }

  if (CO2_sensor_present){
    mySerial.begin(BAUDRATE);                               // (Uno example) device to CM1106 serial start   
 } 

 if (OLED_present) {
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); 
    int percentage; 
    while (millis() < PREHEATING_TIME) {
      percentage = millis()*100/PREHEATING_TIME;
      display.clearDisplay();
      if (percentage < 10) {
        display.drawBitmap(32,0,Increibles_logo,64,32,1);
      } else {
        display.setCursor(0,0);             // Start at top-left corner
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.println(F("PREHEAT..."));
        display.setCursor(60,18);             // Start at top-left corner
       
        drawPercentbar( 1, 17, SCREEN_WIDTH-10, 15,percentage, false, leyend);       
      }
      display.display();
      delay(1000);       
    }
 }
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  pinMode(CALIBRATION_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CALIBRATION_PIN), buttonPressISR, CHANGE);
  pinMode(verde, OUTPUT);
  pinMode(amarillo, OUTPUT);
  pinMode(rojo, OUTPUT);  
  // Apagar los led al empezar
  digitalWrite(verde, LOW);
  digitalWrite(amarillo, LOW);
  digitalWrite(rojo, LOW);
}

int ppm_uart = 0; // Variable donde vuelco el valor CO2 obtenido del CM1106

// Tipos y variables para controlar el estado del semaforo y las acciones asociadas
typedef enum {
  GREEN = 1, 
  YELLOW = 2, 
  RED = 3
} SEMAPHORE_TYPE;

SEMAPHORE_TYPE currentState = GREEN;
SEMAPHORE_TYPE lastState = RED;
const int HISTERESIS = 30;

bool showImage = false; // Variable que indicara si hay que mostrar imagen de smiley
const unsigned long NUM_CYCLES_SHOW_DATA = 10; // muestro datos lecturas durante 10 lecturas ppm
const unsigned long NUM_CYCLES_IMAGE = 2; // muestro smiley durante dos lecturas ppm en segundo plano
unsigned long cycles = 0;
const unsigned long recoveryResetTimeout = 600000;
unsigned long measureUpdatedTimeTag=0;
bool recoveryMaked=false;
bool buttonPressed = false;

void buttonPressISR() {
  if (digitalRead(CALIBRATION_PIN) == LOW) { // PRESS
    buttonPressed = true;
    buttonPressTimestamp = millis();
  } else {                                   // RELEASE
    if (buttonPressed) { // El botón se había pulsado antes
      unsigned long now;
      now = millis();
      if ( now - buttonPressTimestamp > 2000 ) {
        doCalibration = true;
      } 
      buttonPressed = false;
    }
  }
}

int cm1106_co2ppm() {
  static byte cmd[4] = {0x11, 0x01, 0x01, 0xED};
  static byte response[8] = {0};
  int ppm;

  mySerial.write(cmd, 4);
  mySerial.readBytes(response, 8);

  unsigned int responseHigh = (unsigned int) response[3];
  unsigned int responseLow = (unsigned int) response[4];

  ppm = (256 * responseHigh) + responseLow;
  Serial.print("CO2: "); Serial.print(ppm); Serial.println("ppm");

  return ppm;
}

void cm1106_clearSerial() {
    while (mySerial.available()) { mySerial.read(); };
}

byte cm1106_checksum(byte *cmd) {
    int length,
        i,
        accum = 0;
    
    // head + cmd + length + data1...datan
    length = 2 + cmd[2];

    for (i=0; i<length; i++) {
        accum += cmd[i];
    }

    return ( 256 - (accum % 256) );
}

void cm1106_calibrate(int ppm) {
    static byte cmd[6] = { 0x11, 0x03, 0x03, 0x00, 0x00, 0x00};
    static byte response[4] = {0};

    cmd[3] = ppm / 256;;
    cmd[4] = ppm % 256;
    cmd[5] = cm1106_checksum(cmd);

    cm1106_clearSerial();
    mySerial.write(cmd, 6);
    mySerial.readBytes(response, 4);    
    Serial.print("Calibrating sensor to "); Serial.print(ppm); Serial.println("ppm.");
}


void loop() {
  if (measureUpdatedTimeTag==0) measureUpdatedTimeTag = millis();
  if (CO2_sensor_present) {
    
    if (millis() - getDataTimer >= 2000) {
      getDataTimer = millis();
      int ppm_uart_read = cm1106_co2ppm()+OFFSET_CALIBRATION_PPM;
      bool newData = false;
      
      if (ppm_uart != ppm_uart_read) {
        ppm_uart = ppm_uart_read;
        measureUpdatedTimeTag=millis();
        newData = true;
      } else if (millis()-measureUpdatedTimeTag >= recoveryResetTimeout) {
        Serial.print("measureUpdatedTimeTag=");Serial.print(measureUpdatedTimeTag);Serial.print(" millis()=");Serial.println(millis());
        measureUpdatedTimeTag=millis();
        recoveryMaked = true;
      }
      
      // Una razon de newData = true tb es que pasemos a mostrar la imagen o no del smiley (pensar otra forma menos chapucera de hacerlo)
      cycles++;
      if ((!showImage) && ((cycles % NUM_CYCLES_SHOW_DATA) == 0)) {
        cycles = 0; 
        showImage = true; 
        newData = true;
      } else if ((showImage) && ((cycles % NUM_CYCLES_IMAGE) == 0)) {
        cycles = 0; 
        showImage = false;
        newData = true;
      } else if (currentState != lastState) {
        cycles = 0;
        showImage = true;
        speaker();
        lastState = currentState;
       // if (recoveryMaked == false) {
        switch (currentState) {
          case GREEN : 
            strcpy(leyend,leyendGREEN); break;
          case RED :
            strcpy(leyend,leyendRED); break;
          case YELLOW :
            strcpy(leyend,leyendYELLOW); break;
        }
       // } else {
       //   strcpy(leyend,leyendRECOVERY);
       // }
        newData = true;
      }
      

      if (newData == true) {
        p1 = ppm_uart*100/CO2_alarm_level;
         
        if (OLED_present){
          if (!showImage) {
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE); 
            display.setCursor(5,3);
            display.drawRect(1, 0, 49, 20, WHITE);
            display.drawRect(50, 0, 78, 20, WHITE);
            display.drawRect(105, 0, 23, 10, WHITE);
            display.setTextSize(1);
            display.print(F("*"));
            display.setTextSize(2);
            display.print(F("C"));
            if (ppm_uart > 1000){
              display.setCursor(55,3);            
            } else {
              display.setCursor(60,3);
            }
            display.print(ppm_uart, DEC);
            display.setTextSize(1); 
            display.setCursor(108,2);
            display.print(F("PPM")); 
            drawPercentbar( 1, 19, 127, 13,p1,true,leyend);
            display.display();
            traffic_lights(ppm_uart);
          } else { traffic_lights(ppm_uart);
          }
        }
         
      } // if (newData == true)
    } // if (millis() - getDataTimer >= 2000)  
  } // if (CO2_sensor_present)

  digitalWrite(BUILTIN_LED, buttonPressed);
  if (doCalibration) {
    doCalibration = false;
    cm1106_calibrate(CALIBRATION_VALUE);
  }

  delay(1000);
}

const int pinBuzzer = 9;

void speaker() {
  tone(pinBuzzer, 494);
  delay(50);
  tone(pinBuzzer, 392);
  delay(150);  
  tone(pinBuzzer, 694);
  delay(150);
  tone(pinBuzzer, 592);
  delay(300);
  tone(pinBuzzer, 894);
  delay(150);
  tone(pinBuzzer, 792);
  delay(300);
  noTone(pinBuzzer);  
}


void drawPercentbar(int x,int y, int width,int height, int progress, bool customLeyend, char *strCustomLeyend)
{
   int ppm = progress;
   progress = progress > 100 ? 100 : progress;
   progress = progress < 0 ? 0 :progress;

   float bar = ((float)(width-4) / 100) * progress; 

   display.drawRect(x, y, width, height, WHITE);
   display.fillRect(x+2, y+2, bar , height-4, WHITE);

   // Display progress text
   if( height >= 13){
      display.setCursor((width/2) -3, y+5 );
      display.setTextSize(1);
      display.setTextColor(WHITE);

      if( progress >=50) {
         display.setTextColor(BLACK, WHITE); // 'inverted' text
      }
      if (!customLeyend) {
        display.print(progress);
        display.print("%");      
      } else {
        switch (currentState) {
          case GREEN :
            display.setTextColor(WHITE);
            display.setCursor((width/2) +10, y+3 );
            break;
          case YELLOW :          
            display.setTextColor(BLACK, WHITE);
            display.setCursor(x+3, y+3 );
            break;
          case RED :          
            display.setTextColor(BLACK, WHITE);
            display.setCursor(x+3, y+3 );
            break;
        }
        display.print(strCustomLeyend);
      }
   }
}

void traffic_lights(int CO2_value){
      
  Serial.println(CO2_value);
  // Condicion con histeresis en bajada de estado
  if (((currentState != GREEN) && (CO2_value <= (CO2_safe_level-HISTERESIS))) ||
     ((currentState == GREEN) && (CO2_value <= CO2_safe_level))){
    // Encender el led verde y apagar el resto
    Serial.println("Parece que el aula no necesita más ventilación de momento");
    digitalWrite(verde, HIGH);
    digitalWrite(amarillo, LOW);
    digitalWrite(rojo, LOW);
    currentState = GREEN;
    if (showImage) {
      display.clearDisplay();
      display.drawBitmap(32,0,happy_icon,64,32,1);
      display.display();
    }
  } 
  // Condicion con histeresis en bajada de estado
  if (((currentState != RED) && ((CO2_value > CO2_safe_level) & (CO2_value < CO2_alarm_level))) ||
     ((currentState == RED) && ((CO2_value > CO2_safe_level) & (CO2_value < (CO2_alarm_level-HISTERESIS))))) {
    // Encender el led amarillo y apagar el resto
    Serial.println("El aire del aula necesitará renovarse pronto");
    digitalWrite(verde, LOW);
    digitalWrite(amarillo, HIGH);
    digitalWrite(rojo, LOW);
    currentState = YELLOW;
    if (showImage) {
      display.clearDisplay();
      display.drawBitmap(32,0,Surprised_icon,64,32,1);
      display.display();
    }
  }
  if (CO2_value >= CO2_alarm_level){
    // Encender el led rojo y apagar el resto
    Serial.println("Habría que ventilar el aula");
    digitalWrite(verde, LOW);
    digitalWrite(amarillo, LOW);
    digitalWrite(rojo, HIGH);
    currentState = RED;
    if (showImage) {
      display.clearDisplay();
      display.drawBitmap(32,0,Angry_icon,64,32,1);
      display.display();
    }       
  }
}
