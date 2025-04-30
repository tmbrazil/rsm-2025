#include <IRremote.h>
#include <Servo.h>

// =================== DEFINIÇÕES DE PINOS ===================
// Motores (Ponte H)
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

// Sensores
const int trigPin = 46;  // Ultrassom HC-SR04
const int echoPin = 44;
const int IR_PIN = 19;   // Receptor IR (failsafe)

// LEDs
const int LED_MARCO_PIN = 13;   // Sinalização de marcos
const int LED_STATUS_PIN = 22;  // Status do sistema

// Servos
const int servoEsqPin = 12;
const int servoDirPin = 45;
Servo servoEsq;
Servo servoDir;

// Comunicação serial com Raspberry Pi
#define SerialRPi Serial  // Conectar via USB

// =================== VARIÁVEIS GLOBAIS ===================
enum EstadoRobo {
  AGUARDANDO_INICIO,
  NAVEGANDO_MARCO_1,
  NAVEGANDO_MARCO_2,
  NAVEGANDO_MARCO_3,
  NAVEGANDO_MARCO_4,
  COMPLETO,
  FALHA
};

EstadoRobo estadoAtual = AGUARDANDO_INICIO;
int marcoAtual = 1;
unsigned long tempoInicioPartida = 0;
const unsigned long TEMPO_MAXIMO_PARTIDA = 600000;  // 10 minutos em ms
String comandoAtual = "STOP";

// =================== CONFIGURAÇÃO INICIAL ===================
void setup() {
  // Configuração dos pinos
  pinMode(ENA_H1, OUTPUT);
  pinMode(ENB_H1, OUTPUT);
  for (int i = 4; i <= 11; i++) pinMode(i, OUTPUT);

  // Sensores e LEDs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_MARCO_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  IrReceiver.begin(IR_PIN);

  // Servos
  servoEsq.attach(servoEsqPin);
  servoDir.attach(servoDirPin);
  alinharServos();

  // Comunicação
  Serial.begin(9600);
  SerialRPi.begin(9600);

  // Sinalização inicial
  piscarLED(LED_STATUS_PIN, 2, 200);
  Serial.println("Sistema Iniciado");
}

// =================== LOOP PRINCIPAL ===================
void loop() {
  // Controle de emergência por IR
  verificarEmergenciaIR();

  // Processar comandos do Raspberry Pi
  processarComandosRPi();

  // Lógica principal de estados
  switch (estadoAtual) {
    case AGUARDANDO_INICIO:
      if (comandoAtual == "INICIAR") iniciarPartida();
      break;

    case NAVEGANDO_MARCO_1:
    case NAVEGANDO_MARCO_2:
    case NAVEGANDO_MARCO_3:
    case NAVEGANDO_MARCO_4:
      executarNavegacao();
      break;

    case COMPLETO:
      finalizarPartida();
      break;

    case FALHA:
      tratarFalha();
      break;
  }

  delay(50);
}

// =================== FUNÇÕES DE CONTROLE ===================
void mover(int velEsq, int velDir) {
  digitalWrite(IN1_H1, velEsq > 0);
  digitalWrite(IN2_H1, velEsq <= 0);
  digitalWrite(IN1_H2, velEsq > 0);
  digitalWrite(IN2_H2, velEsq <= 0);
  digitalWrite(IN3_H1, velDir > 0);
  digitalWrite(IN4_H1, velDir <= 0);
  digitalWrite(IN3_H2, velDir > 0);
  digitalWrite(IN4_H2, velDir <= 0);

  analogWrite(ENB_H1, abs(velEsq));
  analogWrite(ENA_H1, abs(velDir));
}

void pararMotores() {
  mover(0, 0);
}

// =================== FUNÇÕES DE DIREÇÃO ===================
void alinharServos() {
  servoEsq.write(90);
  servoDir.write(90);
}

void girarDireita(int angulo) {
  angulo = constrain(angulo, 0, 45);
  servoEsq.write(90 + angulo);
  servoDir.write(90 + angulo);
}

void girarEsquerda(int angulo) {
  angulo = constrain(angulo, 0, 45);
  servoEsq.write(90 - angulo);
  servoDir.write(90 - angulo);
}

