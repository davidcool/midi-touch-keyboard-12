// ---------------------------------------------------------------------------
// Touch Key 12
// MIDI Capacitive Touch 12 Key Keyboard Firmware
// touch key 12 v1.0
// David Cool
// http://davidcool.com
//
// Project archive: https://github.com/davidcool/Mr-Touch-Key
//
// Firmware for a MIDI (Musical Instrument Digital Interface) keyboard using an MPR121 touch sensor breakout
// and Arduino Pro Micro 32u4 (Leonardo)
// 
// Uses the MIDIUSB library: https://github.com/arduino-libraries/MIDIUSB
// Uses the Adafruit MPR121 library: https://github.com/adafruit/Adafruit_MPR121
//
// ---------------------------------------------------------------------------

#include "MIDIUSB.h"          // MIDIUSB library
#include "Wire.h"             // I2C data bus library
#include "Adafruit_MPR121.h"  // touch sensor library

#define BUTTONS  12
#define IRQ_PIN  9            // The MPR121 will use the IRQ to let the controller know something has changed

// mpr121 init
Adafruit_MPR121 cap = Adafruit_MPR121();

// keep track of touched keys
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// MIDI note mapping
// Octave   Note Numbers
//          C   C#  D   D#  E   F   F#  G   G#  A   A#  B
//    0     0   1   2   3   4   5   6   7   8   9   10  11
//    1     12  13  14  15  16  17  18  19  20  21  22  23
//    2     24  25  26  27  28  29  30  31  32  33  34  35
//    3     36  37  38  39  40  41  42  43  44  45  46  47
//    4     48  49  50  51  52  53  54  55  56  57  58  59
//    5     60  61  62  63  64  65  66  67  68  69  70  71
//    6     72  73  74  75  76  77  78  79  80  81  82  83
//    7     84  85  86  87  88  89  90  91  92  93  94  95
//    8     96  97  98  99  100 101 102 103 104 105 106 107
//    9     108 109 110 111 112 113 114 115 116 117 118 119
//    10    120 121 122 123 124 125 126 127

// prime dynamic values
int channel = 0;

// C3 major
//int pitch[] = {36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55};

// F# / Gb Major ( F#, G#, A#, B, C#, D#, F, F# )
//int pitch[] = {66, 68, 70, 71, 73, 75, 77, 78, 80, 82, 83, 85};

// E4 major ( E, F#, G#, A, B, C#, D#, E ) 
//int pitch[] = {52, 54, 56, 57, 59, 61, 63, 64, 66, 68, 69, 71};

// Dm Pentatonic Scale ( D, F, G, A, C, D )
int pitch[] = {38, 41, 43, 45, 48, 50, 53, 55, 57, 60, 62, 65};

int vel[] = {80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80};

void setup() {

  // set mpr121 IRQ pin to input
  pinMode(IRQ_PIN, INPUT);

  // endless loop if the mpr121 init fails
  if (! cap.begin(0x5A))
    while (1);

  // set sensitivity of touch surface, lower numbers increase sensitivity
  cap.setThreshholds(12, 1);
}

void loop() {

  // if mpr121 irq goes low, there have been
  // changes to the button states, so read the values
  if (digitalRead(IRQ_PIN) == LOW)
    readButtons();

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                       BUTTON PRESS AND MIDI FUNCTIONS                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// read the currently touched buttons
void readButtons() {

  // read current values
  currtouched = cap.touched();

  handle_note();

  // save current values to compare against in the next loop
  lasttouched = currtouched;

}

// deal with note on and off presses
void handle_note() {

  for (uint8_t i = 0; i < BUTTONS; i++) {

    // note on check
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {

      // play pressed note
      noteOn(channel, pitch[i], vel[i]);

    }

    // note off check
    if ( !(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {

      // play note off
      noteOff(channel, pitch[i], vel[i]);

    }

  }

}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}
