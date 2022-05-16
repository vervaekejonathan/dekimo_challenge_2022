#include "lut.h"
#include "tools.h"
#include <util/atomic.h>
#include <avr/interrupt.h>


// https://store-usa.arduino.cc/products/arduino-nano-every
// https://content.arduino.cc/assets/Nano-Every_processor-48-pin-Data-Sheet-megaAVR-0-series-DS40002016B.pdf
// https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega4808-09-DataSheet-DS40002173C.pdf
// https://tomeko.net/online_tools/ascii.php?lang=en
// https://en.wikipedia.org/wiki/Divisibility_rule#Divisibility_by_7
// https://en.wikipedia.org/wiki/Divisibility_rule#Divisibility_by_9
// https://en.wikipedia.org/wiki/Divisibility_rule#Divisibility_by_11
// https://en.wikipedia.org/wiki/Divisibility_rule#Divisibility_by_99
// https://zeptomath.com/calculators/factorial.php?number=999&hl=en
// https://github.com/Kees-van-der-Oord/Arduino-Nano-Every-Timer-Controller-B/blob/master/EveryTimerB/EveryTimerB.h

int ledState = LOW;

#define setPin() ( PORTB |= 0x01 ) // set D8
#define clrPin() ( PORTB &= 0xFE ) // clear D8

void setup() {
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  init_nanos();
}


int8_t sum_even = 0; // used to check if the number is divisible by 99, when the length is even
int8_t sum_uneven = 0; // used to check if the number is divisible by 99, when the length is uneven

uint8_t mod = 0;
uint16_t fact = 0;

uint16_t total_characters = 0;
uint16_t position_xy = 0;

void loop() {
  static uint32_t counter = 0;

  // read in the numbers
  if(Serial.available() > 0) {
    digitalWrite(13, LOW);
    counter = 0;
    digitalWrite(11, HIGH);
    char input_raw = Serial.read();
    uint8_t input;
    nanos_start();
    if(input_raw == '\r') {
      setPin();
      nanos_start();
      // no extra calculation are needed, and all calculations are done before the next 
      // character is received. 
      // 115200 BAUD is 115200 bits/s
      // (1sec / 115200)*10 (bytes per character, with start and stop bit)
      // = ~86 us
      // so, to avoid cheating (and delaying reading in characters because the calculations are still happening)
      // all calculations done when receiving a character must be at least less than 86us
      // this is the case. you can check by viewing pin D11
      clrPin();
      uint16_t nanoEnd = nanos_end();  
         
      
      Serial.print(fact);
      Serial.print("!xy=");
      if(mod < 10) Serial.print("0");
      Serial.println(mod);
      Serial.println(nanoEnd);
      //Serial.println(" ns");
      total_characters = 0;
      sum_even = 0;
      sum_uneven = 0;
    }
    else if(input_raw == '\n') {
      // ignore
    } else {
     
      if(input_raw > '9' || input_raw < '0') {
        // all non-numeric characters are translated into zero
        input = 0;   
        if(input_raw == 'x') {
          position_xy = total_characters;
        }
      } else {
        // all numeric characters are converted into there integer
        input = input_raw - '0';      
      }
  
      // since we have some time left, waiting for the next character, we already so some calculations
      // append the new character to the sum and to the plusmin_sum
      if(total_characters % 2 == 0) {
        sum_uneven = modu(input + sum_uneven, 99);
        sum_even = modu(input * 10 + sum_even, 99);
      } else {
        sum_uneven = modu(input * 10 + sum_uneven, 99);
        sum_even = modu(input + sum_even, 99);
      }
      //Serial.println(input);
      //Serial.println(sum_even);
      //Serial.println(sum_uneven);
      //Serial.println("******************");
  
      // depending on the length, we pick either the even or uneven summations
      switch(total_characters % 2) {
        case 0: mod = 99 - (sum_even); break;
        case 1: mod = 99 - (sum_uneven); break;
      }    
  
      fact = get_factorial(total_characters+1);
  
      if(mod == 00)   {
        // dividing by 99 could result in two hits, 00 and 99,
        // but since the assignment states that 00 will never be masked,
        // we can safely divide by 99 and modify a 00 result to 99
        mod = 99;
      }
      if(position_xy % 2 != total_characters % 2) {
        // we need to reverse the x and y if the xy is not at a 'multiple of 2' position
        uint8_t temp = mod;
        mod = (temp / 10) + ( temp % 10) * 10;
      }
      
      digitalWrite(11, LOW);
      total_characters++;
    }
  }

  if(counter > 200000) {
    ledState = !ledState;
    digitalWrite(13, ledState);
    counter = 0;
  } else {
    counter++;
  }
}


inline uint8_t modu(uint8_t N, uint8_t D) {
  return N%D;
}

inline uint16_t get_factorial(uint16_t length) {
  return lut[length];
}
