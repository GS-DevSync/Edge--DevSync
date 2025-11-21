#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- CONFIGURAÇÃO LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- CONFIGURAÇÃO WI-FI E MQTT ----------------
const char* SSID = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* BROKER_MQTT = "20.119.99.133";
const int BROKER_PORT = 1883;
const char* TOPICO_SUBSCRIBE = "/TEF/lamp001/cmd";
const char* TOPICO_PUBLISH = "/TEF/lamp001/attrs";
const char* ID_MQTT = "fiware_001";
const int D4 = 2;

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

// ---------------- ESTRUTURA DE USUÁRIOS ----------------
struct Usuario {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};

Usuario usuarios[] = {
  {"123456", "Giovanna", "Front-end Dev", "Junior"},
  {"67890", "Carlos", "Engenheiro Software", "Pleno"},
  {"11111", "Mariana", "Tech Lead", "Senior"}
};
int totalUsuarios = sizeof(usuarios)/sizeof(usuarios[0]);

Usuario* buscarUsuario(String uid){
  for(int i=0; i<totalUsuarios; i++){
    if(usuarios[i].uid == uid) return &usuarios[i];
  }
  return NULL;
}

// ---------------- FUNÇÕES ----------------
void mostrarLCD(String l1, String l2){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(l1);
  lcd.setCursor(0,1);
  lcd.print(l2);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length){
  String msg;
  for(int i=0;i<length;i++) msg += (char)payload[i];
  Serial.print("Mensagem recebida MQTT: "); Serial.println(msg);

  if(msg == "lamp001@on|"){
    digitalWrite(D4,HIGH);
    EstadoSaida = '1';
  }
  if(msg == "lamp001@off|"){
    digitalWrite(D4,LOW);
    EstadoSaida = '0';
  }
}

void reconnectMQTT(){
  while(!MQTT.connected()){
    Serial.print("Conectando ao Broker MQTT...");
    if(MQTT.connect(ID_MQTT)){
      Serial.println("Conectado!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha. Tentando novamente...");
      delay(2000);
    }
  }
}

void reconectWiFi(){
  if(WiFi.status()==WL_CONNECTED) return;
  WiFi.begin(SSID,PASSWORD);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nWiFi conectado!");
  Serial.println(WiFi.localIP());
  digitalWrite(D4,LOW); // LED inicia desligado
}

void enviarEstadoMQTT(){
  if(EstadoSaida=='1') MQTT.publish(TOPICO_PUBLISH,"s|on");
  if(EstadoSaida=='0') MQTT.publish(TOPICO_PUBLISH,"s|off");
}

// ---------------- SETUP ----------------
void setup(){
  Serial.begin(115200);
  pinMode(D4,OUTPUT);
  lcd.init();
  lcd.backlight();
  mostrarLCD("Inicializando","...");

  reconectWiFi();
  MQTT.setServer(BROKER_MQTT,BROKER_PORT);
  MQTT.setCallback(mqtt_callback);

  mostrarLCD("Digite UID","");
}

// ---------------- LOOP ----------------
void loop(){
  // Leitura Serial
  if(Serial.available()){
    String uid = Serial.readStringUntil('\n');
    uid.trim();
    Usuario* u = buscarUsuario(uid);

    if(u){
      Serial.println("Usuario encontrado:");
      Serial.println("Nome: "+u->nome);
      Serial.println("Cargo: "+u->cargo);
      Serial.println("Senioridade: "+u->senioridade);

      mostrarLCD(u->nome,u->cargo+"-"+u->senioridade);
      // Simula envio MQTT
      String json = "{\"uid\":\""+u->uid+"\",\"nome\":\""+u->nome+"\",\"cargo\":\""+u->cargo+"\",\"senioridade\":\""+u->senioridade+"\"}";
      MQTT.publish(TOPICO_PUBLISH,json.c_str());
      delay(3000);
    } else {
      Serial.println("UID nao encontrada");
      mostrarLCD("UID nao","encontrada");
      delay(2000);
    }

    mostrarLCD("Digite UID","");
  }

  // Mantem conexão
  reconectWiFi();
  if(!MQTT.connected()) reconnectMQTT();
  MQTT.loop();
  enviarEstadoMQTT();
}
