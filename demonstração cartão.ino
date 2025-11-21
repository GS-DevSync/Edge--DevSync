#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

String IOTA_URL = "http://20.119.99.133:4041/iot/json";

struct Usuario {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};

Usuario usuarios[] = {
  {"123456", "Giovanna Sayama", "Front-end Developer", "Junior"},
  {"67890", "Carlos Mendes", "Engenheiro de Software", "Pleno"},
  {"11111", "Mariana Silva", "Tech Lead", "Senior"}
};

int total = sizeof(usuarios) / sizeof(usuarios[0]);

Usuario* buscar(String uid) {
  for (int i = 0; i < total; i++) {
    if (usuarios[i].uid == uid) return &usuarios[i];
  }
  return NULL;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.println("Digite o UID:");
}

void loop() {
  if (Serial.available()) {

    String uid = Serial.readStringUntil('\n');
    uid.trim();

    Usuario* u = buscar(uid);

    if (u == NULL) {
      Serial.println("UID não encontrada.");
      return;
    }

    StaticJsonDocument<300> doc;

    doc["id"] = "User." + u->uid;
    doc["type"] = "UserAccess";

    doc["uid"] = u->uid;
    doc["nome"] = u->nome;
    doc["cargo"] = u->cargo;
    doc["senioridade"] = u->senioridade;

    String json;
    serializeJson(doc, json);

    Serial.println("\nENVIANDO PARA O IOT AGENT:");
    Serial.println(json);

    HTTPClient http;
    http.begin(IOTA_URL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Fiware-Service", "helmo");
    http.addHeader("Fiware-ServicePath", "/");

    int status = http.POST(json);

    Serial.print("\nStatus: ");
    Serial.println(status);

    Serial.println("Resposta:");
    Serial.println(http.getString());

    http.end();

    Serial.println("\nDigite outro UID:");
  }
}
