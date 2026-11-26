
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

Po uruchomieniu urządzenie rozpoczyna skanowanie pozycji doniczek. Ze względu na to, że użyłem serwomechanizmu 360° z ciągłą pracą, musiałem wykorzystać czujnik optyczny, który wykrywając szczelinę w platformie wskazuje miejsce w którym serwomechanizm ma się zatrzymać.

Po wykryciu mechanizm zatrzymuje się i wykonywany jest pomiar wilgotności gleby.

Odczyt odbywa się przez przetwornik ADS1115, który komunikuje się z ESP32-C6 za pomocą magistrali I2C. Otrzymana wartość jest przeliczana na procent wilgotności na podstawie wcześniej wykonanej kalibracji.

Jeśli wilgotność jest niższa od ustawionego progu, uruchamiana jest pompka wodna. W przeciwnym razie urządzenie pomija podlewanie i przechodzi do kolejnej doniczki.

Po sprawdzeniu wszystkich czterech doniczek system przechodzi w tryb Deep Sleep, aby ograniczyć zużycie energii przed następnym cyklem.

### Sterowanie urządzeniem

Do obsługi wykorzystałem pilota na podczerwień oraz wyświetlacz LCD 16×2.

Pilot umożliwia uruchamianie i zatrzymywanie pracy, przechodzenie do ustawień oraz zmianę czasu uśpienia.

Ustawienia zapisywane są w pamięci nieulotnej ESP32-C6 za pomocą biblioteki `Preferences`, dzięki czemu nie trzeba konfigurować ich od nowa po restarcie urządzenia.

## 4. Oprogramowanie

Program napisałem w C++ z wykorzystaniem Arduino Framework.

Sterowanie urządzeniem oparłem na automacie stanów. Dzięki temu poszczególne etapy pracy, takie jak obracanie mechanizmu, pomiar wilgotności czy uruchamianie pompki, są obsługiwane oddzielnie.

W programie wykorzystałem następujące stany:

| Stan | Działanie |
|---|---|
| `RUCH` | Obracanie mechanizmu|
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

Kod znajduje się w folderze [`firmware`](/firmware).

## 5. Dlaczego wykorzystałem serwomechanizm?

W projekcie zdecydowałem się na zastosowanie serwomechanizmu do obracania mechanizmu rozprowadzającego wodę.

Był to świadomy wybór. Od początku zdawałem sobie sprawę, że istnieją inne rozwiązania, które mogłyby być prostsze lub bardziej niezawodne, np. wykorzystanie elektrozaworów.

Chciałem jednak przy okazji tego projektu nauczyć się sterowania serwomechanizmem i sprawdzić, jak zachowuje się on w rzeczywistym układzie, a nie tylko podczas prostych testów.

W trakcie budowy pojawiły się problemy, których początkowo nie brałem w pełni pod uwagę. Dotyczyły przede wszystkim obciążenia mechanizmu, ograniczonego zakresu ruchu i przewodów, które przeszkadzały podczas obracania platformy.

Musiałem więc odpowiednio dopasować konstrukcję oraz sposób sterowania.


## 6. Elektronika i zasilanie

Schemat elektryczny przygotowałem w programie KiCad.

Układ wykorzystuje osobne poziomy napięć dla poszczególnych elementów. Pompka i serwomechanizm korzystają z zasilania 6 V, natomiast zasilanie mikrokontrolera jest dopasowywane za pomocą przetwornicy DC-DC.

Pompka sterowana jest tranzystorem MOSFET IRLZ44N. Zastosowałem również diodę zabezpieczającą przed przepięciami powstającymi podczas wyłączania silnika.

W układzie znajdują się także kondensatory filtrujące, których zastosowanie było istotne ze względu na zakłócenia pojawiające się podczas pracy pompki i serwomechanizmu.

### Schemat elektryczny

- [Plik KiCad](hardware/Automatyczna_podlewaczka_roslin_ESP32_C6.kicad_sch)
- [Schemat w PDF](hardware/Automatyczna_podlewaczka roslin_ESP32_C6.kicad.pdf)

## 7. Konstrukcja mechaniczna

Elementy mechaniczne zaprojektowałem z myślą o wykonaniu ich na drukarce 3D.

Głównym elementem jest obrotowa platforma, na której umieszczony został mechanizm kierowania strumienia wody.

Podczas projektowania musiałem uwzględnić miejsce na przewody, sposób mocowania serwomechanizmu i rurki doprowadzającej wodę oraz obciążenie całej konstrukcji.

Modele 3D i pliki do druku są umieszczone w folderze `mechanical`.


## 8. O projekcie

Podlewaczka jest moim indywidualnym projektem.

Zależało mi na zbudowaniu czegoś, co faktycznie może mieć zastosowanie w domu, a jednocześnie pozwoli mi poeksperymentować z elektroniką i programowaniem mikrokontrolerów.

---
## 9 Prezentacja działania

Poniższy film przedstawia działanie prototypu podczas automatycznego cyklu podlewania roślin.

🎥 **[Prezentacja działania](https://www.youtube.com/watch?v=-FItSWhTrL0)**

**Autor:** Michał Dolczewski  
**Rok:** 2026
