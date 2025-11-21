integrantes: Giovanna Lins Sayama - RM565901 e Ana Luiza De Franco e Rinaldi - 564061

📑 Sumário

- [🎯 Objetivo do Projeto](#objetivo-do-projeto)
  
- [📦 Componentes Utilizados](#componentes-utilizados)
  
- [🔌 Ligações (ESP32 → LCD I2C)](#ligações-esp32--lcd-i2c)
  
- [🧠 Como o Sistema Funciona](#como-o-sistema-funciona)
  
- [▶️ Como Rodar no Wokwi](#como-rodar-no-wokwi)
  
- [💻 Código Completo (ESP32 + MQTT + LCD)](#código-completo-esp32--mqtt--lcd)
  
- [✅ Conclusão](#conclusão)

Este projeto demonstra um sistema de controle de acesso digital utilizando ESP32, LCD I2C, MQTT e FIWARE.

O sistema permite:

Receber uma UID via Serial ou interface simulada no Wokwi.

Verificar se a UID existe na tabela interna de usuários.

Exibir no LCD Nome, Cargo e Nível de Senioridade.

Publicar os dados via MQTT para o IoT Agent FIWARE, atualizando o Orion Context Broker.

📦 Componentes Utilizados
Componente	Função
ESP32 Devkit V1	Controlador principal
LCD 16×2 I2C	Exibição das informações do usuário
Biblioteca LiquidCrystal_I2C	Controle do LCD
Serial Monitor	Entrada da UID
MQTT Broker / IoT Agent	Comunicação com FIWARE Orion
🔌 Ligações (ESP32 → LCD I2C)
LCD I2C	ESP32
VCC	5V
GND	GND
SDA	GPIO 21
SCL	GPIO 22
🧠 Como o Sistema Funciona

Ao ligar, o ESP32 inicializa o LCD mostrando:

Sistema de Acesso
Aguardando UID...


O usuário digita a UID no Serial Monitor ou Wokwi GUI.

O ESP32 verifica na tabela interna:
```
struct Usuario {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};
```

Exemplo de dados mock:

UID	Nome	Cargo	Senioridade
12345	Giovanna Sayama	Front-end Developer	Junior
67890	Carlos Mendes	Engenheiro Software	Pleno
11111	Mariana Silva	Tech Lead	Senior

Se a UID existir:

LCD mostra:

Giovanna Sayama
Front-end Developer - Junior


Publica JSON via MQTT para o IoT Agent FIWARE:
```
{
  "id": "User.12345",
  "type": "UserAccess",
  "uid": "12345",
  "nome": "Giovanna Sayama",
  "cargo": "Front-end Developer",
  "senioridade": "Junior"
}
```

Se a UID não existir:

LCD mostra:

UID inválida
Usuário não achado

▶️ Como Rodar no Wokwi

Acesse https://wokwi.com

Crie um projeto ESP32

Adicione o ESP32 Devkit e LCD 16x2 I2C

Configure os pinos conforme a tabela acima

Cole o código do projeto no main.cpp

Clique Start

Abra o Serial Monitor e digite qualquer UID da tabela mock

Observe o LCD e os logs Serial

O código publica automaticamente no IoT Agent MQTT, registrando no Orion Context Broker

💻 Código Completo (ESP32 + MQTT + LCD)
```
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WIFI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT / IoT Agent
const char* MQTT_BROKER = "mqtt://broker.hivemq.com"; // Exemplo público
const int MQTT_PORT = 1883;
const char* TOPIC = "iot/json";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Estrutura de usuário
struct Usuario {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};

Usuario usuarios[] = {
  {"12345", "Giovanna Sayama", "Front-end Developer", "Junior"},
  {"67890", "Carlos Mendes", "Engenheiro de Software", "Pleno"},
  {"11111", "Mariana Silva", "Tech Lead", "Senior"}
};

int total = sizeof(usuarios) / sizeof(usuarios[0]);

Usuario* buscar(String uid){
  for(int i=0;i<total;i++){
    if(usuarios[i].uid==uid) return &usuarios[i];
  }
  return NULL;
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Sistema de Acesso");
  lcd.setCursor(0,1);
  lcd.print("Aguardando UID...");

  // WiFi
  WiFi.begin(ssid,password);
  Serial.print("Conectando ao WiFi...");
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println("\nWiFi conectado!");

  // MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if(Serial.available()){
    String uid = Serial.readStringUntil('\n');
    uid.trim();

    Usuario* u = buscar(uid);
    if(u==NULL){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("UID invalida");
      lcd.setCursor(0,1);
      lcd.print("Usuario nao achado");
      Serial.println("UID nao encontrada.");
      delay(2500);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sistema de Acesso");
      lcd.setCursor(0,1);
      lcd.print("Aguardando UID...");
      return;
    }

    // Mostrar no LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(u->nome);
    lcd.setCursor(0,1);
    lcd.print(u->cargo+" - "+u->senioridade);

    // Preparar JSON MQTT
    StaticJsonDocument<300> doc;
    doc["id"]="User."+u->uid;
    doc["type"]="UserAccess";
    doc["uid"]=u->uid;
    doc["nome"]=u->nome;
    doc["cargo"]=u->cargo;
    doc["senioridade"]=u->senioridade;

    String json;
    serializeJson(doc,json);

    // Enviar via MQTT
    if(!mqttClient.connected()){mqttClient.connect("ESP32_Client");}
    mqttClient.publish(TOPIC,json.c_str());

    Serial.println("\nENVIANDO PARA O FIWARE:");
    Serial.println(json);

    delay(2500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sistema de Acesso");
    lcd.setCursor(0,1);
    lcd.print("Aguardando UID...");
  }

  mqttClient.loop();
}
```
✅ Conclusão

Demonstra domínio completo de ESP32, MQTT e FIWARE

Simulação funcional no Wokwi, sem hardware físico

Integração completa com Orion Context Broker via IoT Agent

Permite criar, visualizar e atualizar entidades de usuários

Estrutura clara, códigos comentados e simulação realista
