#include <Arduino.h>

// =============================================================
// Sensor de Ré - ESP32
// HC-SR04 + 6 LEDs + Buzzer passivo
// =============================================================

// Sensor ultrassônico
const int PIN_TRIG = 5;
const int PIN_ECHO = 18;

// Buzzer passivo
const int PIN_BUZZER = 21;
const int FREQUENCIA_BUZZER = 2500;

// LEDs: longe → perto
const int LED_PINS[] = {
  15, // Verde 1
  2,  // Verde 2
  4,  // Amarelo 1
  22, // Amarelo 2
  23, // Vermelho 1
  19  // Vermelho 2
};

const int NUM_LEDS = 6;

// Intervalo entre leituras
const unsigned long READ_INTERVAL = 70;

unsigned long ultimaLeitura = 0;
unsigned long ultimoEventoBuzzer = 0;

int falhasConsecutivas = 0;
bool buzzerLigado = false;

// Protótipos
float medirDistancia();

void atualizarLEDs(float distancia);
void atualizarBuzzer(float distancia);

void ligarBuzzer();
void desligarBuzzer();
void apagarLEDs();

// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);

  // HC-SR04
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // Buzzer passivo
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  // LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  Serial.println();
  Serial.println("Sensor de re iniciado");
  Serial.println("TRIG: GPIO 5");
  Serial.println("ECHO: GPIO 18");
  Serial.println("BUZZER: GPIO 21");
  Serial.println("Iniciando medicoes...");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura < READ_INTERVAL) {
    return;
  }

  ultimaLeitura = agora;

  float distancia = medirDistancia();

  if (distancia < 0.0f) {
    falhasConsecutivas++;

    Serial.println("Sem leitura do sensor");

    desligarBuzzer();

    if (falhasConsecutivas >= 3) {
      apagarLEDs();
    }

    return;
  }

  falhasConsecutivas = 0;

  Serial.print("Distancia: ");
  Serial.print(distancia, 1);
  Serial.println(" cm");

  atualizarLEDs(distancia);
  atualizarBuzzer(distancia);
}

// =============================================================
// MEDIÇÃO DA DISTÂNCIA
// =============================================================
float medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracao = pulseIn(
    PIN_ECHO,
    HIGH,
    30000UL
  );

  if (duracao == 0) {
    return -1.0f;
  }

  float distancia = duracao * 0.0343f / 2.0f;

  if (distancia < 2.0f || distancia > 150.0f) {
    return -1.0f;
  }

  return distancia;
}

// =============================================================
// CONTROLE DOS LEDs
// =============================================================
void atualizarLEDs(float distancia) {
  int ledsAcesos = 0;

  if (distancia > 45.0f || distancia < 2.0f) {
    ledsAcesos = 0;
  } else if (distancia > 30.0f) {
    ledsAcesos = 1;
  } else if (distancia > 20.0f) {
    ledsAcesos = 2;
  } else if (distancia > 15.0f) {
    ledsAcesos = 3;
  } else if (distancia > 10.0f) {
    ledsAcesos = 4;
  } else if (distancia > 5.0f) {
    ledsAcesos = 5;
  } else {
    ledsAcesos = 6;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(
      LED_PINS[i],
      i < ledsAcesos ? HIGH : LOW
    );
  }
}

// =============================================================
// CONTROLE DO BUZZER
// =============================================================
void atualizarBuzzer(float distancia) {
  unsigned long agora = millis();

  unsigned long intervaloApito = 0;
  unsigned long duracaoApito = 100;

  if (distancia > 45.0f || distancia < 2.0f) {
    desligarBuzzer();
    return;
  }

  if (distancia > 30.0f) {
    intervaloApito = 900;
    duracaoApito = 100;
  } else if (distancia > 20.0f) {
    intervaloApito = 650;
    duracaoApito = 100;
  } else if (distancia > 15.0f) {
    intervaloApito = 450;
    duracaoApito = 100;
  } else if (distancia > 10.0f) {
    intervaloApito = 300;
    duracaoApito = 90;
  } else if (distancia > 5.0f) {
    intervaloApito = 150;
    duracaoApito = 70;
  } else {
    intervaloApito = 50;
    duracaoApito = 50;
  }

  if (buzzerLigado) {
    if (agora - ultimoEventoBuzzer >= duracaoApito) {
      desligarBuzzer();
    }
  } else {
    if (agora - ultimoEventoBuzzer >= intervaloApito) {
      ligarBuzzer();
    }
  }
}

// =============================================================
// LIGA O BUZZER
// =============================================================
void ligarBuzzer() {
  if (!buzzerLigado) {
    tone(PIN_BUZZER, FREQUENCIA_BUZZER);

    buzzerLigado = true;
    ultimoEventoBuzzer = millis();
  }
}

// =============================================================
// DESLIGA O BUZZER
// =============================================================
void desligarBuzzer() {
  if (buzzerLigado) {
    noTone(PIN_BUZZER);

    buzzerLigado = false;
    ultimoEventoBuzzer = millis();
  }
}

// =============================================================
// APAGA TODOS OS LEDs
// =============================================================
void apagarLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}