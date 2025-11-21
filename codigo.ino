#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ENDEREÇO DO LCD (mais comum é 0x27 — se não funcionar tente 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Pessoa {
  String uid;
  String nome;
  String cargo;
  String senioridade;
};

// Banco de dados
Pessoa lista[] = {
  {"123456", "Giovanna Sayama", "Eng. Software", "Senior"},
  {"987654", "Lucas Almeida", "Dev Front-End", "Junior"},
  {"445566", "Maria Santos", "Tecnica IoT", "Pleno"}
};

int total = sizeof(lista) / sizeof(lista[0]);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Sistema RFID");
  lcd.setCursor(0, 1);
  lcd.print("UID por Serial");

  delay(2000);
  lcd.clear();

  Serial.println("Digite a UID (ex: 123456):");
}

void mostrarLCD(String linha1, String linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

void verificaUID(String uidDigitada) {
  uidDigitada.trim();

  for (int i = 0; i < total; i++) {
    if (uidDigitada == lista[i].uid) {
      
      // Serial
      Serial.println("\n=== CARTÃO ENCONTRADO ===");
      Serial.println("Nome: " + lista[i].nome);
      Serial.println("Cargo: " + lista[i].cargo);
      Serial.println("Senioridade: " + lista[i].senioridade);
      Serial.println("==========================\n");

      // LCD – mostra duas informações por vez
      mostrarLCD(lista[i].nome, lista[i].cargo);
      delay(3500);
      mostrarLCD("Senioridade:", lista[i].senioridade);
      delay(3500);
      mostrarLCD("Digite a UID:", "");
      
      return;
    }
  }

  // UID Não encontrada
  Serial.println("\nUID nao encontrada!\n");
  mostrarLCD("UID NAO ENCONTRADA", "");
  delay(2000);
  mostrarLCD("Digite outra UID", "");
}

void loop() {
  if (Serial.available()) {
    String uid = Serial.readStringUntil('\n');
    verificaUID(uid);
  }
}
