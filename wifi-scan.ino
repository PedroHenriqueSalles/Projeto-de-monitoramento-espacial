#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define PIN_LDR     34
#define PIN_BUZZER  23 
#define TRIG_AGUA   13 
#define ECHO_AGUA   12
#define TRIG_COMB   14 
#define ECHO_COMB   27
#define TRIG_MANT   26 
#define ECHO_MANT   25

#define WLAN_SSID       ("Wokwi-GUEST")
#define WLAN_PASS       ""
#define AIO_SERVER      ("io.adafruit.com")
#define AIO_SERVERPORT  (inserir)
#define AIO_USERNAME    ("inserir")
#define AIO_KEY         ("inserir")

const float CRIT_COMB_MIN = 20.0, CRIT_AGUA_MIN = 15.0, CRIT_MANT_MIN = 10.0;     
const float CRIT_BATERIA_MIN = 15.0, CRIT_TEMP_MIN = 15.0, CRIT_TEMP_MAX = 35.0;
const float CRIT_PRES_MIN = 900.0, CRIT_PRES_MAX = 1100.0;   

float ultimaPres = 1013.25, ultimoComb = 100, ultimaAgua = 100;
unsigned long ultimoEnvioMQTT = 0, ultimaLeituraSensores = 0, transicaoBuzzer = 0;
const long intervaloMQTT = 5000, intervaloSensores = 2000;
bool buzzerAtivo = false, estadoAlarmeGlobal = false;

Adafruit_BMP085 bmp;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish pAgua = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/agua");
Adafruit_MQTT_Publish pComb = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/combustivel");
Adafruit_MQTT_Publish pMant = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/mantimentos");
Adafruit_MQTT_Publish pBat  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bateria");
Adafruit_MQTT_Publish pTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish pPres = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressao");
Adafruit_MQTT_Publish pStat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/status");

void MQTT_connect() {
  if (mqtt.connected()) return;
  Serial.print("Conectando MQTT... ");
  int8_t ret; uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       mqtt.disconnect(); delay(5000); retries--;
       if (retries == 0) return;
  }
  Serial.println("MQTT Conectado!");
}

float lerUltrassonico(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duracao = pulseIn(echo, HIGH);
  float dist = duracao * 0.034 / 2;
  if (dist <= 0 || dist > 400) dist = 2.0; 
  float porcentagem = ((400.0 - dist) / (400.0 - 2.0)) * 100.0;
  return constrain(porcentagem, 0.0, 100.0);
}

void controlarBuzzer(unsigned long tempoAtual) {
  if (!estadoAlarmeGlobal) { noTone(PIN_BUZZER); buzzerAtivo = false; return; }
  if (buzzerAtivo) {
    if (tempoAtual - transicaoBuzzer >= 1000) {
      noTone(PIN_BUZZER); buzzerAtivo = false; transicaoBuzzer = tempoAtual;
    } else { tone(PIN_BUZZER, 1500); }
  } else {
    if (tempoAtual - transicaoBuzzer >= 2000) {
      tone(PIN_BUZZER, 1500); buzzerAtivo = true; transicaoBuzzer = tempoAtual;
    } else { noTone(PIN_BUZZER); }
  }
}

