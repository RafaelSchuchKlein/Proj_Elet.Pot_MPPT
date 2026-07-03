#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

const int pwmPin = 10;
const int pinoDivisor = 7;
const int pinoDisparo = 8;

float tensao = 0;
float corrente = 0;
float potAntes = 0;
int valor_pwm = 30;
int passo = 1;

void setup(){
  pinMode(pinoDivisor, INPUT);
  pinMode(pinoDisparo, OUTPUT);
  digitalWrite(pinoDisparo, LOW);

  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, valor_pwm);

  ACSR = bit(ACI)  |
         bit(ACIE) |
         bit(ACBG) |
         bit(ACIS1);

  TCCR1B = TCCR1B & 0b11111000;
  TCCR1B = TCCR1B | 0b00000001;

  ina219.begin();
}

ISR(ANALOG_COMP_vect) {
  analogWrite(pwmPin, 0);
  digitalWrite(pinoDisparo, HIGH);
  while(true){
  }
}

void loop() {
  tensao = ina219.getBusVoltage_V();
  corrente = ina219.getCurrent_mA() / 1000.00;

  float pot_in = tensao * corrente;

  // --- BLOCO DE SEGURANÇA ALTERADO ---
  if (pot_in > 11.0) {
    valor_pwm = valor_pwm - 5;                 
    valor_pwm = constrain(valor_pwm, 10, 245); 
    analogWrite(pwmPin, valor_pwm);            
    potAntes = pot_in;                        
    delay(50);                                 
    return;                                    
  }
  
  
  float delta_pot = pot_in - potAntes;

  if (delta_pot < 0) {
    passo = -passo;
  }

  valor_pwm = valor_pwm + passo;
  potAntes = pot_in;

  valor_pwm = constrain(valor_pwm, 10, 245);
  analogWrite(pwmPin, valor_pwm);
  delay(50);
}
