#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- LCD I2C ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ------------------- CONFIGURAÇÕES -------------------
const char* default_SSID = "Optilink casa praia";
const char* default_PASSWORD = "sayama00";
const char* default_BROKER_MQTT = "20.119.99.133";
const int   default_BROKER_PORT = 1883;

const char* default_TOPICO_SUBSCRIBE = "/TEF/lamp001/cmd";
const char* default_TOPICO_PUBLISH_1 = "/TEF/lamp001/attrs";
const char* default_TOPICO_PUBLISH_2 = "/TEF/lamp001/attrs/l";

const char* default_ID_MQTT = "fiware_001";
const int default_D4 = 2;

const int DHTPIN = 4;
#define DHTTYPE DHT11

const char* topicPrefix = "lamp001";

// ------------------- VARIÁVEIS EDITÁVEIS -------------------
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4;

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

DHT dht(DHTPIN, DHTTYPE);

// ------------------- ESTRUTURA DE USUÁRIOS -------------------

struct Pessoa {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};

Pessoa lista[] = {
  {"123456", "Giovanna", "Eng. Software", "Sênior"},
  {"987654", "Lucas", "Front-End", "Júnior"},
  {"445566", "Maria", "Técnica IoT", "Pleno"}
};

int total = sizeof(lista) / sizeof(lista[0]);

// ------------------- FUNÇÕES -------------------

void mostrarLCD(String l1, String l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
}

void verificarUID(String uid) {
  uid.trim();

  for (int i = 0; i < total; i++) {
    if (uid == lista[i].uid) {

      Serial.println("\n=== USUARIO ENCONTRADO ===");
      Serial.println("Nome: " + lista[i].nome);
      Serial.println("Cargo: " + lista[i].cargo);
      Serial.println("Senioridade: " + lista[i].senioridade);
      Serial.println("==========================");

      mostrarLCD(lista[i].nome, lista[i].cargo);
      delay(3000);
      mostrarLCD("Senioridade:", lista[i].senioridade);
      delay(3000);

      String json = "{";
      json += "\"uid\":\"" + lista[i].uid + "\",";
      json += "\"nome\":\"" + lista[i].nome + "\",";
      json += "\"cargo\":\"" + lista[i].cargo + "\",";
      json += "\"senioridade\":\"" + lista[i].senioridade + "\"";
      json += "}";

      MQTT.publish(TOPICO_PUBLISH_1, json.c_str());

      Serial.println("Dados enviados ao FIWARE:");
      Serial.println(json);

      mostrarLCD("Digite UID:", "");
      return;
    }
  }

  Serial.println("UID NAO ENCONTRADA");
  mostrarLCD("UID NAO", "ENCONTRADA");
  delay(2000);
  mostrarLCD("Digite UID:", "");
}

// ------------------- WI-FI -------------------

void reconectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nWiFi conectado!");
  Serial.println(WiFi.localIP());
  digitalWrite(D4, LOW);
}

// ------------------- MQTT -------------------

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("Comando MQTT recebido: ");
  Serial.println(msg);

  if (msg == "lamp001@on|") {
    digitalWrite(D4, HIGH);
    EstadoSaida = '1';
  }

  if (msg == "lamp001@off|") {
    digitalWrite(D4, LOW);
    EstadoSaida = '0';
  }
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Reconectando ao broker...");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falhou. Tentando novamente...");
      delay(2000);
    }
  }
}

void EnviaEstadoOutputMQTT() {
  if (EstadoSaida == '1') MQTT.publish(TOPICO_PUBLISH_1, "s|on");
  if (EstadoSaida == '0') MQTT.publish(TOPICO_PUBLISH_1, "s|off");
  delay(1000);
}

// ------------------- LEITURA TEMPERATURA -------------------

void handleDHT() {
  float t = dht.readTemperature();
  if (isnan(t)) return;

  char buffer[10];
  itoa((int)t, buffer, 10);
  MQTT.publish(TOPICO_PUBLISH_2, buffer);

  Serial.print("Temperatura enviada: ");
  Serial.println(buffer);
}

// ------------------- SETUP -------------------

void setup() {
  pinMode(D4, OUTPUT);
  Serial.begin(115200);

  dht.begin();
  lcd.init();
  lcd.backlight();

  mostrarLCD("Inicializando", "...");

  reconectWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);

  mostrarLCD("Digite UID:", "");
}

// ------------------- LOOP -------------------

void loop() {
  if (Serial.available()) {
    String uid = Serial.readStringUntil('\n');
    verificarUID(uid);
  }

  reconectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
  MQTT.loop();

  handleDHT();
  EnviaEstadoOutputMQTT();
}
