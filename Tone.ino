#include <Wire.h>

const uint8_t num_samples = 100;
const uint8_t prescaler_1    = 0b001;
const uint8_t prescaler_8    = 0b010;
const uint8_t prescaler_64   = 0b011;
const uint8_t prescaler_256  = 0b100;
const uint8_t prescaler_1024 = 0b101;

uint8_t counter = 0;
String frq_str;
float frequency;
bool update;

const uint8_t samples[num_samples] = {
  127, 135, 143, 151, 159, 166, 174, 181, 188, 195, 202, 208, 214, 220,
  225, 230, 235, 239, 242, 246, 248, 250, 252, 253, 254, 255, 254, 253,
  252, 250, 248, 246, 242, 239, 235, 230, 225, 220, 214, 208, 202, 195,
  188, 181, 174, 166, 159, 151, 143, 135, 127, 119, 111, 103, 95, 88,
  80, 73, 66, 59, 52, 46, 40, 34, 29, 24, 19, 15, 12, 8, 6, 4, 2,
  1, 0, 0, 0, 1, 2, 4, 6, 8, 12, 15, 19, 24, 29, 34, 40, 46, 52,
  59, 66, 73, 80, 88, 95, 103, 111, 119
};

uint16_t calc_ocr1a(float frq)
{
  float result = 16e6 / (frq*1.0*num_samples) - 1;
  Serial.println(result);
  return round(result);
}

void set_frq(float frq) {
  OCR1A = calc_ocr1a(frq);
  Serial.println(OCR1A);
}

ISR(TIMER1_COMPA_vect)
{
  PORTD = samples[counter++];
  counter %= num_samples;
  TCNT1 = 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  //set pins as outputs
  DDRD = B11111111;
  cli();//stop interrupts
  // set timer 1 control register
  TCCR1B = 0;
  TCCR1A = 0;
  TCCR1B |= prescaler_1;
  TCNT1 = 0;
  TIMSK1 |= 0b00000010; // set OCIE1A to 1 = enable compare match A
  // set initial frequency
  OCR1A = calc_ocr1a(5000);

  sei();//allow interrupts
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
    Serial.println(frequency);
    set_frq(frequency);
    update = false;
  }
}
