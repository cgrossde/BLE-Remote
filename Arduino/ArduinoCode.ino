/**
 * Copyright Christoph Gross 2015 <gross@blubyte.de>
 * 
 * HM-10 Bluetooth 4.0 Low Energy remote control using
 * a KY-040 rotary encoder. Make sure the HM-10 has a 5V
 * logic level shifter or use a 3.3V Arduino board.
 */
#include <avr/sleep.h>
// Rotary encoder states
volatile boolean TurnDetected;
volatile boolean up;
// BLE status
boolean bleOn = false;
// PINS
const int BleVcc = 15;   // Powers BLE module or turns it off
const int PinCLK = 2;    // Used for generating interrupts using CLK signal
const int PinDT = 4;     // Used for reading DT signal
const int PinSW = 5;     // Used for the push button switch
const int led = 8;       // Status led
const int switch1 = 16;  // Mode switch: control volume or browse menu

// Inactivity tracking to send Arduino to sleep
unsigned long StartTime = 0;
unsigned long SleepInactive = 1000*30;  // 30 Sek: After 30 seconds the Arduino will enter sleep mode (~4mA), 
                                        // until the the rotary encoder is turned again.

// Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
void isr() {
  // Debounce rotary encoder
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 20ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 20) {
      if (!digitalRead(PinDT))
          up = true;
      else
          up = false;
    TurnDetected = true;
    lastInterruptTime = interruptTime;
  }
}


void setup ()  {
  // Enable LED and give 4 seconds time
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(4000); 
  // Setup pins
  pinMode(switch1, INPUT);
  pinMode(PinCLK,INPUT);
  pinMode(PinDT,INPUT);
  pinMode(PinSW,INPUT);
  pinMode(BleVcc,OUTPUT);
  // Enable BLE module
  digitalWrite(BleVcc, HIGH);
  bleOn = true;
  // Interrupt 1 is always connected to pin 2 on Arduino Pro Micro
  attachInterrupt(1,isr,FALLING);   
  delay(100);
  // Open serial to BLE and wait for connection
  Serial1.begin(9600);
  while(!Serial1) {
    ;
  }
  Serial1.println("Online");
  delay(300);
  digitalWrite(led, LOW); // Startup done, turn LED off
  // Sleepmode: Power-Down (lowest sleep mode possible)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop ()  {
  // Used to track inactivity time
  if(StartTime == 0) {
    StartTime = millis();
  }
  // Check runtime
  unsigned long CurrentTime = millis();
  unsigned long ElapsedTime = CurrentTime - StartTime;

  // Sleep?
  if(ElapsedTime > SleepInactive) {
    // Long - Short blink => Go to sleed
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    // Shutdown BLE
    digitalWrite(BleVcc, LOW);
    bleOn = false;
    sleep_enable();
    sleep_mode();
    // Arduino will now sleep until it detects a change on 
    // Pin 2 which means the rotary encoder was turned
    sleep_disable();
    // Start up BLE
    digitalWrite(BleVcc, HIGH);
    bleOn = true;
    // Loooong blink => woke up
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    StartTime = 0; // Reset starttime
  }

  if(bleOn && Serial1) {
    // Check rotary encoder push button
    if (digitalRead(PinSW) == LOW) {  // check if pushbutton is pressed
      StartTime = 0; // Reset starttime/inactivity timeout
      digitalWrite(led, HIGH);
      // Mode switch HIGH: Navigate menu
      if(digitalRead(switch1) == HIGH) {
        // Short press OK, long press go back
        delay(500);
        if(!(digitalRead(PinSW))) {
          // Long press
          Serial1.println("back");
        } else {
          // Short press
          Serial1.println("ok");
        }

      } 
      // Mode switch LOW: Volume controle / playpause
      else {
        Serial1.println("playpause");
      }
      delay(400);
      digitalWrite(led, LOW);
      TurnDetected = false;
    }
    // Do this only if rotation was detected
    if (TurnDetected)  {       
      StartTime = 0; // Reset starttime/inactivity timeout
      // Rotation up or down?
      if (up) {
        // Mode switch HIGH: Navigate menu
        if(digitalRead(switch1) == HIGH)
          Serial1.println("goup");
        else // Mode switch LOW: Volume controle / playpause
          Serial1.println("volup");
      } else {
        // Mode switch HIGH: Navigate menu
        if(digitalRead(switch1) == HIGH)
          Serial1.println("godown");
        else // Mode switch LOW: Volume controle / playpause
          Serial1.println("voldown");
      }
      TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
    }

  } else {
    delay(100);
  }
}

