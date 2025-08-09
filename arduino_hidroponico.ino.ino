#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Config LCD
LiquidCrystal_I2C lcd(0x27,16,2);

// Pines sensor ultrasónico
const int trigPin = 9;
const int echoPin = 8;

// Pines LEDs
const int ledRojo = 4;
const int ledVerde = 5;

// Pin motor (relay)
const int motorPin = 6;

// Tanque dimensiones (cm)
const float diametro = 30.0;
const float alturaTotal = 30.0;
const float radio = diametro / 2.0;
const float areaBase = 3.1416 * radio * radio; // cm²

// Umbral mínimo litros para activar motor
const int litrosMinimos = 5;

// Variables tiempo para motor
unsigned long tiempoAnterior = 0;
const unsigned long intervaloMotor = 3UL * 60UL * 60UL * 1000UL; // 3 horas en ms
const unsigned long tiempoMotorEncendido = 2UL * 60UL * 1000UL;   // 2 minutos en ms
bool motorEncendido = false;
unsigned long tiempoMotorInicio = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); // motor apagado inicialmente

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
}

void loop() {
  // Leer sensor ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.034 / 2.0;

  // Calcular altura agua y limitar
  float alturaAgua = alturaTotal - distancia;
  if (alturaAgua < 0) alturaAgua = 0;
  if (alturaAgua > alturaTotal) alturaAgua = alturaTotal;

  // Volumen y litros
  float volumenCm3 = areaBase * alturaAgua;
  int litros = (int)(volumenCm3 / 1000.0);

  // Mostrar LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Litros: ");
  lcd.print(litros);

  lcd.setCursor(0,1);
  if (litros < litrosMinimos) {
    lcd.print("Nivel Bajo");
    digitalWrite(ledRojo, HIGH);
    digitalWrite(ledVerde, LOW);
  } else {
    lcd.print("Nivel bueno  ");
    digitalWrite(ledRojo, LOW);
    digitalWrite(ledVerde, HIGH);
  }

  // Control motor: solo si nivel OK
  unsigned long tiempoActual = millis();

  if (!motorEncendido && (tiempoActual - tiempoAnterior >= intervaloMotor) && litros >= litrosMinimos) {
    motorEncendido = true;
    tiempoMotorInicio = tiempoActual;
    digitalWrite(motorPin, HIGH); // Enciende motor
    tiempoAnterior = tiempoActual;
  }

  if (motorEncendido && (tiempoActual - tiempoMotorInicio >= tiempoMotorEncendido)) {
    motorEncendido = false;
    digitalWrite(motorPin, LOW); // Apaga motor
  }

  // Debug Serial
  Serial.print("Dist cm: ");
  Serial.print(distancia);
  Serial.print(" Litros: ");
  Serial.println(litros);

  delay(500);
}



