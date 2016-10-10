//#define PROTO_BOARD 1
#define DEBUG
//#define FLASH_ENABLE

#include <SPI.h>
#ifdef FLASH_ENABLE
#include<SPIFlash.h>
#endif
#include "nRF24L01.h"
#include "RF24.h"
#include "FastLED.h"
#include <Bounce2.h>
#include "wrist_states.h"

#ifdef DEBUG
#include "printf.h"
#endif

// PIN Defines
#define P_FLASH_CS 8  // done // nrf mode select pin
#define P_NRF_CSN 9  // done  //spi chip select
#define P_NRF_CE 10  //done
#define P_MOSI 11  // not needed
#define P_MISO 12  // not needed
#define P_SCK 13  // not neded
#define P_NRF_SW A0  // done
#define P_LED_DIN A1 // done
#define P_LED_DOUT A2  // not needed / not used 
#define P_SW0 A3  // done
#ifndef PROTO_BOARD // protoboard not defined
  #define P_SW1 A4 // done
  #define P_SW2 A5  // done
#else
  #define P_SW1 3  //done
  #define P_SW2 4  // done
#endif
#define P_RX 0  // done
#define P_TX 1 // done
#define P_NRF_IRQ 2  // done
#define P_FLASH_HR 7  // done

#define BRIGHTNESS  10

Bounce bot_sw = Bounce();
Bounce mid_sw = Bounce();
Bounce top_sw = Bounce();
unsigned long top_sw_fts;

unsigned long ts;

#define P_SWB P_SW0
#define P_SWM P_SW1
#define P_SWT P_SW2


// LED Defines
#ifndef PROTO_BOARD // protoboard not defined
#define NUM_LEDS 93
#else
#define NUM_LEDS 135
#endif
#define LED_PIN P_LED_DIN
CRGB leds[NUM_LEDS];

// Flash vars
#ifdef FLASH_ENABLE
SPIFlash flash(P_FLASH_CS);
byte pageInputBuffer[256];
byte pageOutputBuffer[256];
#endif

// Declare RF24 as radio class
RF24 radio(P_NRF_CE, P_NRF_CSN);

// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC, 0xCE, 0xCC, 0xCE, 0xCC , 0xCE, 0xCC, 0xCE, 0xCC, 0xCE};

//#define ENABLE_PKT_ACK

/********************** Setup *********************/

#define PALETTE_RAINBOW 0
#define PALETTE_PURPLE 1
const TProgmemPalette16 myPurplePalette_p PROGMEM =
{
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple,
  CRGB::Purple,CRGB::Purple,CRGB::Purple,CRGB::Purple
};

#define PALETTE_RED 2
const TProgmemPalette16 myRedPalette_p PROGMEM =
{
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red,
  CRGB::Red,CRGB::Red,CRGB::Red,CRGB::Red
};

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

CRGBPalette16 backupPalette;
TBlendType    backupBlending;

#define STATE_OFF 0
#define STATE_OUTLINE 1
#define STATE_OUTLINE_DUAL 2
#define STATE_OUTLINE_QUAD 3
#define STATE_OUTLINE_ON 4
#define STATE_OUTLINE_ON_RAINBOW 5
#define STATE_NIGHTRIDER_RED 6
#define STATE_NIGHTRIDER 7
#define NUM_STATES 8

uint8_t state = STATE_OUTLINE;
uint8_t state_step = 0;
uint8_t state_init = 1;
uint8_t state_change_requested = 0;
uint8_t state_change_allowed = 0;
uint8_t state_next_state = 0xff;
uint8_t palette_step = 0;
uint8_t radio_write = 0;

unsigned long last_state_change = 0;
unsigned long state_change_timeout = 10 * 1000; // in seconds

void backup_palette() {
  backupPalette = currentPalette;
  backupBlending == currentBlending;
}

void restore_palette() {
  currentPalette = backupPalette;
  currentBlending = backupBlending;
}

