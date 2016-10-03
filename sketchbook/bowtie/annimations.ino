void tie_off() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void tie_no_leds() {
  if (state_step == 0)
    {
      tie_off();
      state_step++;
    }
    if(state_change_requested == 1)
    {
      // allow state transition immediately
      state_change_allowed = 1;
    }
}

void outline(uint8_t ct)
{
  uint8_t idxs[44] = {0,1,2,3,4,5,6,7,8,9,26,27,38,39,44,45,50,51,58,59,74,75,92,91,90,89,88,87,86,85,84,83,66,65,54,53,48,47,42,41,34,33,18,17};

  // turn off last step
  tie_off();
  // set new light
  leds[idxs[state_step]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);

  if(ct == 4) {
    if (state_step < 11)
    {
      leds[idxs[state_step+33]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-11]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }

    if (state_step < 33)
    {
      leds[idxs[state_step+11]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-33]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
  }

  if(ct == 2 || ct == 4) {
    if (state_step < 22)
    {
      leds[idxs[state_step+22]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
    else
    {
      leds[idxs[state_step-22]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    }
  }

  FastLED.show();
  delay(20);
  
  state_step++;
  palette_step++;
  if (state_step == 44) {
    state_step = 0;
  }

  if(state_change_requested == 1 && state_step == 0)
  {
    // allow state transition immediately
    state_change_allowed = 1;
  }
}

void outline_on_from_palette() {
  uint8_t idxs[44] = {0,1,2,3,4,5,6,7,8,9,26,27,38,39,44,45,50,51,58,59,74,75,92,91,90,89,88,87,86,85,84,83,66,65,54,53,48,47,42,41,34,33,18,17};

  for(uint8_t i = 0; i<44; i++)
  {
    leds[idxs[i]] = ColorFromPalette( currentPalette, state_step, 255, currentBlending);
  }
  FastLED.show();
  state_step = state_step + 2;
      
  if(state_change_requested == 1)
  {
    // allow state transition immediately
    state_change_allowed = 1;
  }
}

void test_nightrider(){
  uint8_t idxs[15] = {4,13,22,30,36,40,43,46,49,52,56,62,70,79,88};

  if (state_step < 15)
  {
    // right to left
    tie_off();
    leds[idxs[state_step]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    leds[idxs[state_step]-1] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    leds[idxs[state_step]+1] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    FastLED.show();
    delay(20);
  }

  if(state_step == 15)
  {
    //pause
    delay(500);
  }

  palette_step++;
  if(state_step >= 15)
  {
    //left to right
    // max state_step is 30
    uint8_t idx = 29 - state_step;
    tie_off();
    leds[idxs[idx]] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    leds[idxs[idx]-1] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    leds[idxs[idx]+1] = ColorFromPalette( currentPalette, palette_step, 255, currentBlending);
    FastLED.show();
    delay(20);
  }

  palette_step++;
  if(state_step == 29)
  {
    delay(500);
    state_step = 0;
  }
  else
  {
    state_step++;
  }

  if(state_change_requested == 1)
  {
    // allow state transition immediately
    state_change_allowed = 1;
  }
}
