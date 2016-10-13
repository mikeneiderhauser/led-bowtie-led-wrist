//#define PROTO_BOARD 1
#define DEBUG
//#define FLASH_ENABLE

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "FastLED.h"

#ifdef DEBUG
#include "printf.h"
#endif

// PIN Define
#define P_NRF_CSN 9  // done  //spi chip select
#define P_NRF_CE 10  //done
#define P_MOSI 11  // not needed
#define P_MISO 12  // not needed
#define P_SCK 13  // not neded
#define P_NRF_SW A0  // done
#define P_LED_DIN A1 // done
#define P_RX 0  // done
#define P_TX 1 // done
#define P_NRF_IRQ 2  // done
#define LED_PIN A1

// Define LEDs in ring
#define BRIGHTNESS  30
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

// Declare RF24 as radio class
//#define ENABLE_PKT_ACK
RF24 radio(P_NRF_CE, P_NRF_CSN);
uint8_t nrf_byte = 0;
uint8_t last_nrf_byte = 0;
uint8_t request_radio_reset = 0;

// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC, 0xCE, 0xCC, 0xCE, 0xCC , 0xCE, 0xCC, 0xCE, 0xCC, 0xCE};

// State processing variables
uint8_t state = 0;                    // current state
uint8_t state_step = 0;               // current animation step in state
uint8_t palette_step = 0;             // current palette step in state
unsigned long last_state_change = 0;
unsigned long state_change_timeout = 300000; // state timout. goto default in seconds
// (300 sec -> 5 mins)

// States
#define STATE_OFF 0
#define STATE_RING_ALL_RAINBOW 1
#define STATE_RING_ALL_PURPLE 2
#define STATE_RING_ALL_RED 3
#define STATE_RING_CHASE4CLK_RAINBOW 4
#define STATE_RING_CHASE4CLK_PURPLE 5
#define STATE_RING_CHASE4CLK_RED 6
#define STATE_RING_CHASE4NCLK_RAINBOW 7
#define STATE_RING_CHASE4NCLK_PURPLE 8
#define STATE_RING_CHASE4NCLK_RED 9
#define STATE_RING_CHASE2CLK_RAINBOW 10
#define STATE_RING_CHASE2CLK_PURPLE 11
#define STATE_RING_CHASE2CLK_RED 12
#define STATE_RING_CHASE2NCLK_RAINBOW 13
#define STATE_RING_CHASE2NCLK_PURPLE 14
#define STATE_RING_CHASE2NCLK_RED 15
#define STATE_RING_CHASE1CLK_RAINBOW 16
#define STATE_RING_CHASE1CLK_PURPLE 17
#define STATE_RING_CHASE1CLK_RED 18
#define STATE_RING_CHASE1NCLK_RAINBOW 19
#define STATE_RING_CHASE1NCLK_PURPLE 20
#define STATE_RING_CHASE1NCLK_RED 21

#define NUM_STATES 22  // Total number of stats
#define DEFAULT_STATE STATE_RING_ALL_PURPLE  // Default state

// const arrays for animation indexes
const uint8_t chase_clockwise_idxs[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const uint8_t chase_nclockwise_idxs[8] = {7, 6, 5, 4, 3, 2, 1, 0};

// define color palettes
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

#define PALETTE_OFF 3
const TProgmemPalette16 myOffPalette_p PROGMEM =
{
  CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,
  CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,
  CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,
  CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black
};

// vars for current color palette
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// vars for backup color palette
CRGBPalette16 backupPalette;
TBlendType    backupBlending;

// function to backup the current color palette
void backup_palette() {
  backupPalette = currentPalette;
  backupBlending == currentBlending;
}

// function to restore the backup color palette
void restore_palette() {
  currentPalette = backupPalette;
  currentBlending = backupBlending;
}

// function to load a color palette
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
  else if (palette_id == PALETTE_OFF)
  {
    currentPalette = myOffPalette_p;
    currentBlending = NOBLEND;
  }
}

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
  radio.openWritingPipe(address[1]);             // communicate back and forth.  One listens on it, the other talks to it.
  radio.openReadingPipe(1, address[0]);
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