void load_palette(uint8_t palette_id)
{
  if (palette_id == PALETTE_RAINBOW)
  {
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
  }
  else if (palette_id == PALETTE_PURPLE)
  {
    currentPalette = myPurplePalette_p;
    currentBlending = NOBLEND;
  }
  else if (palette_id == PALETTE_RED)
  {
    currentPalette = myRedPalette_p;
    currentBlending = NOBLEND;
  }
}

void setup() {
  // setup buttons
  pinMode(P_SWB, INPUT_PULLUP);
  pinMode(P_SWM, INPUT_PULLUP);
  pinMode(P_SWT, INPUT_PULLUP);

  bot_sw.attach(P_SWB);
  bot_sw.interval(5);
  
  mid_sw.attach(P_SWM);
  mid_sw.interval(5);

  top_sw.attach(P_SWT);
  top_sw.interval(5);
  top_sw_fts = millis();

  // setup flash pins
  // http://www.winbond.com/resource-files/w25q128fv_revhh1_100913_website1.pdf
  pinMode(P_FLASH_HR, OUTPUT);  // active low
  pinMode(P_FLASH_CS, OUTPUT);  // active low
  digitalWrite(P_FLASH_HR, HIGH); // do not hold spi. do not reset
  digitalWrite(P_FLASH_CS, HIGH); // deselect flash

  #ifdef FLASH_ENABLE
  flash.begin();  
  #endif

  // setup nrf pins
  pinMode(P_NRF_SW, OUTPUT);
  digitalWrite(P_NRF_SW, HIGH);  // Turn off NRF
  pinMode(P_NRF_IRQ, INPUT);
  //pinMode(P_NRF_CSN, OUTPUT);  // Let the RF24 lib handle csn
  //pinMode(P_NRF_CE, OUTPUT);   // Let the RF24 lib handle ce

  // LED Matrix setup
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Load default palette to memory and init backup palette
  load_palette(PALETTE_RAINBOW);
  backup_palette();
  
  // BEGIN
  Serial.begin(115200);
  #ifdef DEBUG 
    printf_begin();
  #endif
  // power on NRF
  digitalWrite(P_NRF_SW, LOW);
  delay(500);
  
  // Setup and configure rf radio
  radio.begin();
  delay(100);
  //radio.setPALevel(RF24_PA_LOW);
  #ifdef ENABLE_PKT_ACK
      radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
  #endif
  radio.enableDynamicPayloads();                    // Ack payloads are dynamic payloads
  // Open pipes to other node for communication
  radio.openWritingPipe(address[0]);             // communicate back and forth.  One listens on it, the other talks to it.
  radio.openReadingPipe(1, address[1]);
  #ifdef DEBUG
    radio.printDetails();                             // Dump the configuration of the rf unit for debugging
  #endif
  
  delay(50);
  attachInterrupt(0, radio_irq, LOW);             // Attach interrupt handler to interrupt #0 (using pin 2) on BOTH the sender and receiver
  last_state_change = millis();
}


