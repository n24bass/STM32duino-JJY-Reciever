//-------------------------------------------------------------------------------------------
// JJY Receiver for STM32F103 (Blue-pill/STM32duino)
//
// BluePill STM32F103
// RCCM-6181B-JJY module 
// i2c LCD (ST7032i) - https://strawberry-linux.com/catalog/items?code=27001
//-------------------------------------------------------------------------------------------

#include <JJYReceiver.h> // https://github.com/Blue-Crescent/JJYReceiver/
#include <Wire.h>
#include <LCD_ST7032.h> // https://github.com/olkal/LCD_ST7032

#define LED_PIN PC13
#define DATA_PIN PB7 // JJY receiver
// SEL_PIN
// PON_PIN

LCD_ST7032 lcd;
JJYReceiver jjy(DATA_PIN); // ,SEL,PON); // JJYReceiver lib set up.

//

void handle_timer() {
  jjy.delta_tick();
}

void isr_routine() { // pin change interrupt service routine
  jjy.jjy_receive(); 
}

//

void setup() {
  // Serial.begin(115200);

  // i2c LCD
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  Wire.begin();
    
  lcd.begin();
  lcd.setcontrast(30); //contrast value range is 0-63, try 25@5V or 50@3.3V as a starting value
  lcd.clear();
  lcd.setCursor(0,0);

#if defined(TIM2)
  TIM_TypeDef *Instance = TIM2;
#else
  TIM_TypeDef *Instance = TIM1;
#endif
  // Instantiate HardwareTimer object. Thanks to 'new' instanciation, HardwareTimer is not destructed when setup() function is finished.
  HardwareTimer *MyTim = new HardwareTimer(Instance);

  MyTim->setOverflow(100, HERTZ_FORMAT); // 100 Hz = 10ms
  MyTim->attachInterrupt(handle_timer);
  MyTim->resume();
  
  // DATA pin signal change edge detection. (Mandatory)
  attachInterrupt(digitalPinToInterrupt(DATA_PIN), isr_routine, CHANGE);

  // jjy.freq(40); // Carrier frequency setting. Default:40
  jjy.monitor(LED_PIN);
  jjy.begin(); // Start JJY Receive
}

void loop() {
  time_t now = jjy.get_time();
  time_t lastreceived = jjy.getTime();
  tm tm_info;
  tm tm_lastinfo;

  localtime_r(&now, &tm_info);
  localtime_r(&lastreceived, &tm_lastinfo);

  lcd.clear();
  lcd.setCursor(0,0);
  char buf1[16];
  if (lastreceived==-1) {
    lcd.print("Receiving.. Q:");
    lcd.print(jjy.quality);
    lcd.setCursor(1,0);
    strftime(buf1, sizeof(buf1), "%H:%M:%S", &tm_info);
    lcd.print(buf1);
  } else {
    if (tm_info.tm_sec % 10 < 5) {
      strftime(buf1, sizeof(buf1), "Last:%H:%M", &tm_lastinfo);
    } else {
      strftime(buf1, sizeof(buf1), "%Y/%m/%d(%a)", &tm_info);
    }
    lcd.print(buf1);
    lcd.setCursor(1,0);
    char buf3[16];
    strftime(buf3, sizeof(buf3), "%H:%M:%S", &tm_info);
    lcd.print(buf3);
  }

  delay(100);
  if (tm_info.tm_min == 0 && lastreceived != -1){ // Receive start on the hour 
    jjy.begin();
  }
}
