#include <Servo.h>

// PONTE H TRASEIRA
// const int ENA_H1;
// const int ENB_H1;
const int IN1_H1 = 7;
const int IN2_H1 = 6;
const int IN3_H1 = 5;
const int IN4_H1 = 4;

// PONTE H DIANTEIRA
const int IN1_H2 = 8;
const int IN2_H2 = 9;
const int IN3_H2 = 10;
const int IN4_H2 = 11;
// const int ENA_H2;
// const int ENB_H2;

// SERVOS
// const int SERVO_ESQ_PIN = 10;
// const int SERVO_DIR_PIN = 3;

// ULTRASSOM
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;


Servo servoEsq;
Servo servoDir;

void setup() {
  // pinMode(ENA_H1, OUTPUT);
  // pinMode(ENB_H1, OUTPUT);

  pinMode(IN1_H1, OUTPUT);
  pinMode(IN2_H1, OUTPUT);
  pinMode(IN3_H1, OUTPUT);
  pinMode(IN4_H1, OUTPUT);


  // pinMode(ENA_H2, OUTPUT);
  // pinMode(ENB_H2, OUTPUT);

  pinMode(IN1_H2, OUTPUT);
  pinMode(IN2_H2, OUTPUT);
  pinMode(IN3_H2, OUTPUT);
  pinMode(IN4_H2, OUTPUT);

  // servoEsq.attach(SERVO_ESQ_PIN);
  // servoDir.attach(SERVO_DIR_PIN);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);
}

// VELOCIDADE (0 a 255)
// MOTOR: TRASEIRO (T) ou DIANTEIRO (D)
// void setVelocidadeMotores(char motor, int velocidadeEsq, int velocidadeDir) {
//   if (motor == 'T') {
//     analogWrite(ENA_H1, velocidadeEsq);
//     analogWrite(ENB_H1, velocidadeDir);
//   } else if (motor == 'D') {
//     analogWrite(ENA_H2, velocidadeEsq);
//     analogWrite(ENB_H2, velocidadeDir);
//   }
// }

void moverFrente() {
  digitalWrite(IN1_H1, HIGH);
  digitalWrite(IN1_H2, HIGH);

  digitalWrite(IN2_H1, LOW);
  digitalWrite(IN2_H2, LOW);

  digitalWrite(IN3_H1, HIGH);
  digitalWrite(IN3_H2, HIGH);

  digitalWrite(IN4_H1, LOW);
  digitalWrite(IN4_H2, LOW);

  delay(3000);
}

void moverTras() {
  digitalWrite(IN1_H1, LOW);
  digitalWrite(IN1_H2, LOW);

  digitalWrite(IN2_H1, HIGH);
  digitalWrite(IN2_H2, HIGH);

  digitalWrite(IN3_H1, LOW);
  digitalWrite(IN3_H2, LOW);

  digitalWrite(IN4_H1, HIGH);
  digitalWrite(IN4_H2, HIGH);

  delay(3000);
}

void pararMotor() {
  digitalWrite(IN1_H1, LOW);
  digitalWrite(IN1_H2, LOW);

  digitalWrite(IN2_H1, LOW);
  digitalWrite(IN2_H2, LOW);

  digitalWrite(IN3_H1, LOW);
  digitalWrite(IN3_H2, LOW);

  digitalWrite(IN4_H1, LOW);
  digitalWrite(IN4_H2, LOW);
  delay(2000);
}

// 0 -> extremo esquerdo
// 180 -> extremo direito
void giroDireita(int angle) {
  Serial.println("Girando: " + String(angle) + "(direita)"); 

  angle = 90 + angle;
  servoEsq.write(angle);
  servoDir.write(angle);
  delay(3000);
}

void giroEsquerda(int angle) {
  Serial.println("Girando: " + String(angle) + "(esquerda)");

  angle = 90 - angle;
  servoEsq.write(angle);
  servoDir.write(angle);
  delay(3000);

}

void alinharServo() {
  servoEsq.write(90);
  servoDir.write(90);
  Serial.println("Alinhando servos");
  delay(3000);
}

double getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.0343 / 2;
  delay(500);

  Serial.println("Distancia ultrassom: " + String(distance));

  return distance;
}

void loop() {
  // double distanceUltrassom = getDistance();

  // Obstáculo detectado
  // if (distanceUltrassom < 15) {
  //    giroDireita(90);
  //  }
  moverFrente();
  pararMotor();

  moverTras();
  pararMotor();

  moverFrente();
  pararMotor();
  
  // giroDireita(45);
  // giroEsquerda(60);
  // delay(10);
  // alinharServo();
  // giroEsquerda();
}
