// Process button inputs and return button activity

#define NUMBUTTONS 2
#define MODEBUTTON 4
#define BRIGHTNESSBUTTON 3

#define BTNIDLE 0
#define BTNDEBOUNCING 1
#define BTNPRESSED 2
#define BTNRELEASED 3
#define BTNLONGPRESS 4
#define BTNLONGPRESSREAD 5
#define BTNGUARDTIME 6

#define BTNDEBOUNCETIME 30
#define BTNLONGPRESSTIME 1500

unsigned long buttonEvents[NUMBUTTONS];
byte buttonStatuses[NUMBUTTONS];
byte buttonmap[NUMBUTTONS] = {BRIGHTNESSBUTTON, MODEBUTTON};
extern const byte numEffectsAudio;
extern const byte numEffectsNoAudio;



void updateButtons() {
  for (byte i = 0; i < NUMBUTTONS; i++) {
    switch (buttonStatuses[i]) {
      case BTNIDLE:
        if (digitalRead(buttonmap[i]) == LOW) {
          buttonEvents[i] = currentMillis;
          buttonStatuses[i] = BTNDEBOUNCING;
        }
        break;

      case BTNDEBOUNCING:
        if (currentMillis - buttonEvents[i] > BTNDEBOUNCETIME) {
          if (digitalRead(buttonmap[i]) == LOW) {
            buttonStatuses[i] = BTNPRESSED;
          }
        }
        break;

      case BTNPRESSED:
        if (digitalRead(buttonmap[i]) == HIGH) {
          buttonStatuses[i] = BTNRELEASED;
        } else if (currentMillis - buttonEvents[i] > BTNLONGPRESSTIME) {
          buttonStatuses[i] = BTNLONGPRESS;
        }
        break;

      case BTNRELEASED:
        break;

      case BTNLONGPRESS:
        break;

      case BTNLONGPRESSREAD:
        if (digitalRead(buttonmap[i]) == HIGH) {
          buttonStatuses[i] = BTNIDLE;
        }
        break;

      case BTNGUARDTIME:
        if (digitalRead(buttonmap[0]) == HIGH && digitalRead(buttonmap[1]) == HIGH) {
          buttonStatuses[i] = BTNIDLE;
        }
    }
  }
}

byte buttonStatus(byte buttonNum) {

  byte tempStatus = buttonStatuses[buttonNum];
  if (tempStatus == BTNRELEASED) {
    buttonStatuses[buttonNum] = BTNIDLE;
  } else if (tempStatus == BTNLONGPRESS) {
    buttonStatuses[buttonNum] = BTNLONGPRESSREAD;
  }

  return tempStatus;

}

void doButtons() {


  if ((buttonStatuses[0] == BTNPRESSED) && (buttonStatuses[1] == BTNPRESSED)) {
    audioEnabled = !audioEnabled; // toggle audio mode
    switch (audioEnabled) {
      case true:
        numEffects = numEffectsAudio;
        break;
      case false:
        numEffects = numEffectsNoAudio;
        break;
    }
    currentEffect = 0;
    effectInit = false;
    confirmBlink(CRGB::DarkGreen, 3);
    buttonStatuses[0] = BTNGUARDTIME;
    buttonStatuses[1] = BTNGUARDTIME;
  } else {

    // Check the mode button (for switching between effects)
    switch (buttonStatus(0)) {

      case BTNRELEASED: // button was pressed and released quickly
        cycleMillis = currentMillis;
        if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
        effectInit = false; // trigger effect initialization when new effect is selected
        eepromMillis = currentMillis;
        eepromOutdated = true;
        break;

      case BTNLONGPRESS: // button was held down for a while
        autoCycle = !autoCycle; // toggle auto cycle mode
        // one blue blink: auto mode. two red blinks: manual mode.
        if (autoCycle) {
          confirmBlink(CRGB::Blue, 1);
        } else {
          confirmBlink(CRGB::Red, 2);
        }
        eepromMillis = currentMillis;
        eepromOutdated = true;
        break;

    }

    // Check the brightness adjust button
    switch (buttonStatus(1)) {

      case BTNRELEASED: // button was pressed and released quickly
        currentBrightness += 51; // increase the brightness (wraps to lowest)
        FastLED.setBrightness(scale8(currentBrightness, MAXBRIGHTNESS));
        eepromMillis = currentMillis;
        eepromOutdated = true;
        break;

      case BTNLONGPRESS: // button was held down for a while
        currentBrightness = STARTBRIGHTNESS; // reset brightness to startup value
        FastLED.setBrightness(scale8(currentBrightness, MAXBRIGHTNESS));
        eepromMillis = currentMillis;
        eepromOutdated = true;
        break;

    }

  }

}
