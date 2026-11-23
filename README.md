
# Automatyczna podlewaczka roślin – ESP32-C6

Projekt automatycznej podlewaczki, którą stworzyłem z myślą o nawadnianiu kilku roślin podczas dłuższej nieobecności w domu.

Urządzenie wykorzystuje mikrokontroler ESP32-C6, czujniki wilgotności gleby, pompkę wodną oraz obrotowy mechanizm sterowany serwomechanizmem. Na podstawie pomiarów wilgotności decyduje, czy dana roślina wymaga podlania.

Projekt powstał również po to, żeby poćwiczyć programowanie mikrokontrolerów, pracę z różnymi czujnikami i projektowanie elektroniki. Poszczególne rozwiązania testowałem i modyfikowałem w trakcie budowy urządzenia.

## 1. Założenia projektu

Chciałem zbudować urządzenie, które będzie w stanie samodzielnie podlewać kilka doniczek, korzystając z jednej pompki wodnej.

Główne założenia:

- Pomiar wilgotności gleby w czterech doniczkach.
- Podlewanie tylko wtedy, gdy wilgotność spadnie poniżej określonego progu.
- Rozprowadzanie wody za pomocą obrotowego mechanizmu.
- Możliwość zmiany ustawień przy użyciu pilota IR.
- Wyświetlanie informacji na LCD.
- Ograniczenie poboru energii poprzez wykorzystanie trybu Deep Sleep.

Zależało mi też na tym, żeby urządzenie mogło działać bez konieczności stałego podłączenia do komputera.

## 2. Wykorzystane elementy

| Element | Zastosowanie |
|---|---|
| Seeed XIAO ESP32-C6 | Sterowanie całym urządzeniem |
| ADS1115 | Odczyt czterech czujników wilgotności |
| Czujniki wilgotności gleby | Pomiar wilgotności w doniczkach |
| Serwomechanizm | Obracanie mechanizmu rozprowadzania wody |
| Pompka wodna 6 V | Podlewanie roślin |
| MOSFET IRLZ44N | Sterowanie pompką |
| Czujnik optyczny | Wykrywanie pozycji doniczek |
| LCD 16×2 I2C | Wyświetlanie informacji |
| Odbiornik IR | Sterowanie za pomocą pilota |
| Przetwornica DC-DC | Dopasowanie napięcia zasilania |

Do tego wykorzystałem podstawowe elementy elektroniczne, takie jak rezystory, kondensatory i diody zabezpieczające.

## 3. Jak działa podlewaczka?

Po uruchomieniu urządzenie rozpoczyna skanowanie pozycji doniczek. Serwomechanizm obraca mechanizm rozprowadzania wody, a czujnik optyczny wykrywa kolejne pozycje.

Po wykryciu doniczki mechanizm zatrzymuje się i wykonywany jest pomiar wilgotności gleby.

Odczyt odbywa się przez przetwornik ADS1115, który komunikuje się z ESP32-C6 za pomocą magistrali I2C. Otrzymana wartość jest przeliczana na procent wilgotności na podstawie wcześniej wykonanej kalibracji.

Jeśli wilgotność jest niższa od ustawionego progu, uruchamiana jest pompka wodna. W przeciwnym razie urządzenie pomija podlewanie i przechodzi do kolejnej doniczki.

Po sprawdzeniu wszystkich czterech doniczek system przechodzi w tryb Deep Sleep, aby ograniczyć zużycie energii przed następnym cyklem.

### Sterowanie urządzeniem

Do obsługi wykorzystałem pilota na podczerwień oraz wyświetlacz LCD 16×2.

Pilot umożliwia uruchamianie i zatrzymywanie pracy, przechodzenie do ustawień oraz zmianę wybranych parametrów.

Ustawienia zapisywane są w pamięci nieulotnej ESP32-C6 za pomocą biblioteki `Preferences`, dzięki czemu nie trzeba konfigurować ich od nowa po restarcie urządzenia.

## 4. Oprogramowanie

Program napisałem w C++ z wykorzystaniem Arduino Framework.

Sterowanie urządzeniem oparłem na automacie stanów. Dzięki temu poszczególne etapy pracy, takie jak obracanie mechanizmu, pomiar wilgotności czy uruchamianie pompki, są obsługiwane oddzielnie.

W programie wykorzystałem następujące stany:

| Stan | Działanie |
|---|---|
| `RUCH` | Obracanie mechanizmu i wykrywanie doniczek |
| `WILGOTNOSC` | Odczyt wilgotności gleby |
| `PODLEWANIE` | Uruchomienie pompki |
| `SLEEP` | Uśpienie mikrokontrolera |
| `PAUZA` | Wstrzymanie pracy |
| `USTAWIENIA` | Zmiana parametrów urządzenia |

Wykorzystane biblioteki:

- `ESP32Servo` – sterowanie serwomechanizmem.
- `Adafruit_ADS1X15` – obsługa przetwornika ADS1115.
- `LiquidCrystal_I2C` – obsługa wyświetlacza LCD.
- `IRremote` – odbieranie sygnałów z pilota.
- `Preferences` – zapisywanie ustawień w pamięci nieulotnej.
- `esp_sleep` – obsługa trybu Deep Sleep.

Kod znajduje się w folderze [`firmware`](firmware/).

