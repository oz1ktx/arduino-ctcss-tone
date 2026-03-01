#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

typedef struct Settings
{
  float frequency;
};

const uint8_t num_samples = 100;

uint8_t counter = 0;
String frq_str;
float frequency;
bool update;
Settings settings;

void loadSettings()
{
  EEPROM.get(0, settings);
  frequency = settings.frequency;
}

void saveSettings()
{
  settings.frequency = frequency;
  EEPROM.put(0, settings);
}

/** Set up Timer1 for a given frequency (Hz)
 Returns true if the frequency could be configured, false otherwise.
*/
bool timer1_setFrequency(uint32_t freqHz)
{
  // ----- 1. Choose prescaler -------------------------------------------------
  const struct {
    uint16_t prescale;
    uint8_t csBits;          // CS12..CS10 bits for TCCR1B
  } presc[] = {
    {1,    (1 << CS10)},
    {8,    (1 << CS11)},
    {64,   (1 << CS11) | (1 << CS10)},
    {256,  (1 << CS12)},
    {1024, (1 << CS12) | (1 << CS10)}
  };

  uint16_t chosenPrescale = 0;
  uint8_t   csBits = 0;
  uint16_t ocr = 0;

  for (uint8_t i = 0; i < sizeof(presc)/sizeof(presc[0]); ++i) {
    // compute OCR value for this prescaler
    uint32_t tmp = (F_CPU / (presc[i].prescale * (uint64_t)freqHz)) - 1UL;
    // fits in 16‑bit?
    if (tmp <= 0xFFFF) {
      chosenPrescale = presc[i].prescale;
      csBits = presc[i].csBits;
      ocr = (uint16_t)tmp;
      // take the first (smallest) prescaler that works
      break;
    }
  }

  if (chosenPrescale == 0) return false;   // frequency out of range

  // ----- 2. Stop timer while we reconfigure ---------------------------------
  TCCR1B = 0;                     // no clock source → timer stopped

  // ----- 3. Set CTC mode (WGM12 = 1) -----------------------------------------
  TCCR1A = 0;                     // normal port operation
  TCCR1B = (1 << WGM12);          // CTC mode, keep CS bits cleared for now

  // ----- 4. Load compare register ---------------------------------------------
  OCR1A = ocr;                    // value we calculated above

  // ----- 5. Enable interrupt -------------------------------------------------
  TIMSK1 = (1 << OCIE1A);         // Output‑Compare‑A Match Interrupt Enable

  // ----- 6. Start timer with selected prescaler -------------------------------
  TCCR1B |= csBits;               // start counting

  return true;
}


const uint8_t samples[num_samples] = {
  127, 135, 143, 151, 159, 166, 174, 181, 188, 195, 202, 208, 214, 220,
  225, 230, 235, 239, 242, 246, 248, 250, 252, 253, 254, 255, 254, 253,
  252, 250, 248, 246, 242, 239, 235, 230, 225, 220, 214, 208, 202, 195,
  188, 181, 174, 166, 159, 151, 143, 135, 127, 119, 111, 103, 95, 88,
  80, 73, 66, 59, 52, 46, 40, 34, 29, 24, 19, 15, 12, 8, 6, 4, 2,
  1, 0, 0, 0, 1, 2, 4, 6, 8, 12, 15, 19, 24, 29, 34, 40, 46, 52,
  59, 66, 73, 80, 88, 95, 103, 111, 119
};

bool set_frq(float frq) {
  return timer1_setFrequency(100*frq);
}

ISR(TIMER1_COMPA_vect)
{
  PORTD = samples[counter++];
  counter %= num_samples;
}

void setup() {
  // set all port D to output
  DDRD = 0xFF;
  Serial.begin(9600);
  Serial.println("Start");
  loadSettings();
  if (!isnan(settings.frequency) && settings.frequency > 30 && settings.frequency < 500) {
    set_frq(settings.frequency);
  } else {
    set_frq(100);
  }
  update = false;
}

void check_input() {
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 10 || ch == 13) {
      if (frq_str.length() > 0) {
        float f = atof(frq_str.c_str());
        if (f>10 && f<300000) {
          frequency = f;
          frq_str = "";
          update = true;
        }
      }
    } else {
      frq_str += ch;
      if (frq_str.length() > 8) {
        frq_str = "";
      }
    }
  }
}

void loop() {
  delay(100);
  check_input();
  if (update) {
    Serial.print("Trying to set frequency "); Serial.println(frequency);
    if (set_frq(frequency)) {
      saveSettings();
      Serial.println("Success");
    } else {
      Serial.println("Out of range");
    }
    update = false;
  }
}
