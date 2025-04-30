#include <IRremote.h>
#include <Servo.h>

// ========= CONFIGURAÇÃO DOS PINOS =========
// Motores
const int ENA_H1 = 2;   // PWM motor direito traseiro
const int ENB_H1 = 3;   // PWM motor esquerdo traseiro

const int IN1_H1 = 7;   // Controle motor esquerdo traseiro
const int IN2_H1 = 6;
const int IN3_H1 = 5;   // Controle motor direito traseiro
const int IN4_H1 = 4;

const int IN1_H2 = 8;   // Controle motor esquerdo dianteiro
const int IN2_H2 = 9;
const int IN3_H2 = 10;  // Controle motor direito dianteiro
const int IN4_H2 = 11;

// Servos
const int servoEsqPin = 12;
const int servoDirPin = 45;
Servo servoEsq;
Servo servoDir;

// Ultrassom
const int trigPin = 46;
const int echoPin = 44;

// Controle IR
const int infraRedPin = 19;
bool turnedOn = false;

// ========= PARÂMETROS DE CONTROLE =========
const int offsetMax = 30;     // Máximo desvio angular dos servos

// Navegação
const unsigned long correctionInterval = 500;
unsigned long lastCorrectionTime = 0;
int lastCorrection = 0;       // -1:esq, 0:centro, 1:dir

// ========= FUNÇÕES BÁSICAS =========
void setup() {
  // Configura pinos dos motores
  pinMode(ENA_H1, OUTPUT);
  pinMode(ENB_H1, OUTPUT);
  for (int i = 4; i <= 11; i++) pinMode(i, OUTPUT);

  // Inicializa sensores e atuadores
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  IrReceiver.begin(infraRedPin);
  servoEsq.attach(servoEsqPin);
  servoDir.attach(servoDirPin);
  
  Serial.begin(9600);
  alinharServo();
  Serial.println("Sistema iniciado");
}

void setVelocidadeMotores(int velEsq, int velDir) {
  analogWrite(ENB_H1, velEsq);  // Motor esquerdo
  analogWrite(ENA_H1, velDir);  // Motor direito
}

void mover(int velEsq, int velDir) {
  // Controle de direção
  digitalWrite(IN1_H1, velEsq > 0);
  digitalWrite(IN2_H1, velEsq <= 0);
  digitalWrite(IN1_H2, velEsq > 0);
  digitalWrite(IN2_H2, velEsq <= 0);
  
  digitalWrite(IN3_H1, velDir > 0);
  digitalWrite(IN4_H1, velDir <= 0);
  digitalWrite(IN3_H2, velDir > 0);
  digitalWrite(IN4_H2, velDir <= 0);

  setVelocidadeMotores(abs(velEsq), abs(velDir));
}

void pararMotor() {
  mover(0, 0);
  Serial.println("Motores parados");
}

// ========= CONTROLE DOS SERVOS =========
void alinharServo() {
  servoEsq.write(90);
  servoDir.write(90);
  lastCorrection = 0;
  delay(300);
}

void giroDireita(int angle) {
  angle = constrain(angle, 0, offsetMax);
  servoEsq.write(90 + angle);
  servoDir.write(90 + angle);
  Serial.print("Giro DIREITA: ");
  Serial.print(angle);
  Serial.println(" graus");
  delay(300);
}

void giroEsquerda(int angle) {
  angle = constrain(angle, 0, offsetMax);
  servoEsq.write(90 - angle);
  servoDir.write(90 - angle);
  Serial.print("Giro ESQUERDA: ");
  Serial.print(angle);
  Serial.println(" graus");
  delay(300);
}

// ========= NAVEGAÇÃO =========
double medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  double distance = duration * 0.0343 / 2;
  
  Serial.print("Distância: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  return distance;
}

void decidirCurva() {
  pararMotor();
  delay(500);
  
  // Verifica três direções
  alinharServo();
  double frente = medirDistancia();
  
  giroEsquerda(30);  // Olha para esquerda
  double esquerda = medirDistancia();
  
  giroDireita(60);   // Olha para direita (30° a partir do centro)
  double direita = medirDistancia();
  
  alinharServo();
  
  // Toma decisão
  if(esquerda > direita && esquerda > 40) {
    Serial.println("Decisão: Virar ESQUERDA");
    curvaEsquerda();
  } 
  else if(direita > 40) {
    Serial.println("Decisão: Virar DIREITA");
    curvaDireita();
  } 
  else {
    Serial.println("Decisão: Dar ré e tentar novamente");
    mover(-100, -100);
    delay(1000);
    decidirCurva();
  }
}

void curvaEsquerda() {
  Serial.println("Executando curva para ESQUERDA");
  for(int i = 0; i <= offsetMax; i += 5) {
    giroEsquerda(i);
    mover(100, 70);  // Motor direito mais lento
    delay(100);
  }
  delay(800);
  alinharServo();
}

void curvaDireita() {
  Serial.println("Executando curva para DIREITA");
  for(int i = 0; i <= offsetMax; i += 5) {
    giroDireita(i);
    mover(70, 100);  // Motor esquerdo mais lento
    delay(100);
  }
  delay(800);
  alinharServo();
}

void correcaoTrajetoria() {
  double dist = medirDistancia();
  
  if(dist < 30) {  // Muito perto
    if(lastCorrection <= 0) {
      mover(90, 100);  // Ajusta para direita
      lastCorrection = 1;
    } else {
      mover(100, 90);  // Ajusta para esquerda
      lastCorrection = -1;
    }
  } 
  else if(dist > 50) {  // Muito longe
    if(lastCorrection >= 0) {
      mover(100, 90);
      lastCorrection = -1;
    } else {
      mover(90, 100);
      lastCorrection = 1;
    }
  } 
  else {  // Distância ideal
    mover(100, 100);
    lastCorrection = 0;
  }
  
  Serial.print("Correção: ");
  Serial.println(lastCorrection);
}

// ========= LOOP PRINCIPAL =========
void loop() {
  // Controle por IR
  if(IrReceiver.decode()) {
    unsigned long codigo = IrReceiver.decodedIRData.decodedRawData;
    
    if(codigo == 0xE916FF00) {  // Botão ON
      turnedOn = true;
      Serial.println("Sistema ATIVADO");
      mover(100, 100);
    } 
    else if(codigo == 0xF20DFF00) {  // Botão OFF
      turnedOn = false;
      Serial.println("Sistema DESATIVADO");
      pararMotor();
      alinharServo();
    }
    IrReceiver.resume();
  }

  // Lógica de navegação
  if(turnedOn) {
    if(medirDistancia() < 25) {
      decidirCurva();
    }
    
    if(millis() - lastCorrectionTime > correctionInterval) {
      correcaoTrajetoria();
      lastCorrectionTime = millis();
    }
  }
  
  delay(100);
}
