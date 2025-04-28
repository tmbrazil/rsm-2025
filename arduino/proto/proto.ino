#include <Servo.h>

// PONTE H TRASEIRA
const int ENA_H1 = 2;
const int ENB_H1 = 3;
const int IN1_H1 = 7;
const int IN2_H1 = 6;
const int IN3_H1 = 5;
const int IN4_H1 = 4;

// PONTE H DIANTEIRA
const int IN1_H2 = 8;
const int IN2_H2 = 9;
const int IN3_H2 = 10;
const int IN4_H2 = 11;
const int ENA_H2 = 3;
const int ENB_H2 = 2;

// SERVOS
const int SERVO_ESQ_PIN = 12;
const int SERVO_DIR_PIN = 13;

// ULTRASSOM
const int trigPin = 44;
const int echoPin = 46;

Servo servoEsq;
Servo servoDir;

void setup() {
  pinMode(ENA_H1, OUTPUT);
  pinMode(ENB_H1, OUTPUT);

  pinMode(IN1_H1, OUTPUT);
  pinMode(IN2_H1, OUTPUT);
  pinMode(IN3_H1, OUTPUT);
  pinMode(IN4_H1, OUTPUT);


  pinMode(ENA_H2, OUTPUT);
  pinMode(ENB_H2, OUTPUT);

  pinMode(IN1_H2, OUTPUT);
  pinMode(IN2_H2, OUTPUT);
  pinMode(IN3_H2, OUTPUT);
  pinMode(IN4_H2, OUTPUT);

  servoEsq.attach(SERVO_ESQ_PIN);
  servoDir.attach(SERVO_DIR_PIN);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);
}

// VELOCIDADE (0 a 255)
void setVelocidadeMotores(int velocidadeEsq, int velocidadeDir) {
  analogWrite(ENA_H1, velocidadeEsq); // Traseira esquerda
  analogWrite(ENB_H2, velocidadeEsq); // Dianteira esquerda

  analogWrite(ENA_H2, velocidadeDir); // Traseira direita
  analogWrite(ENB_H1, velocidadeDir); // Dianteira direita
}

void mover(int velEsq, int velDir) {
  // Direção do lado esquerdo
  bool frenteEsq = velEsq >= 0;
  digitalWrite(IN1_H1, frenteEsq);
  digitalWrite(IN2_H1, !frenteEsq);
  digitalWrite(IN1_H2, frenteEsq);
  digitalWrite(IN2_H2, !frenteEsq);

  // Direção do lado direito
  bool frenteDir = velDir >= 0;
  digitalWrite(IN3_H1, frenteDir);
  digitalWrite(IN4_H1, !frenteDir);
  digitalWrite(IN3_H2, frenteDir);
  digitalWrite(IN4_H2, !frenteDir);

  // Aplica a velocidade
  int pwmEsq = abs(velEsq);
  int pwmDir = abs(velDir);

  setVelocidadeMotores(pwmEsq, pwmDir);
}

void pararMotor() {
  mover(0, 0);

  delay(2000);
}

void alinharServo() {
  servoEsq.write(90);
  servoDir.write(90);
  Serial.println("Alinhando servos");
  delay(3000);
}

// 0 -> extremo esquerdo
// 180 -> extremo direito
void giroDireita(int angle) {
  Serial.println("Girando: " + String(angle) + "(direita)"); 

  angle = 90 + angle;
  servoEsq.write(angle);
  servoDir.write(angle);
  delay(2000);

  pararMotor();
}

void giroEsquerda(int angle) {
  Serial.println("Girando: " + String(angle) + "(esquerda)");

  angle = 90 - angle;
  servoEsq.write(angle);
  servoDir.write(angle);
  delay(2000);

  pararMotor();
}

void curvaDireita(int angle=45, int velEsq=200, int velDir=150) {
  mover(-100, -100);

  giroDireita(angle);

  //velocidade esquerdo maior que direito
  mover(velEsq, velDir);

  alinharServo();
  pararMotor();
}

void curvaEsquerda(int angle=45, int velEsq=150, int velDir=200) {
  mover(-100, -100);

  giroEsquerda(angle);

  //velocidade direito maior que esquerdo
  mover(velEsq, velDir);

  alinharServo();
  pararMotor();
}

void desvioObstaculo() {
  pararMotor();
  
  double distanciaFrente = getDistance();

  if (distanciaFrente < 20) {
    Serial.println("Obstáculo detectado à frente!");

    alinharServo();

    double distanciaEsquerda = 0;
    double distanciaDireita = 0;

    // Curva leve para a esquerda
    Serial.println("Curvando levemente para a esquerda para verificar...");
    curvaEsquerda(velEsq=50, velDir=100);
    delay(400);
    distanciaEsquerda = getDistance();
    delay(300);

    mover(-100, -100);

    // Curva leve para a direita
    Serial.println("Curvando levemente para a direita para verificar...");
    curvaEsquerda(velEsq=100, velDir=50);
    delay(400);
    distanciaDireita = getDistance();
    delay(300);

    // Voltar para posição inicial
    Serial.println("Voltando para a posição inicial...");
    mover(-100, -100);
    delay(400);
    pararMotor();
    alinharServo();

    Serial.println("Distância esquerda: " + String(distanciaEsquerda));
    Serial.println("Distância direita: " + String(distanciaDireita));

    if (distanciaEsquerda > distanciaDireita) {
      curvaEsquerda(45);
    } else {
      curvaDireita(45);
    }

    mover(200, 200);
    delay(1000);
    pararMotor();
  }
}

double getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.0343 / 2;
  delay(500);

  Serial.println("Distancia ultrassom: " + String(distance));

  return distance;
}

void loop() {

  mover(200, 200);
  pararMotor();

  mover(-200, -200);
  pararMotor();

  mover(200, 200);
  pararMotor();
  
}
