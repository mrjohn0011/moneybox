#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

/**
* Количество разных номиналов монет
*/
const byte coin_count = 5;

/**
* Цифровые пины, к которым подключены провода по каждому номиналу
*/
const byte coin_pins[coin_count] = {4, 6, 7, 8, 5};

/**
* Номиналы, соответствующие пинам монеток
*/
const float coin_nominals[coin_count] = {0.5, 1, 2, 5, 10};

/**
* Цифровой пин кнопки статистики
*/
const byte STAT_BTN = 9;

/**
* Цифровой пин кнопки очистки
*/
const byte CLEAR_BTN = 2;

/**
* Начальная ячейка памяти, в которую записывать данные
*/
const int START_MEMORY_ADDRESS = 0;

/**
* Сколько монеток каждого номинала уже лежит в копилке
* Это значение опции по умолчанию. Оно используется только если нет доступа к EEPROM,
* то есть, для отладки.
*/
int dropped_coins[coin_count] = {0, 0, 0, 0, 0};

LiquidCrystal_I2C lcd(0x3f,20,4);

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < coin_count; i++){
    pinMode(coin_pins[i], INPUT);
  }

  pinMode(STAT_BTN, INPUT);
  pinMode(CLEAR_BTN, INPUT);

  lcd.init();
  lcd.backlight();

  loadCoins();
}

void log(String msg, float value){
  Serial.print(msg);
  Serial.println(value);
}

void writeInt(int index, int number){
  int address = START_MEMORY_ADDRESS + 2 * index;
  byte hi = highByte(number);
  byte lo = lowByte(number);
  EEPROM.write(address, hi);
  EEPROM.write(address + 1, lo);
}

int readInt(int index){
  int address = START_MEMORY_ADDRESS + 2 * index;
  byte hi = EEPROM.read(address);
  byte lo = EEPROM.read(address + 1);
  return hi * 256 + lo;
}

void writeCoin(int index, int amount){
  writeInt(index, amount);
  dropped_coins[index] = amount;
  log("write coin: ", amount);
}

int readCoin(int index){
  dropped_coins[index] = readInt(index);
  return dropped_coins[index];
}

void addCoin(int index){
  dropped_coins[index]++;
  writeCoin(index, dropped_coins[index]);
  lcd.clear();
  showTotal();
}

void loadCoins(){
  for(int i = 0; i < coin_count; i++){
    dropped_coins[i] = readCoin(i);
  }
  showTotal();
}

void showCoin(int index){
  log("Show coin: ", index);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(coin_nominals[index]);
  lcd.setCursor(0,1);
  lcd.print(dropped_coins[index]);
  showTotal();
}

void clearCoins(){
  for(int i = 0; i < coin_count; i++){
    writeCoin(i, 0);
  }
  lcd.clear();
  showTotal();
}

void showTotal(){
  float total = 0;
  for (int i = 0; i < coin_count; i++){
    total += dropped_coins[i] * coin_nominals[i];
  }
  lcd.setCursor(9, 0);
  lcd.print("RUB");
  lcd.setCursor(9, 1);
  lcd.print(total);
}

byte current_coin = 0;
void loop() {
  for (int i = 0; i < coin_count; i++){
    if (digitalRead(coin_pins[i]) == HIGH){
      addCoin(i);
      delay(500);
      break;
    }
  }

  if(digitalRead(STAT_BTN) == HIGH){
    showCoin(current_coin);
    current_coin++;
    if (current_coin >= coin_count){
      current_coin = 0;
    }
    delay(500);
  }

  if (digitalRead(CLEAR_BTN) == HIGH){
    clearCoins();
    delay(500);
  }
}