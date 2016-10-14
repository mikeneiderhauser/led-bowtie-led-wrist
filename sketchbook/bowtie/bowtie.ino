//#define PROTO_BOARD 1
//#define FLASH_ENABLE

#define DEBUG
#define EN_SER_PR

#include <SPI.h>
#ifdef FLASH_ENABLE
#include<SPIFlash.h>
#endif
#include "nRF24L01.h"
#include "RF24.h"
#include "FastLED.h"
#include <Bounce2.h>
#include "animations.h"
#include "wrist_states.h"
#include "pixel.h"

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

uint8_t state = ANIM_NIGHTRIDER;  //ANIM_NIGHTRIDER;
uint8_t wrist_state = 0;
uint8_t state_step = 0;
uint8_t state_init = 0;
uint8_t state_change_requested = 0;
uint8_t state_next_state = 0xff;
uint8_t palette_step = 0;
uint8_t radio_write = 0;
uint8_t tx_fail_ct = 0;
#define MAX_TX_FAIL 10
uint8_t bt_anim_mode = 1;
uint8_t bt_anim_cfg = 0;

unsigned long last_state_change = 0;
unsigned long state_change_timeout = 10 * 1000; // in seconds

// function to turn on (physically) and init the radio
void start_radio() {
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
  radio.startListening();
  #ifdef DEBUG
    radio.printDetails();                             // Dump the configuration of the rf unit for debugging
  #endif
  
  delay(50);
  attachInterrupt(0, radio_irq, LOW);             // Attach interrupt handler to interrupt #0 (using pin 2) on BOTH the sender and receiver
}

// function to turn off (physically) the radio
void stop_radio() {
  detachInterrupt(0);
  radio.stopListening();
  delay(10);
  digitalWrite(P_NRF_SW, HIGH);
  delay(100);
}

// function to power cycle the radio
void reset_radio()
{
  stop_radio();
  start_radio();
}

