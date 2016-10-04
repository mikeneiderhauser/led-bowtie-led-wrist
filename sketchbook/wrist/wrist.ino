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

#define BRIGHTNESS  10
#define NUM_LEDS 2

CRGB leds[NUM_LEDS];

// Declare RF24 as radio class
RF24 radio(P_NRF_CE, P_NRF_CSN);

// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC, 0xCE, 0xCC, 0xCE, 0xCC , 0xCE, 0xCC, 0xCE, 0xCC, 0xCE};

//#define ENABLE_PKT_ACK

void setup() {
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
}
  
void loop() {

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
    #ifdef ENABLE_PKT_ACK
    radio.read(&message_count, sizeof(message_count));
    Serial.print(F("Ack: "));
    Serial.println(message_count);
    #endif
  }
}
  
