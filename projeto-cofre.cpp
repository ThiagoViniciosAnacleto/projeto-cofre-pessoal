	//Versão 7 - Cofre de uso pessoal, com senha e alertas sonoros.

	//Autores: Thiago Anacleto, Fernando Schezaro e Matheus Stoco

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

char* senha = "ABC123"; // Defina a senha aqui, ela deve conter seis dígitos podendo utilizar os caracteres do char "digitos"

const byte LINHAS = 4;
const byte COLUNAS = 4;

char digitos[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinosLinha[LINHAS] = { 11, 10, 9, 8 };
byte pinosColuna[COLUNAS] = { 7, 6, 5, 4 };

Keypad keypad = Keypad(makeKeymap(digitos), pinosLinha, pinosColuna, LINHAS, COLUNAS);

int position = 0;
int ledVermelho = 12;
int ledVerde = 13;
Servo microServo;
int buzzer = 2;
int tempoBuzz = 50;
int tempoAberto = 4000;
int tentativasIncorretas = 0;
int limiteTentativas = 3;
int inicioAlarme = 0;
bool alarmeAtivo = false;
int tempoPisca = 0;
int intervaloPisca = 300;

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  estadoPorta(true);// porta fechada
  pinMode(ledVerde, OUTPUT); // inicia desligado
  pinMode(ledVermelho, OUTPUT); // inicia desligado
  pinMode(buzzer, OUTPUT); // inicia desligado

  microServo.attach(3); // micro motor fica na porta digital 3
  Serial.println("Sistema Iniciado");
}

void loop() {
  char digito = keypad.getKey();

  if (digito) {
    Serial.print("Digito pressionado: "); // apenas registro
    Serial.println(digito); // apenas registro

    if (position < 6) { 
      lcd.setCursor(position, 1);
      lcd.print('*');
      digitalWrite(buzzer, HIGH);
      delay(tempoBuzz);
      digitalWrite(buzzer, LOW);

      if (digito == senha[position]) {
        position++;
        if (position == 6) {
          lcd.clear();
          lcd.print("Porta Aberta");
          Serial.println("Senha correta, acesso permitido");
          estadoPorta(false); // inicia função de abrir a porta
          delay(2000); // 2 segundos de delay
          estadoPorta(true); // volta para porta fechada
          tentativasIncorretas = 0;
        }
      } else {
        tentativasIncorretas++;
        Serial.println("Senha incorreta, tente novamente");
        position = 0;

        if (tentativasIncorretas >= limiteTentativas && !alarmeAtivo) {
          ativarAlarme();
          return;
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Senha Incorreta");
        delay(500);
        lcd.clear();
        estadoPorta(true); // volta estado inicial para porta fechada
      }
    }
  }

  if (alarmeAtivo) {
    int tempoAtual = millis();

    if (tempoAtual - inicioAlarme >= 10000) {
      desativarAlarme();
      estadoPorta(true); // volta estado para porta fechada
      tentativasIncorretas = 0; // redefine número de tentativas de senha incorretas
    } else {
      if (tempoAtual - tempoPisca >= intervaloPisca) { //intervalo 300 milisegundos
        toggleAlarme();
        lcd.clear();
        lcd.print("Bloqueado");
        tempoPisca = tempoAtual;
      }
    }
  }
}

void estadoPorta(bool pedirSenha) {
  if (pedirSenha) {
    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledVerde, LOW);
    microServo.write(0);
    lcd.setCursor(0, 0);
    lcd.print("Digite a Senha");
    position = 0;
  } else {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledVerde, HIGH);
    microServo.write(90);
    digitalWrite(buzzer, HIGH);
    delay(tempoBuzz);
    digitalWrite(buzzer, LOW);
    delay(tempoAberto);
    microServo.write(0);
  }
}

void ativarAlarme() {
  alarmeAtivo = true;
  inicioAlarme = millis();
  Serial.println("Alarme ativado");
}

void desativarAlarme() {
  alarmeAtivo = false;
  digitalWrite(ledVermelho, LOW);
  digitalWrite(buzzer, LOW);
  Serial.println("Alarme desativado");
}

void toggleAlarme() {
  static bool estado = false;
  if (estado) {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(buzzer, LOW);
  } else {
    digitalWrite(ledVermelho, HIGH);
    digitalWrite(buzzer, HIGH);
  }
  estado = !estado;
}