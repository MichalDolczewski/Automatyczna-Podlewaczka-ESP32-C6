#include <ESP32Servo.h>
#include "esp_sleep.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include <Preferences.h>
#include <IRremote.hpp>
#include <Adafruit_ADS1X15.h>


Preferences preferences;
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);


int suchy = 13500;
int mokry = 9500;
int wartosc = 1;
int zapisanaWartosc = 1;
int poprzedniStanCzujnika = LOW;
int licznikRuchu = 0;
int licznikCzujnikow = 0;

const int czujnik = D7;
const int IR_PIN = D0;
const int POMPA = D8;

const uint8_t KOD_POWER = 0x45;
const uint8_t KOD_MODE = 0x46;
const uint8_t KOD_PLUS = 0x09;
const uint8_t KOD_MINUS = 0x15;
const uint8_t KOD_OK = 0x44;

unsigned long startprzerwyruchu = 0;
unsigned long startpodlewania = 0;
unsigned long ostatnieWykrycie = 0;

bool przerwaRuchu = false;
bool powrot = false;
bool pierwszeWykrycie = true;

const unsigned long blokadaCzujnika = 1000;

Adafruit_ADS1115 ads;

enum class Stany {
  RUCH,
  WILGOTNOSC,
  PODLEWANIE,
  SLEEP,
  PAUZA,
  USTAWIENIA,
};
Stany stan_obecny = Stany::PAUZA;


void ruch() {

  if (przerwaRuchu) {
    servo.write(90);
  } else if (!powrot) {
    servo.write(84);
  } else {
    servo.write(96);
  }

  int stanCzujnika = digitalRead(czujnik);

  if (stanCzujnika == HIGH && poprzedniStanCzujnika == LOW) {

    if (pierwszeWykrycie || millis() - ostatnieWykrycie >= blokadaCzujnika) {
      ostatnieWykrycie = millis();
      pierwszeWykrycie = false;

      licznikRuchu++;

      servo.write(90);
      poprzedniStanCzujnika = stanCzujnika;
      stan_obecny = Stany::WILGOTNOSC;
      return;
    }
  }
  poprzedniStanCzujnika = stanCzujnika;
  if (licznikRuchu == 4 && !przerwaRuchu && !powrot) {
    servo.write(90);
    startprzerwyruchu = millis();
    przerwaRuchu = true;
  }

  if (przerwaRuchu && millis() - startprzerwyruchu >= 3000) {
    powrot = true;
    przerwaRuchu = false;
  }

  if (licznikRuchu >= 7) {
    servo.write(90);

    licznikRuchu = 0;
    powrot = false;
    przerwaRuchu = false;
    licznikCzujnikow = 0;

    poprzedniStanCzujnika = LOW;
    stan_obecny = Stany::SLEEP;
  }
}


void czujnik_wilgotnosci() {
  if (powrot) {
    stan_obecny = Stany::RUCH;
    return;
  }

  bool trzeba_podlac = PomiarWilgotnosci();

  if (licznikCzujnikow < 3) {
    licznikCzujnikow++;
  }

  if (trzeba_podlac) {
    startpodlewania = millis();
    stan_obecny = Stany::PODLEWANIE;
  } else {
    stan_obecny = Stany::RUCH;
  }
}


void podlewanie() {
  if (millis() - startpodlewania < 3000) {
    digitalWrite(POMPA, HIGH);
  } else {
    digitalWrite(POMPA, LOW);
  }

  if (millis() - startpodlewania >= 5000) {
    stan_obecny = Stany::RUCH;
  }
}


void pauza() {
  lcd.setCursor(0, 0);
  lcd.print("Podlewaczka");
  lcd.setCursor(0, 1);
  lcd.print("Roslin");
  int pilot = pobierzPilota();
  if (pilot == 2) {
    lcd.clear();
    stan_obecny = Stany::USTAWIENIA;
  } else if (pilot == 1) {
    stan_obecny = Stany::RUCH;
  }
}


void ustawienia() {
  int pilot = pobierzPilota();

  lcd.setCursor(0, 0);
  lcd.print("Aktualna wart:");
  lcd.setCursor(0, 1);
  lcd.print(wartosc);
  lcd.print("    ");
  if (pilot == 3 && wartosc < 360) {
    wartosc++;
  } else if (pilot == 4 && wartosc > 1) {
    wartosc--;
  } else if (pilot == 5) {
    zapisanaWartosc = wartosc;
    preferences.putInt("wartosc", zapisanaWartosc);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Zapisano");
    lcd.setCursor(0, 1);
    lcd.print(wartosc);

    delay(1000);
    lcd.clear();
    stan_obecny = Stany::PAUZA;
  }
}


void sleep() {

  servo.detach();

  preferences.end();

  esp_sleep_enable_timer_wakeup((uint64_t)wartosc * 10 * 1000000ULL);
  esp_sleep_enable_ext1_wakeup(1ULL << IR_PIN, ESP_EXT1_WAKEUP_ANY_LOW);

  esp_deep_sleep_start();
}


int pobierzPilota() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      IrReceiver.resume();
      return 0;
    }

    uint8_t kod = IrReceiver.decodedIRData.command;

    IrReceiver.resume();

    if (kod == KOD_POWER) {

      return 1;
    }

    if (kod == KOD_MODE) return 2;
    if (kod == KOD_PLUS) return 3;
    if (kod == KOD_MINUS) return 4;
    if (kod == KOD_OK) return 5;
  }

  return 0;
}


bool PomiarWilgotnosci() {
  int16_t odczytADC = ads.readADC_SingleEnded(licznikCzujnikow);
  int wilgotnosc = przelicz(odczytADC);

  return wilgotnosc < 40;
}


int przelicz(int pomiar) {
  int przeliczony = map(pomiar, suchy, mokry, 0, 100);
  przeliczony = constrain(przeliczony, 0, 100);
  return przeliczony;
}


void setup() {
  esp_sleep_wakeup_cause_t powod = esp_sleep_get_wakeup_cause();
  delay(1000);

  pinMode(czujnik, INPUT);  // czujnik optyczny

  pinMode(POMPA, OUTPUT);
  digitalWrite(POMPA, LOW);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  Wire.begin();

  if (!ads.begin()) {
    while (true)
      ;
  }
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Podlewaczka");
  lcd.setCursor(0, 1);
  lcd.print("Roslin");
  preferences.begin("ust", false);

  zapisanaWartosc = constrain(preferences.getInt("wartosc", 1), 1, 360);
  wartosc = zapisanaWartosc;

  servo.setPeriodHertz(50);
  servo.attach(D10, 500, 2500);
  if (powod == ESP_SLEEP_WAKEUP_TIMER) {
    stan_obecny = Stany::RUCH;
  } else if (powod == ESP_SLEEP_WAKEUP_EXT1) {
    stan_obecny = Stany::PAUZA;
  }
}


void loop() {
  switch (stan_obecny) {
    case Stany::RUCH:
      ruch();
      break;
    case Stany::WILGOTNOSC:
      czujnik_wilgotnosci();
      break;
    case Stany::PODLEWANIE:
      podlewanie();
      break;
    case Stany::PAUZA:
      pauza();
      break;
    case Stany::SLEEP:
      sleep();
      break;
    case Stany::USTAWIENIA:
      ustawienia();
      break;
  }
}
