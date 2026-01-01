#include <Wire.h>

uint8_t counter = 0;
String frq_str;
float frequency;
bool update;

const uint8_t samples[100] = {
  127, 135, 143, 151, 159, 166, 174, 181, 188, 195, 202, 208, 214, 220,
  225, 230, 235, 239, 242, 246, 248, 250, 252, 253, 254, 255, 254, 253,
  252, 250, 248, 246, 242, 239, 235, 230, 225, 220, 214, 208, 202, 195,
  188, 181, 174, 166, 159, 151, 143, 135, 127, 119, 111, 103, 95, 88,
  80, 73, 66, 59, 52, 46, 40, 34, 29, 24, 19, 15, 12, 8, 6, 4, 2,
  1, 0, 0, 0, 1, 2, 4, 6, 8, 12, 15, 19, 24, 29, 34, 40, 46, 52,
  59, 66, 73, 80, 88, 95, 103, 111, 119
};

uint8_t calc_ocr2a(float frq) {
  return round( (16E6) / (frq*8) - 1 );
}

void set_frq(float frq) {
  uint8_t x = calc_ocr2a(frq*100);
  OCR2A = x;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  //set pins as outputs
  DDRD = B11111111;
  cli();//stop interrupts

  //set timer2 interrupt at 10kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for frq increments
  // change to 199 for 10kHz
  OCR2A = calc_ocr2a(10000); // = (16*10^6) / (10000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  sei();//allow interrupts
  update = false;
}

ISR(TIMER2_COMPA_vect) {
  //timer1 interrupt
  PORTD = samples[counter++];
  counter %= 100;
}

void check_input() {
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 10 || ch == 13) {
      if (frq_str.length() > 0) {
        float f = atof(frq_str.c_str());
        if (f>10 && f<30000) {
          frequency = f;
          frq_str = "";
          update = true;
        }
      }
    } else {
      frq_str += ch;
      if (frq_str.length() > 5) {
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
