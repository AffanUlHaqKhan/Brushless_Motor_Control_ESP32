#include <driver/adc.h>
#include "esp_adc_cal.h"
#include <ESP32_Servo.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int input;
int inputvoltage;
int temp;

const unsigned char bar [] = {
0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00
};
const unsigned char BT_con [] = {
0x03, 0x00, 0x03, 0xC0, 0x03, 0xE0, 0x03, 0x78, 0x73, 0x3E, 0x7F, 0x3E, 0x1F, 0x78, 0x07, 0xE0,
0x07, 0xE0, 0x0F, 0xF0, 0x3F, 0x7C, 0x7B, 0x1E, 0x03, 0x7C, 0x03, 0xF0, 0x03, 0xE0, 0x03, 0x80
};

const unsigned char BT_discon [] = {
0x83, 0x01, 0x43, 0xC3, 0x23, 0xE6, 0x13, 0x7C, 0x7B, 0x3E, 0x7F, 0x3E, 0x1F, 0x78, 0x07, 0xE0,
0x07, 0xE0, 0x0F, 0xF0, 0x3F, 0x7C, 0x7B, 0x1E, 0x33, 0x7C, 0x63, 0xF4, 0xC3, 0xE2, 0x83, 0x81
};

const unsigned char battery [] = {
0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xFF, 0xC0, 0x60, 0x00, 0x00, 0xC0,
0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0,
0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0, 0x6F, 0x7B, 0xDE, 0xF0,
0x60, 0x00, 0x00, 0xC0, 0x7F, 0xFF, 0xFF, 0xC0, 0x7F, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00
};


#define SERVICE_UUID           "5e15dea1-664e-4e2c-93de-478c3fa3341d"
#define CHARACTERISTIC_UUID_RX "b03bd4f3-0e15-4efc-9662-20392f33a60f"
#define CHARACTERISTIC_UUID_TX "86a81b84-d13d-4990-875d-8a99e973fe24"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++)
        {
          inputvoltage = (int(rxValue[i]));
        }
        Serial.println("BLE Value: " + inputvoltage);
      }


    }
};

Servo motor;
int buttonState1 = 0;
int lastButtonState1 = 0;

int buttonState2 = 0;
int lastButtonState2 = 0;

//#define InPin 33
int x;

void setup() {
  Serial.begin(9600);
  input = 0;
  inputvoltage = 90;
  temp = 0;
  motor.attach(2);

  motor.writeMicroseconds(2000);
  delay(1000);
  motor.writeMicroseconds(1050);
  delay(2000);

  input = map(inputvoltage, 0, 1023, 1050, 2000);
  motor.write(input);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  
  display.clearDisplay();
  display.drawBitmap(10, 40, bar, 8, 16, WHITE);
  display.display();

  BLEDevice::init("EKKO");
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                    
  pCharacteristic->addDescriptor(new BLE2902());
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
                                     
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start();
  
  pServer->getAdvertising()->start();
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_5,ADC_ATTEN_6db);

}