void atualizarSensores(unsigned long tempoAtual) {
  if (tempoAtual - ultimaLeituraSensores < intervaloSensores) return;
  ultimaLeituraSensores = tempoAtual;

  float agua = lerUltrassonico(TRIG_AGUA, ECHO_AGUA);
  float comb = lerUltrassonico(TRIG_COMB, ECHO_COMB);
  float mant = lerUltrassonico(TRIG_MANT, ECHO_MANT);
  float bateria = constrain(map(analogRead(PIN_LDR), 0, 4095, 100, 0), 0.0, 100.0);
  
  float temp = bmp.readTemperature();
  float pres = bmp.readPressure() / 100.0; 
  if (pres < 300 || pres > 1200) pres = 1013.25; 

  bool erroDetectado = false;
  String alertas = "", statusMsg = "SISTEMA OK";

  if ((ultimaPres - pres) > 10.0)  { alertas += "[ALERTA]: QUEDA DE PRESSÃO!\n"; statusMsg = "RISCO: PRESSAO"; }
  if ((ultimoComb - comb) > 15.0)  { alertas += "[ALERTA]: VAZAMENTO COMB.!\n"; statusMsg = "RISCO: COMB."; }
  if ((ultimaAgua - agua) > 15.0)  { alertas += "[ALERTA]: CONSUMO ALTO AGUA!\n"; statusMsg = "RISCO: AGUA"; }

  static float mAgua = -100, mComb = -100, mMant = -100, mBat = -100, mTemp = -100, mPres = -100;
  static String mStat = "";

  // Verificação rigorosa das faixas críticas
  if (agua < CRIT_AGUA_MIN || comb < CRIT_COMB_MIN || mant < CRIT_MANT_MIN || 
      bateria < CRIT_BATERIA_MIN || pres < CRIT_PRES_MIN || pres > CRIT_PRES_MAX || 
      temp < CRIT_TEMP_MIN || temp > CRIT_TEMP_MAX) {
    erroDetectado = true; 
    statusMsg = "CRITICO"; // Texto curto e direto para o bloco do painel receber sem erros
  }
  estadoAlarmeGlobal = erroDetectado;

  Serial.println("\n==================================================");
  Serial.print("  [AGUA]: "); Serial.print(agua, 1); Serial.print(" % | [COMB]: "); Serial.print(comb, 1); Serial.println(" %");
  Serial.print("  [MANT]: "); Serial.print(mant, 1); Serial.print(" % | [BAT]: "); Serial.print(bateria, 1); Serial.println(" %");
  Serial.print("  [TEMP]: "); Serial.print(temp, 1); Serial.print(" C | [PRES]: "); Serial.print(pres, 1); Serial.println(" hPa");
  Serial.print("  STATUS: "); Serial.println(statusMsg);
  if (alertas != "") Serial.print(alertas);
  Serial.println("==================================================");

  bool tempoMinimoRespeitado = (tempoAtual - ultimoEnvioMQTT >= 10000);
  bool forcarEnvioPorTempo = (tempoAtual - ultimoEnvioMQTT >= 20000);

  if (tempoMinimoRespeitado) {
    // Se o status mudou (Ex: de OK para CRITICO), o "statusMsg != mStat" vai disparar na hora!
    if (forcarEnvioPorTempo || statusMsg != mStat ||
        abs(agua - mAgua) >= 10.0 || abs(comb - mComb) >= 10.0 || 
        abs(mant - mMant) >= 10.0 || abs(bateria - mBat) >= 10.0 || 
        abs(temp - mTemp) >= 0.2 || abs(pres - mPres) >= 1.0) {           

      ultimoEnvioMQTT = tempoAtual;
      mAgua = agua; mComb = comb; mMant = mant; mBat = bateria; mTemp = temp; mPres = pres; mStat = statusMsg;

      // CORREÇÃO: Enviamos o status primeiro e diretamente como String estável
      pStat.publish(statusMsg.c_str());
      
      // Envia os demais feeds numéricos em sequência
      pAgua.publish(agua); pComb.publish(comb); pMant.publish(mant);
      pBat.publish(bateria); pTemp.publish(temp); pPres.publish(pres);
      
      Serial.println(">>> Painel e Status atualizados no Adafruit IO! <<<");
    }
  }
  ultimaPres = pres; ultimoComb = comb; ultimaAgua = agua;
}

void setup() {
  Serial.begin(115200); Wire.begin(21, 22); 
  if (!bmp.begin()) Serial.println("Erro BMP180!");
  pinMode(PIN_BUZZER, OUTPUT); digitalWrite(PIN_BUZZER, LOW);
  pinMode(TRIG_AGUA, OUTPUT); pinMode(ECHO_AGUA, INPUT);
  pinMode(TRIG_COMB, OUTPUT); pinMode(ECHO_COMB, INPUT);
  pinMode(TRIG_MANT, OUTPUT); pinMode(ECHO_MANT, INPUT);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  MQTT_connect();
  unsigned long tempoAtual = millis();
  atualizarSensores(tempoAtual);
  controlarBuzzer(tempoAtual);
}