/********************** Main Loop *********************/
void loop() {
  // Read inputs
  read_buttons();

  // Transition state on timer
  if (millis() >= (last_state_change + state_change_timeout))
  {
    state_change_requested = 1;
  }

  // State Change logic
  if (state_change_requested == 1 && state_change_allowed==1)
  {
    Serial.print("State Change from "); Serial.print(state);
    if (state_next_state == 0xff)
    {
      state++;
      if(state >= NUM_STATES)
      {
        state = 1;  // 0 is the off state. we dont want to automatically transition to off
      }
    }
    else
    {
      // Allow external events (nrf incoming, button presses, etc to set the state)
      state = state_next_state;
      state_next_state = 0xff;
    }
    Serial.print(" to "); Serial.println(state);
    state_change_requested = 0;
    state_change_allowed = 0;
    state_step = 0;
    restore_palette();
    state_init = 1;
    // save current time
    last_state_change = millis();
    // Send State ID
    radio.stopListening();
    radio.flush_tx();
    #ifdef ENABLE_PKT_ACK
    radio.startWrite( &state, sizeof(uint8_t), 0; //ACK
    #else
    radio.startWrite( &state, sizeof(uint8_t), 0); //NAK
    #endif
  }

  // State machine
  if (state == STATE_OFF) {
    tie_no_leds();
  }
  else if (state == STATE_OUTLINE) {
    outline(1);
    //outline_single();
  }
  else if (state == STATE_OUTLINE_DUAL) {
    outline(2);
    //outline_dual();  
  }
  else if (state == STATE_OUTLINE_QUAD) {
    outline(4);
    //outline_quad();  
  }
  else if (state == STATE_OUTLINE_ON) {
    if(state_init != 0)
    {
      state_init = 0;
      backup_palette();
      load_palette(PALETTE_PURPLE);
    }
    outline_on_from_palette();
  }
  else if (state == STATE_OUTLINE_ON_RAINBOW) {
    if(state_init != 0)
    {
      state_init = 0;
      backup_palette();
      load_palette(PALETTE_RAINBOW);
    }
    outline_on_from_palette();
  }
  else if (state == STATE_NIGHTRIDER_RED) {
    if(state_init != 0)
    {
      state_init = 0;
      backup_palette();
      load_palette(PALETTE_RED);
    }
    test_nightrider();
  }
  else if (state == STATE_NIGHTRIDER) {
    test_nightrider();
  }

  /*
    #ifdef ENABLE_PKT_ACK
    radio.startWrite( &button_pressed_last, sizeof(uint8_t) , 0; //ACK 
    #else
    radio.startWrite( &button_pressed_last, sizeof(uint8_t) , 1); //NAK
    #endif
  */
}

void read_buttons() {
  bot_sw.update();
  mid_sw.update();
  top_sw.update();

  if ( bot_sw.fell() ) {
    Serial.println("BOT FALL");
    currentBlending = LINEARBLEND;
    currentPalette = RainbowColors_p;
  }

  if ( mid_sw.fell() ) {
    Serial.println("MID FALL");
    currentBlending = NOBLEND;
    currentPalette = myPurplePalette_p;
  }

  // Falling edge first then rising edge
  if ( top_sw.fell() ) {
    top_sw_fts = millis();
    Serial.println("TOP FALL");
  }
  else if ( top_sw.rose() ) {
    ts = millis() - top_sw_fts;
    Serial.println(ts);
    if (ts > 5000) {
      Serial.println("TOP HOLD 5+ SEC");
    }
    else if (ts > 4000) {
      Serial.println("TOP HOLD 4 SEC");
    }
    else if (ts > 3000) {
      Serial.println("TOP HOLD 3 SEC");
    }
    else if (ts > 2000) {
      Serial.println("TOP HOLD 2 SEC");
    }
    else if (ts > 1000) {
      Serial.println("TOP HOLD 1 SEC");
      state_change_requested = 1;
    }
    else if (ts > 500) {
      Serial.println("TOP HOLD 0.5 SEC");
      state_change_requested = 1;
      state_next_state = STATE_OUTLINE_ON_RAINBOW;  // forces next state
    }
    else
    {
      Serial.println("TOP PRESS");
    }
    
  }
}

/********************** Interrupts *********************/
void radio_irq(void)                                // Receiver role: Does nothing!  All the work is in IRQ
{

  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);                   // What happened?

  if ( tx ) {                                         // Have we successfully transmitted?
    radio.startListening();
    Serial.println(F("Send:OK"));
  }

  if ( fail ) {                                       // Have we failed to transmit?
    radio.startListening();
    Serial.println(F("Send:Failed"));
  }

  if ( rx || radio.available()) {                     // Did we receive a message?
    #ifdef ENABLE_PKT_ACK
    radio.read(&message_count, sizeof(message_count));
    Serial.print(F("Ack: "));
    Serial.println(message_count);
    #endif
  }
}