// Arduino setup function
void setup() {
  // setup nrf pins
  pinMode(P_NRF_SW, OUTPUT);
  digitalWrite(P_NRF_SW, HIGH);  // Turn off NRF
  pinMode(P_NRF_IRQ, INPUT);
  //pinMode(P_NRF_CSN, OUTPUT);  // Let the RF24 lib handle csn
  //pinMode(P_NRF_CE, OUTPUT);   // Let the RF24 lib handle ce

  // LED ring setup
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  ring_off();  // turn off led's

  // load rainbow palette as default (may change depending on state)
  load_palette(PALETTE_RAINBOW);
  backup_palette();

  state = DEFAULT_STATE;
  
  // BEGIN
  Serial.begin(115200);
  #ifdef DEBUG 
    printf_begin();
  #endif

  // turn on the raido
  start_radio();
  last_state_change = (unsigned long)millis();
}

void loop() {
  // Handle RX of single byte from radio
  if (nrf_byte != last_nrf_byte)
  {
    last_nrf_byte = nrf_byte;
    // process state transition
    if (last_nrf_byte < NUM_STATES)
    {
      Serial.print("Changing to state ");
      Serial.println(last_nrf_byte);
      // do a quick state change, dont allow animations to 'finish'
      state_step = 0;
      state = last_nrf_byte;
      last_state_change = (unsigned long)millis();
    }
    // else dont do anything.. allow timeout to occur
  }

#define TIMEOUT_ENABLED
#ifdef TIMEOUT_ENABLED
  // Handle radio timeout (last state_change_timeout)
  if ((unsigned long)(millis() -last_state_change) >= state_change_timeout)
  {
    Serial.println("Hit timeout");
    state = DEFAULT_STATE;
    state_step = 0;
    last_state_change = (unsigned long)millis();
    // More than likely an issue w/ the radio.  request reset
    request_radio_reset = 1;
  }
#endif

  // State machine
  if(state == STATE_OFF)
  {
    load_palette(PALETTE_RAINBOW);
    state_off();
  }
  else if(state == STATE_RING_ALL_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    outline(0xff);
  }
  else if(state == STATE_RING_ALL_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    outline_solid(0xff);
  }
  else if(state == STATE_RING_ALL_RED)
  {
    load_palette(PALETTE_RED);
    outline_solid(0xff);
  }
  else if(state == STATE_RING_CHASE4CLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(4, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE4CLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(4, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE4CLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(4, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE4NCLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(4, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE4NCLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(4, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE4NCLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(4, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2CLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(2, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2CLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(2, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2CLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(2, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2NCLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(2, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2NCLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(2, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE2NCLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(2, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1CLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(1, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1CLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(1, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1CLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(1, chase_clockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1NCLK_RAINBOW)
  {
    load_palette(PALETTE_RAINBOW);
    chase(1, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1NCLK_PURPLE)
  {
    load_palette(PALETTE_PURPLE);
    chase(1, chase_nclockwise_idxs);
  }
  else if(state == STATE_RING_CHASE1NCLK_RED)
  {
    load_palette(PALETTE_RED);
    chase(1, chase_nclockwise_idxs);
  }

  // Process radio reset
  if (request_radio_reset == 1)
  {
    Serial.println("RESETTING RADIO!!!");
    request_radio_reset = 0;
    reset_radio();
  }
}
  
/********************** Interrupts *********************/
void radio_irq(void)                                // Receiver role: Does nothing!  All the work is in IRQ
{

  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);                   // What happened?

  if ( tx ) {                                         // Have we successfully transmitted?
    Serial.println(F("Send:OK"));
  }

  if ( fail ) {                                       // Have we failed to transmit?
    Serial.println(F("Send:Failed"));
  }

  if ( rx || radio.available()) {                     // Did we receive a message?
    // read single byte from radio 
    radio.read(&nrf_byte, sizeof(nrf_byte));
    #ifdef DEBUG
    Serial.print(F("NRF RX: "));
    Serial.println(nrf_byte);
    #endif
  }
}

