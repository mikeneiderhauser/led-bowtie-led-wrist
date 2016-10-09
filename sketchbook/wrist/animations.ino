// function to turn off all leds in the ring
void ring_off() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

// state off helper
void state_off()
{
  // Prevents multiple writes out to led's.. saving some power
  if(state_step == 0)
  {
    ring_off();
    state_step++;
  } 
}

// Turn on led's based on mask (only works for 8 led's)
void outline(uint8_t mask)
{
  if (((1 << state_step) & mask) > 0)
  {
    leds[state_step] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
  }
  else
  {
    leds[state_step] = CRGB::Black;
  }
  
  state_step++;
  if(state_step >= NUM_LEDS) 
  {
    palette_step = palette_step + 1;
    FastLED.show();
    delay(10);
    state_step = 0;
  }
}

// Turn on led's based on mask (only works for 8 led's)
// for solid color palettes.. saves power
void outline_solid(uint8_t mask)
{
  if (state_step == 0)
  {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      if (((1 << i) & mask) > 0)
      {
        leds[state_step] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
      }
      else
      {
        leds[state_step] = CRGB::Black;
      }
    }
    state_step++;
    FastLED.show();
  }
}

void chase(uint8_t ct, uint8_t * idxs)
{
  // ct -> Number of px to have on for chase (mirrored). Only 1,2,4 works. Anyting else 1 px will be lit
  // *idxs -> ptr to chase_idxs array
  
  // Clear all pixels  
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  leds[idxs[state_step]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);

  if(ct == 2 || ct == 4)
  {
    if(state_step < 4)
    {
      leds[idxs[state_step+4]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-4]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
  }

  if (ct == 4)
  {
    if(state_step < 2)
    {
      leds[idxs[state_step+6]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-2]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }

    if(state_step < 6)
    {
      leds[idxs[state_step+2]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-6]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
  }

  // different delays for px ct
  // todo.. use millis for these changes
  if(ct==2)
  {
    delay(150);
  }
  else if(ct == 4)
  {
    delay(300);
  }
  else
  {
    delay(80);
  }
  
  state_step++;
  palette_step = palette_step + 4;
  FastLED.show();
  if(state_step >= NUM_LEDS) {
    state_step = 0;
  }
}