void loop() {
  
  //float x = adc1_get_voltage(ADC1_CHANNEL_5);
  //adc1_config_width(ADC_WIDTH_BIT_12);
    //adc1_config_channel_atten(ADC1_CHANNEL_5,ADC_ATTEN_DB_0);
  //int val = adc1_get_raw(ADC1_CHANNEL_5);
  //float x = analogRead(InPin);
  //Serial.println(x);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,2);
   // display.println(x);

  
  //if(x>2000)
  //{   
              
              //if( x >= 2810)
              //{   display.println("100%");
              //}
              //if(x<2810 && x >= 2780)
              //{   display.println("90%");
              //}
              //if(x<2780 && x >= 2750)
              //{   display.println("80%");
              //}
              //if(x<2750 && x >= 2720)
              //{   display.println("70%");
              //}
              //if(x<2720 && x >= 2690)
              //{   display.println("60%");
              //}
              //if(x<2690 && x >= 2660)
              //{   display.println("50%");
              //}
              //if(x<2660 && x >= 2630)
              //{   display.println("40%");
              //}
              //if(x<2630 && x >= 2600)
              //{   display.println("30%");
              //}
              //  if(x<2600 && x >= 2570)
              //{   display.println("20%");
              //}
            //  if(x<1700 && x >= 1600)
            //  {   display.println("15%");
            //  }
            //   if(x<1600 && x >= 1500)
            //  {   display.println("10%");
            //  }
            //    if(x<1500 && x >= 1400)
            //  {   display.println("5%");
            //  }
            //    if(x<1400 )
            //  {   display.println("0%");
            //  }
              
            //     if(x<1700 && x >= 1600)
            //  {   display.println("35%");
            //  }
            //       if(x<1600 && x >= 1500)
            //  {   display.println("30%");
            //  }
            //        if(x<1500 && x >= 1400)
            //  {   display.println("25%");
            //  }
            //         if(x<1400 && x >= 1300)
            //  {   display.println("20%");
            //  }
            //          if(x<1300 && x >= 1200)
            //  {   display.println("15%");
            //  }
            //            if(x<1200 && x >= 1100)
            //  {   display.println("10%");
            //  }
            //             if(x<1100 && x >= 1000)
            //  {   display.println("5%");
            //  }
              
              
              
            
              
              if(deviceConnected == true)
              {
                
                display.drawBitmap(90, 0,  BT_con, 16, 16, WHITE);
                display.setCursor(10,22);
              display.println("L");
              display.setCursor(90,22);
              display.println("H");
              if(inputvoltage < 95 && inputvoltage >= 90)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 100 && inputvoltage >= 95)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 105 && inputvoltage >= 100)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 110 && inputvoltage >= 105)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
              }
              if(inputvoltage < 115 && inputvoltage >= 110)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 120 && inputvoltage >= 115)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 125 && inputvoltage >= 120)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 130 && inputvoltage >= 125)
              {
                 
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
                display.drawBitmap(80, 40, bar, 8, 16, WHITE);
              }
               if(inputvoltage <= 135 && inputvoltage >= 130)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
                display.drawBitmap(80, 40, bar, 8, 16, WHITE);
                display.drawBitmap(90, 40, bar, 8, 16, WHITE);
              }
              delay(100);
              input = map((inputvoltage-25), 0, 1023, 1050, 2000);
              delay(100);
              motor.write(input);
              delay(100);
              Serial.println("BLE Value after setting: ");
              Serial.println(inputvoltage);
              Serial.println("BLE Input Augmented value: ");
              Serial.println(input);
              }

              
              else
              {
                display.drawBitmap(90, 0,  BT_discon, 16, 16, WHITE); 
                
            
              display.setCursor(10,22);
              display.println("L");
              display.setCursor(90,22);
              display.println("H");
              if(inputvoltage < 95 && inputvoltage >= 90)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 100 && inputvoltage >= 95)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 105 && inputvoltage >= 100)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 110 && inputvoltage >= 105)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
              }
              if(inputvoltage < 115 && inputvoltage >= 110)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 120 && inputvoltage >= 115)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 125 && inputvoltage >= 120)
              {
                
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
              }
              if(inputvoltage < 130 && inputvoltage >= 125)
              {
                 
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
                display.drawBitmap(80, 40, bar, 8, 16, WHITE);
              }
               if(inputvoltage <= 135 && inputvoltage >= 130)
              {
                display.drawBitmap(10, 40, bar, 8, 16, WHITE);
                display.drawBitmap(20, 40, bar, 8, 16, WHITE);
                display.drawBitmap(30, 40, bar, 8, 16, WHITE);
                display.drawBitmap(40, 40, bar, 8, 16, WHITE); 
                display.drawBitmap(50, 40, bar, 8, 16, WHITE);
                display.drawBitmap(60, 40, bar, 8, 16, WHITE);
                display.drawBitmap(70, 40, bar, 8, 16, WHITE);
                display.drawBitmap(80, 40, bar, 8, 16, WHITE);
                display.drawBitmap(90, 40, bar, 8, 16, WHITE);
              }
               

  
            
              buttonState1 = touchRead(4);
              if (buttonState1 != lastButtonState1) {
                if (buttonState1 == 1 and inputvoltage < 135) {
                  
                    inputvoltage = inputvoltage + 5;
                 
                  //display.clearDisplay();
                  //display.setTextSize(2);
                  //display.setTextColor(WHITE);
                  //display.setCursor(0,20);
                  //display.println("Value: " + String(inputvoltage));
                } else {
                  inputvoltage = inputvoltage;
                }
                delay(50);
              }
              lastButtonState1 = buttonState1;
            
            
              buttonState2 = touchRead(15);
              if (buttonState2 != lastButtonState2) {
                if (buttonState2 == 1  and inputvoltage > 90 ) {
                  
                    inputvoltage = inputvoltage - 5;
                  //display.clearDisplay();
                  //display.setTextSize(2);
                  //display.setTextColor(WHITE);
                  //display.setCursor(0,20);
                  //display.println("Value: " + String(inputvoltage));
                  //display.display();
                  
                } else {
                  inputvoltage = inputvoltage;
                }
                delay(50);
              }
              lastButtonState2 = buttonState2;
              input = map((inputvoltage+15), 0, 1023, 1050, 2000);
              motor.write(input);
              Serial.println("Manual Value after setting: ");
              Serial.println(inputvoltage);
              Serial.println("Manual Input Augmented value: ");
              Serial.println(input);
            //  Serial.println(input);
              }
              
              //display.drawBitmap(50, 0, battery, 32, 16, WHITE);
            //  display.setTextSize(2);
            //  display.setCursor(0,40);
            //  display.println("FREQ: " + String(inputvoltage));
            // display.drawBitmap(10, 40, bar, 8, 16, WHITE);
            //  display.println("");
            
              
  //}
//else
//{
  
  //display.println("0%");
  //display.setCursor(10,22);
  //display.println("Please");
  //display.setCursor(10,40);
  //display.println("Charge!");
  //}
  
display.display();
  //x = float(analogRead(InPin));

  //float VBAT = (127.0f/100.0f) * 3.30f * x / 4096.0f;
  //Serial.println((1-((0.9-VBAT)/0.3)) * 100);
}