// =================== FUNÇÕES DE SENSORIAMENTO ===================
float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duracao = pulseIn(echoPin, HIGH);
  return (duracao * 0.0343) / 2;  // Distância em cm
}

void desviarObstaculo() {
  pararMotores();
  piscarLED(LED_STATUS_PIN, 2, 100);

  // Verificar ambos os lados
  girarEsquerda(30);
  float distEsq = medirDistancia();
  delay(100);
  
  girarDireita(60);
  float distDir = medirDistancia();
  delay(100);
  
  alinharServos();

  // Tomar decisão
  if (distEsq > distDir && distEsq > 40) {
    for (int i = 0; i <= 30; i += 5) {
      girarEsquerda(i);
      mover(100, 70);
      delay(100);
    }
  } else if (distDir > 40) {
    for (int i = 0; i <= 30; i += 5) {
      girarDireita(i);
      mover(70, 100);
      delay(100);
    }
  } else {
    mover(-80, -80);
    delay(1000);
  }
  comandoAtual = "STOP";  // Resetar comando após desvio
}

// =================== FUNÇÕES DE COMUNICAÇÃO ===================
void processarComandosRPi() {
  if (SerialRPi.available()) {
    String mensagem = SerialRPi.readStringUntil('\n');
    mensagem.trim();

    // Comandos de direção
    if (mensagem == "LEFT" || mensagem == "RIGHT" || 
        mensagem == "FORWARD" || mensagem == "STOP") {
      comandoAtual = mensagem;
    }
    // Detecção de marcos
    else if (mensagem.startsWith("MARCO_")) {
      int novoMarco = mensagem.substring(6).toInt();
      if (novoMarco == marcoAtual + 1) {
        marcoAtual = novoMarco;
        sinalizarMarco();
        if (marcoAtual == 4) estadoAtual = COMPLETO;
      }
    }
  }
}

// =================== FUNÇÕES DE ESTADOS ===================
void iniciarPartida() {
  estadoAtual = NAVEGANDO_MARCO_1;
  tempoInicioPartida = millis();
  Serial.println("Partida Iniciada!");
}

void finalizarPartida() {
  pararMotores();
  digitalWrite(LED_MARCO_PIN, HIGH);
  Serial.println("Desafio Completo!");
}

void tratarFalha() {
  while (true) {
    piscarLED(LED_MARCO_PIN, 5, 100);
    delay(100);
  }
}

// =================== FUNÇÕES AUXILIARES ===================
void verificarEmergenciaIR() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.decodedRawData == 0xF20DFF00) {
      estadoAtual = FALHA;
      pararMotores();
      Serial.println("Parada de Emergência!");
      while (true) piscarLED(LED_STATUS_PIN, 2, 200);
    }
    IrReceiver.resume();
  }
}

void piscarLED(int pin, int vezes, int intervalo) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(pin, HIGH);
    delay(intervalo);
    digitalWrite(pin, LOW);
    if (i < vezes - 1) delay(intervalo);
  }
}

void sinalizarMarco() {
  piscarLED(LED_MARCO_PIN, 3, 200);
  digitalWrite(LED_MARCO_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_MARCO_PIN, LOW);
  Serial.print("Marco ");
  Serial.print(marcoAtual);
  Serial.println(" Detectado!");
}

// =================== LÓGICA DE NAVEGAÇÃO ===================
void executarNavegacao() {
  // Verificação prioritária de obstáculos
  if (medirDistancia() < 25) {
    desviarObstaculo();
    return;
  }

  // Executar comando atual
  if (comandoAtual == "LEFT") {
    girarEsquerda(30);
    mover(80, 100);
  } 
  else if (comandoAtual == "RIGHT") {
    girarDireita(30);
    mover(100, 80);
  } 
  else if (comandoAtual == "FORWARD") {
    alinharServos();
    mover(100, 100);
  }
  else {
    pararMotores();
  }

  // Verificar timeout
  if (millis() - tempoInicioPartida > TEMPO_MAXIMO_PARTIDA) {
    estadoAtual = FALHA;
    Serial.println("Tempo Excedido!");
  }
}
