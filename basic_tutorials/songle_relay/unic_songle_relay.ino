// Programa de teste do modulo rele
// Autor : Arduino e Cia.
// Source: https://arduinoecia.com.br/ligando-uma-lampada-com-modulo-rele-arduino/
//
// Ligar o pino S do rele no pino 7 do Arduino
// Ligar o pino + do rele no pino 5V do Arduino
// Ligar o pino - do rele no pino GND do Arduino

//define a saida a ser utilizada para o acionamento do rele
int sinalparaorele = 7; 

void setup()
{
  pinMode(sinalparaorele, OUTPUT); //Define o pino como saida
}

void loop()

{
  digitalWrite(sinalparaorele, HIGH); //Aciona o rele
  delay(5000); //Aguarda 5 segundos
  digitalWrite(sinalparaorele, LOW); //Desliga o rele
  delay(5000); //Aguarda 5 segundos e reinicia o processo
}