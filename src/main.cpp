/*
Program: demo gpio 2 led, 1 interrupt switch, luxMeter and DHT11
  - program will print Temperature, Humidity and Light every 2 secs
  - program will flash red Led then green Led
Device for ESP32: 
- Led Red: IO2 (active high, Anode Led to IO02 via R (68-220) Ohm, Cathode: GND)
- Led Green: IO12 (active high, Anode Led to IO12 via R (68-220) Ohm, Cathode: GND)
- PushButton (PIN_SW): IO0 (active Low, add pull up 10K to 3.3V)
- DHT11: IO13, add pullup 10K to VCC
- BH1750: I2C (SDA: IO14, SCL:IO15; add pullup 10K to 3.3V)

Device for ESP8266: 
- Led Red: D8 (active high, Anode Led to IO02 via R (68-220) Ohm, Cathode: GND)
- Led Yellow: D7 (active high, Anode Led to IO02 via R (68-220) Ohm, Cathode: GND)
- Led Green: D6 (active high, Anode Led to IO12 via R (68-220) Ohm, Cathode: GND)
- PushButton (PIN_SW): D5 (active Low, add pull up 10K to 3.3V)
- DHT11: D3, add pullup 10K to VCC
- BH1750: I2C (D2: IO14, SCL:D1; add pullup 10K to 3.3V)

*/
#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "BH1750.h"
#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
#include "device.h"

#if defined(ESP32)
  #define LED_COUNT 2
  const uint8_t arLed[LED_COUNT] = {LED_RED, LED_GREEN};
#endif

#if defined(ESP8266)
  #define LED_COUNT 3
  const uint8_t arLed[LED_COUNT] = {LED_RED, LED_YELLOW, LED_GREEN};
#endif

int nCount=0;

Ticker timer1Sec, ledOff;
DHTesp dht;
BH1750 lightMeter;
void onReadDht()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  if (dht.getStatus()==DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n", temperature, humidity);
  }
}

void onReadLight()
{
  float lux = lightMeter.readLightLevel();
  Serial.printf("Light: %.2f lx\n", lux);
}

void onReadSensors()
{
  Serial.printf("Hello World %d\n", nCount);
  digitalWrite(arLed[nCount%LED_COUNT], LED_ON);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);
  onReadDht();
  onReadLight();
  ledOff.once_ms(100, [](){
      digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);
      digitalWrite(arLed[nCount%LED_COUNT], LED_OFF);
      nCount++;
  });
}

IRAM_ATTR void onResetCounter()
{
  nCount = 0;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
#if defined(LED_FLASH)    
  pinMode(LED_FLASH, OUTPUT);
#endif  
  for (uint8_t i=0; i<LED_COUNT; i++)
    pinMode(arLed[i], OUTPUT);
  pinMode(PIN_SW, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_SW), onResetCounter, FALLING); // extra checking

  dht.setup(PIN_DHT, DHTesp::DHT11);
  Wire.begin(PIN_SDA, PIN_SCL);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);

  Serial.printf("Board: %s\n", ARDUINO_BOARD);
  Serial.printf("DHT Sensor ready, sampling period: %d ms\n", dht.getMinimumSamplingPeriod());  
  timer1Sec.attach_ms_scheduled(2*dht.getMinimumSamplingPeriod(), onReadSensors);
}

void loop() {
}