## 5. Dlaczego wykorzystałem serwomechanizm?

W projekcie zdecydowałem się na zastosowanie serwomechanizmu do obracania mechanizmu rozprowadzającego wodę.

Był to świadomy wybór. Od początku zdawałem sobie sprawę, że istnieją inne rozwiązania, które mogłyby być prostsze lub bardziej niezawodne, np. wykorzystanie elektrozaworów.

Chciałem jednak przy okazji tego projektu nauczyć się sterowania serwomechanizmem i sprawdzić, jak zachowuje się on w rzeczywistym układzie, a nie tylko podczas prostych testów.

W trakcie budowy pojawiły się problemy, których początkowo nie brałem w pełni pod uwagę. Dotyczyły przede wszystkim obciążenia mechanizmu, ograniczonego zakresu ruchu i przewodów, które przeszkadzały podczas obracania platformy.

Musiałem więc odpowiednio dopasować konstrukcję oraz sposób sterowania.

Nie uważam, że serwomechanizm jest najlepszym możliwym rozwiązaniem dla tego urządzenia, ale jego wykorzystanie pozwoliło mi sprawdzić w praktyce kilka zagadnień, które wcześniej znałem głównie z teorii.

## 6. Elektronika i zasilanie

Schemat elektryczny przygotowałem w programie KiCad.

Układ wykorzystuje osobne poziomy napięć dla poszczególnych elementów. Pompka i serwomechanizm korzystają z zasilania 6 V, natomiast zasilanie mikrokontrolera jest dopasowywane za pomocą przetwornicy DC-DC.

Pompka sterowana jest tranzystorem MOSFET IRLZ44N. Zastosowałem również diodę zabezpieczającą przed przepięciami powstającymi podczas wyłączania silnika.

W układzie znajdują się także kondensatory filtrujące, których zastosowanie było istotne ze względu na zakłócenia pojawiające się podczas pracy pompki i serwomechanizmu.

### Schemat elektryczny

- [Plik KiCad](hardware/podlewaczka_schemat.kicad_sch)
- [Schemat w PDF](hardware/podlewaczka_schemat.pdf)

## 7. Problemy napotkane podczas budowy

Podczas pracy nad podlewaczką nie wszystko działało od razu. Sporo czasu zajęło mi testowanie poszczególnych elementów i szukanie przyczyn nieprawidłowego działania.

### Zakłócenia od pompki

Podczas uruchamiania pompki pojawiały się zakłócenia wpływające na pracę innych elementów, między innymi LCD i serwomechanizmu.

Testowałem różne sposoby prowadzenia zasilania oraz zastosowanie kondensatorów filtrujących.

### Problemy z mechanizmem obrotowym

W trakcie obrotu platformy przewody potrafiły ograniczać jej ruch. Dodatkowo obciążenie mechanizmu wpływało na pracę serwomechanizmu.

Wymagało to modyfikacji konstrukcji oraz ograniczenia zakresu obrotu.

### Odczyty czujników wilgotności

Czujniki wymagały kalibracji, ponieważ same wartości odczytywane przez ADC nie informują bezpośrednio o procentowej wilgotności gleby.

Wykonałem pomiary dla suchego i mokrego podłoża, a następnie wykorzystałem te wartości do przeliczania wyników.

### Deep Sleep i wybudzanie

Kolejnym wyzwaniem była obsługa trybu Deep Sleep. Podczas testowania pojawiały się problemy związane z wybudzaniem urządzenia oraz komunikacją przez USB.

### Odbiornik podczerwieni

Zdarzało się również, że podczas pracy serwomechanizmu odbiornik IR rejestrował nieprawidłowe kody. Wymagało to sprawdzenia, jak zakłócenia i sposób obsługi pilota wpływają na działanie programu.

## 8. Konstrukcja mechaniczna

Elementy mechaniczne zaprojektowałem z myślą o wykonaniu ich na drukarce 3D.

Głównym elementem jest obrotowa platforma, na której umieszczony został mechanizm kierowania strumienia wody.

Podczas projektowania musiałem uwzględnić miejsce na przewody, sposób mocowania serwomechanizmu i rurki doprowadzającej wodę oraz obciążenie całej konstrukcji.

Modele 3D i pliki do druku będą umieszczone w folderze `mechanical`.


## 9. Struktura repozytorium

    Automatyczna-Podlewaczka-ESP32-C6/
    │
    ├── firmware/
    │   └── podlewaczka.ino
    │
    ├── hardware/
    │   ├── podlewaczka_schemat.kicad_sch
    │   └── podlewaczka_schemat.pdf
    │
    ├── mechanical/
    │
    ├── images/
    │
    └── README.md

## 11. O projekcie

Podlewaczka jest moim indywidualnym projektem, który stopniowo rozwijam i poprawiam.

Zależało mi na zbudowaniu czegoś, co faktycznie może mieć zastosowanie w domu, a jednocześnie pozwoli mi poeksperymentować z elektroniką i programowaniem mikrokontrolerów.

Nie wszystkie zastosowane rozwiązania są optymalne i część z nich z pewnością można wykonać lepiej. Projekt traktuję jednak również jako okazję do sprawdzania własnych pomysłów, popełniania błędów i wyciągania z nich wniosków.

---

**Autor:** Michał Dolczewski  
**Rok:** 2026