void setup() {
  // Initialize the random number generator
  CreateTrulyRandomSeed();

  // Enable the serial device
  #ifdef EN_SER_PR
  Serial.begin(115200);
  #ifdef DEBUG 
    printf_begin();
  #endif
  #endif

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
  LoadPalette(PALETTE_RAINBOW);
  BackupPalette();
  
  // turn on the raido
  start_radio();
  last_state_change = millis();
  // Enable first state transition
  state_change_requested = 1;
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
  if (state_change_requested == 1 && switchAnimation(state, state_step)==true)
  {
    #ifdef EN_SER_PR
    Serial.print("State Change from "); Serial.print(state);
    #endif
    // no defined next state
    if (state_next_state == 0xff)
    {
      state++;
      if(state >= ANIM_COUNT)
      {
        state = 1;  // 0 is the off state. we dont want to automatically transition to off
      }
    }
    // defined next state
    else
    {
      // Allow external events (nrf incoming, button presses, etc to set the state)
      state = state_next_state;
      state_next_state = 0xff;
    }
    #ifdef EN_SER_PR
    Serial.print(" to "); Serial.println(state);
    #endif

    // state var cleanup
    state_change_requested = 0;  // clear state change request
    state_step = 0;  // reset state step
    RestorePalette();  // restore previous color palette - TBD May not be needed
    // save current time
    last_state_change = millis();

    state_init = 1;  // we changed states.. perform state init (execution later)
  }

  state = ANIM_MATRIX;
  // State machine - select animation mode, cfg, wrist state, and palette
  // TODO load palette per state
  if (state == ANIM_TIE_OFF) {
    bt_anim_mode = 1;
    bt_anim_cfg = 0;
    wrist_state = 0; // FORCE OFF
  }
  else if (state == ANIM_OUTLINE2) {
    bt_anim_mode = 1;
    bt_anim_cfg = 2;
    LoadPalette(PALETTE_RAINBOW);
    wrist_state = STATE_RING_CHASE4NCLK_RAINBOW;
  }
  else if (state == ANIM_OUTLINE4) {
    bt_anim_mode = 1;
    bt_anim_cfg = 4;
    LoadPalette(PALETTE_RAINBOW);
    wrist_state = STATE_RING_CHASE4CLK_RAINBOW;
  }
  else if (state == ANIM_OUTLINE_ON) {
    bt_anim_mode = 1;
    bt_anim_cfg = 255;
    LoadPalette(PALETTE_PURPLE);
    wrist_state = STATE_RING_CHASE4NCLK_RAINBOW;
  }
  else if (state == ANIM_OUTLINE_ON_RB) {
    bt_anim_mode = 1;
    bt_anim_cfg = 255;
    LoadPalette(PALETTE_RAINBOW);
    wrist_state = STATE_RING_ALL_RAINBOW;
  }
  else if (state == ANIM_WHISKERS) {
    // TODO ANIMATION
    bt_anim_mode = 3;
    bt_anim_cfg = 0;
    // TODO palette
    wrist_state = 0;  // define via color palette
  }
  else if (state == ANIM_PINWHEEL) {
    // TODO ANIMATION
    bt_anim_mode = 4;
    bt_anim_cfg = 0;
    // TODO palette
    wrist_state = 0;  // define via color palette
  }
  else if (state == ANIM_NIGHTRIDER) {
    // TODO ANIMATION FIX
    bt_anim_mode = 2;
    bt_anim_cfg = 0;
    LoadPalette(PALETTE_RED);
    wrist_state = STATE_RING_CHASE4NCLK_RED;
  }
  else if (state == ANIM_NIGHTRIDER_RB) {
    // TODO ANIMATION FIX
    bt_anim_mode = 2;
    bt_anim_cfg = 0;
    LoadPalette(PALETTE_RAINBOW);
    wrist_state = STATE_RING_CHASE4NCLK_RAINBOW;
  }
  else if (state == ANIM_MATRIX) {
    bt_anim_mode = 5;
    bt_anim_cfg = 0;
    // TODO palette
    wrist_state = 0;  // define via color palette
  }
#if 0
  else if (state == ANIM_BTANIMATION) {
    // TODO ANIMATION
    bt_anim_mode = 6;
    bt_anim_cfg = 0;
    // TODO palette
    wrist_state = 0;  // define via color palette
  }
#endif
  else if (state == ANIM_RAINBOW) {
    // TODO ANIMATION
    bt_anim_mode = 7;
    bt_anim_cfg = 0;
    LoadPalette(PALETTE_RAINBOW);
    wrist_state = STATE_RING_CHASE4CLK_RAINBOW;
  }
#if 0
  else if (state == ANIM_MOUSTACHE) {
    bt_anim_mode = ;
    bt_anim_cfg = 0;
  }
  else if (state == ANIM_MOUSTACHE_ON) {
    bt_anim_mode = ;
    bt_anim_cfg = 255;

  }
#endif

  // perform animation init
  if(state_init != 0)
  {
    state_init = 0;
    initAnimation(bt_anim_mode, bt_anim_cfg);
    // send wrist state
    radio.stopListening();
    radio.flush_tx();
    // If we have failed to TX x num times.. reset radio before this TX
    if(tx_fail_ct >= MAX_TX_FAIL)
    {
        tx_fail_ct = 0;
        reset_radio();
    }
    #ifdef ENABLE_PKT_ACK
    radio.startWrite(&wrist_state, sizeof(uint8_t), 0; //ACK
    #else
    radio.startWrite(&wrist_state, sizeof(uint8_t), 0); //NAK
    #endif
  }

  // run animation
  if(animAnimation(bt_anim_mode, state_step) == true) {
    state_step = 0;
  }
  else
  {
    // increment animation step
    state_step++;
  }
}

void read_buttons() {
  bot_sw.update();
  mid_sw.update();
  top_sw.update();

  if ( bot_sw.fell() ) {
    #ifdef EN_SER_PR
    Serial.println("BOT FALL");
    #endif
    state_change_requested = 1;
    state_next_state = ANIM_TIE_OFF;
    //forces next state to off
  }

  if ( mid_sw.fell() ) {
    #ifdef EN_SER_PR
    Serial.println("MID FALL");
    #endif
    // TODO add one of Joe's Animations here
    state_change_requested = 1;
    state_next_state = ANIM_OUTLINE_ON;
    //forces next state to purple outline
  }

  // Falling edge first then rising edge
  if ( top_sw.fell() ) {
    top_sw_fts = millis();
    #ifdef EN_SER_PR
    Serial.println("TOP FALL");
    #endif
    state_change_requested = 1;
    state_next_state = ANIM_OUTLINE_ON;
    //forces next state to purple outline
  }
}

/********************** Interrupts *********************/
void radio_irq(void)                                // Receiver role: Does nothing!  All the work is in IRQ
{

  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);                   // What happened?

  if ( tx ) {                                         // Have we successfully transmitted?
    radio.startListening();
    #ifdef EN_SER_PR
    Serial.println(F("Send:OK"));
    #endif
  }

  if ( fail ) {                                       // Have we failed to transmit?
    tx_fail_ct++;
    radio.startListening();
    #ifdef EN_SER_PR
    Serial.println(F("Send:Failed"));
    #endif
  }

  if ( rx || radio.available()) {                     // Did we receive a message?
    #ifdef ENABLE_PKT_ACK
    radio.read(&message_count, sizeof(message_count));
    #ifdef EN_SER_PR
    Serial.print(F("Ack: "));
    Serial.println(message_count);
    #endif
    #endif
  }
}
