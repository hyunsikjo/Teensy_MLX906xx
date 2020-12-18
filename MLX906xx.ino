#include "string.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include "SparkFun_MLX90632_Arduino_Library.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 4
//#define OLED_RESET LED_BUILTIN

#define SensorSel 2 // 0 MLX90614, 1 MLX90632, 2 MLX90640

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
MLX90632 myTempSensor;

volatile byte state = LOW;
volatile byte state1 = LOW;
volatile int ButtonLongPress = LOW;
volatile int ButtonShortPress = LOW;
volatile unsigned long Current_HIGH;
volatile unsigned long Current_LOW;

const int irReceiverPin = 2;
const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

float SensorTemp;
float objectTemp;
float objectTempF;
float mlx90640To[768];
paramsMLX90640 mlx90640;

void setup() 
{
  Wire.begin();
  
  #if   SensorSel == (0 || 1)
    Serial.begin(9600);
    myTempSensor.begin();
    mlx.begin(); //MLX90614
  
    //Interrupt
    pinMode(irReceiverPin, INPUT_PULLUP); //2pin
    attachInterrupt(digitalPinToInterrupt(2), IntState, CHANGE); //CHANGE
    
    //LCD
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //어드레스 확인 
    { 
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
    display.display();
    delay(2000);
    
    display.clearDisplay();
    display.drawCircle(display.width()/2, display.height()/2, 25, SSD1306_WHITE);
    display.display();
    display.setTextColor(WHITE);
    display.setTextSize(2); //1~8
    
    for(int i = 0; i < 3; i++)
    {
      display.setCursor((display.width()/2 -15) + i*10, display.height()/2 - 10);
      display.println(".");
      display.display();
      delay(1000);
    }

  #elif SensorSel == 2
    Wire.setClock(400000); //Increase I2C clock speed to 400kHz
    Serial.begin(921600); // ESP32
//    Serial.begin(115200); //Fast serial as possible
    
    while (!Serial); //Wait for user to open terminal
    //Serial.println("MLX90640 IR Array Example");
  
    if (isConnected() == false)
    {
      Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
      while (1);
    }
  
    //Get device parameters - We only have to do this once
    int status;
    uint16_t eeMLX90640[832];
    status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
    if (status != 0)
    {
      Serial.println("Failed to load system parameters");
    }
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    
    if (status != 0)
    {
      Serial.println("Parameter extraction failed");
    }
  
    //Once params are extracted, we can release eeMLX90640 array
    //MLX90640_SetRefreshRate(MLX90640_address, 0x01); //Set rate to 1Hz
    MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 2Hz
    //MLX90640_SetRefreshRate(MLX90640_address, 0x03); //Set rate to 4Hz
    //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 64Hz
    
  #endif
}

void loop() 
{  
  
  #if   SensorSel == 0 //MLX90614
    objectTemp = mlx.readObjectTempC();    //Get the temperature of the object we're looking at
    SensorTemp = mlx.readAmbientTempC();    //Get the temperature of the sensor
    delay(1);
    
  #elif SensorSel == 1 //MXL90632
    objectTempF = myTempSensor.getObjectTempF();  //Get the temperature of the object we're looking at
    objectTemp = myTempSensor.getObjectTemp();    //Get the temperature of the object we're looking at
    SensorTemp = myTempSensor.getSensorTemp();    //Get the temperature of the sensor
  
  #elif SensorSel == 2 //MLX90640
    long startTime = millis();
    for (byte x = 0 ; x < 2 ; x++)
    {
      uint16_t mlx90640Frame[834];
      int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
  
      float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
      float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
  
      float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
  //    float emissivity = 0.95;
      float emissivity = 1;
  
      MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    }
    long stopTime = millis();
  
    for (int x = 0 ; x < 768 ; x++)
    {
      //if(x % 8 == 0) Serial.println();
      Serial.print(mlx90640To[x], 2);
      Serial.print(",");
    }
    Serial.println("");   
  #endif

  #if   SensorSel == (0 || 1)
    Temp_Display();
  
    if(ButtonShortPress == HIGH) 
    {
      delay(4000);
      ButtonShortPress = LOW;
    }
    if(ButtonLongPress == HIGH) 
    {
      Temp_Display();
      ButtonLongPress = LOW;
    }
    Serial.print(objectTemp);
    Serial.print(",");
    Serial.print(SensorTemp);
    Serial.print(",");
    Serial.print("*");   
  #endif
}

void Temp_Display() 
{  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 10);
  
  if(state1 == LOW) 
  {
    display.println("- Object Temp -");
    display.setTextSize(3);
    display.setCursor(18,display.height()/2);
    display.println(objectTemp); 
    display.setTextSize(2);
    display.setCursor(110, 30);
    display.println("C");
    display.display();   
  }
  else 
  {
    display.println("- Ambient Temp -");
    display.setTextSize(3);
    display.setCursor(18,display.height()/2);
    display.println(SensorTemp); 
    display.setTextSize(2);
    display.setCursor(110, 30);
    display.println("C");
    display.display();     
  }  
}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

void MLX90614_Temp_Display() 
{
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
  Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
  Serial.println();
}

void IntState() 
{ 
  if(digitalRead(irReceiverPin) == LOW) 
  {
    Current_HIGH = millis();
    state = HIGH;
  }
  if(digitalRead(irReceiverPin) == HIGH && state == HIGH) 
  {
    Current_LOW = millis();
    
    if((Current_LOW - Current_HIGH) > 1 && (Current_LOW - Current_HIGH) < 300) 
    {
      ButtonShortPress = HIGH;
      state = LOW;    
    }
    else if((Current_LOW - Current_HIGH) >= 300 && (Current_LOW - Current_HIGH) < 4000) 
    {
      ButtonLongPress = HIGH;
      state = LOW;  
      state1 = !state1;        
    }
  }  
}
