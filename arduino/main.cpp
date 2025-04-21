#include <hcsr04.h>
#include <Servo.h>

const int IN1_H1 = 4;
const int IN2_H1 = 5;
const int IN3_H1 = 6;
const int IN4_H1 = 7;

const int IN1_H2 = 8;
const int IN2_H2 = 9;
const int IN3_H2 = 10;
const int IN4_H2 = 11;

const int TRIG_PIN = 12;
const int ECHO_PIN = 13;

HCSR04 ultrassom(TRIG_PIN, ECHO_PIN, 20, 4000);

String comandoRecebido = "";
bool comandoEmExecucao = false;
unsigned long tempoUltimoComando = 0;

void setup() {
  pinMode(IN1_H1, OUTPUT); pinMode(IN2_H1, OUTPUT);
  pinMode(IN3_H1, OUTPUT); pinMode(IN4_H1, OUTPUT);
  pinMode(IN1_H2, OUTPUT); pinMode(IN2_H2, OUTPUT);
  pinMode(IN3_H2, OUTPUT); pinMode(IN4_H2, OUTPUT);

  Serial.begin(9600);
  delay(1000);
}

void moverFrente() {
  digitalWrite(IN1_H1, HIGH); digitalWrite(IN2_H1, LOW);
  digitalWrite(IN3_H1, HIGH); digitalWrite(IN4_H1, LOW);
  digitalWrite(IN1_H2, HIGH); digitalWrite(IN2_H2, LOW);
  digitalWrite(IN3_H2, HIGH); digitalWrite(IN4_H2, LOW);
}

void moverTras() {
  digitalWrite(IN1_H1, LOW); digitalWrite(IN2_H1, HIGH);
  digitalWrite(IN3_H1, LOW); digitalWrite(IN4_H1, HIGH);
  digitalWrite(IN1_H2, LOW); digitalWrite(IN2_H2, HIGH);
  digitalWrite(IN3_H2, LOW); digitalWrite(IN4_H2, HIGH);
}

void pararMotor() {
  digitalWrite(IN1_H1, LOW); digitalWrite(IN2_H1, LOW);
  digitalWrite(IN3_H1, LOW); digitalWrite(IN4_H1, LOW);
  digitalWrite(IN1_H2, LOW); digitalWrite(IN2_H2, LOW);
  digitalWrite(IN3_H2, LOW); digitalWrite(IN4_H2, LOW);
}

void processarComando(String comando) {
  comando.trim();
  comandoEmExecucao = true;

  if (comando == "FRENTE") {
    moverFrente();
    delay(2000);
    pararMotor();
    Serial.println("MOVIDO_FRENTE");
  } else if (comando == "TRAS") {
    moverTras();
    delay(2000);
    pararMotor();
    Serial.println("MOVIDO_TRAS");
  } else if (comando == "PARAR") {
    pararMotor();
    Serial.println("PARADO");
  } else if (comando == "DISTANCIA") {
    float distancia = ultrassom.Distance();
    Serial.print("DISTANCIA_CM:");
    Serial.println(distancia);
  } else {
    Serial.println("COMANDO_INVALIDO");
  }

  comandoRecebido = "";
  comandoEmExecucao = false;
}

void loop() {
  if (Serial.available()) {
    comandoRecebido = Serial.readStringUntil('\n');
    processarComando(comandoRecebido);
  }

  //Loop para o Arduino executar automaticamente
  if (!comandoEmExecucao) {
    moverFrente();
    delay(3000);
    moverTras();
    delay(3000);
    pararMotor();
    delay(3000);
  }
}
