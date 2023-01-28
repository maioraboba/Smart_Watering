//  Подключаем библиотеку для работы с сервисом ioContol
#include <iocontrol.h>
//  Подключаем стандартную библиотеки для работы с Ethernet Shield'ом по шине SPI
#include <SPI.h>
#include <UIPEthernet.h> // подключаем библиотеку для Ethernet модуля

//  Определяем выводы силовых ключей вентилятора и светодиодной ленты
#define LED_PIN 8
#define WATER_PIN 3

//  Определяем постоянные On и Off
#define On true
#define Off false

//  Название панели на сайте iocontrol.ru
const char* myPanelName = "smartwatering";
//  Название переменных на сайте
const char* panelHumidity = "humidity";
const char* panelLight = "light";
const char* panelLine= "line";
const char* panelWater = "waterpump";
const char* panelAuto = "Auto";
const char* panelRibbon = "ribbon_brightness";

//  Создаём объект клиента класса EthernetClient
EthernetClient client;
//  Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);

// определяем конфигурацию сети
byte mac[] = {0xAE, 0xB2, 0x26, 0xE4, 0x4A, 0x5C}; // MAC-адрес
byte ip[] = {192, 168, 0, 10}; // IP-адрес
byte myDns[] = {192, 168, 0, 1}; // адрес DNS-сервера
byte gateway[] = {192, 168, 0, 1}; // адрес сетевого шлюза
byte subnet[] = {255, 255, 255, 0}; // маска подсети
EthernetServer server(2000); // создаем сервер, порт 2000

bool waterHyst = false; // флаг помпы

//  Переменные для чтения с сайта ioControl
bool Auto = false; //  флаг автоматической работы

//  Переменные для записи на сайт ioControl
int hum; // влажность
uint16_t lum; // освещённость
int soilMoistureValue = 0;
int percentage = 0;

void setup()
{
    //  Инициируем работу с последовательным портом на скорости 9600 бод
    Serial.begin(9600);

    Ethernet.begin(mac, ip, myDns, gateway, subnet); // инициализация контроллера
    server.begin(); // включаем ожидание входящих соединений

    soilMoistureValue = analogRead(A0);
    hum = map(soilMoistureValue, 660, 0, 100, 0);

    lum = analogRead(A5);

    //  Вызываем функцию первого запроса к сервису ioControl
    mypanel.begin();
}

void loop()
{
    //  Записываем переменные из объекта mypanel в локальные переменные
    bool line = mypanel.readBool(panelLine);
    bool water = mypanel.readBool(panelWater);
    int rib = mypanel.readInt(panelRibbon);
    //  Записываем флаг автоматической работы в глобальную переменную
    Auto = mypanel.readBool(panelAuto);

    //  Если влажность превысила установленный порог
    if (hum < 80 && Auto) {
        // устанавливаем флаг помпы
        waterHyst = true;
        // выключаем помпу
        digitalWrite(3, HIGH);
        if (Auto)
          mypanel.write(panelWater, On);
    }
    //  Если не автоматический режим и флаг помпы установлен
    else if (!Auto && water) {
        // включаем вентилятор
        digitalWrite(3, HIGH);
        if (Auto)
          mypanel.write(panelWater, On);
    }
    //  Иначе, если флаг помпы не установлен
    else if (!waterHyst) {
        // Выключаем вентилятор
        digitalWrite(3, LOW);
        if (Auto)
          mypanel.write(panelWater, Off);
    }

    //  Если флаг помпы установлен
    if (waterHyst) {
        // сбрасываем флаг помпы
        waterHyst = false;
    }

    //  Если освещённость меньше установленного порога
    if (lum < 250 && Auto) {
        digitalWrite(LED_PIN, HIGH);
        if (Auto)
          mypanel.write(panelLight, On);
    }
    //  Если не автоматический режим и флаг света установлен
    else if (!Auto && line) {
        digitalWrite(LED_PIN, HIGH);
        if (Auto)
          mypanel.write(panelLight, On);
    }
    //  Иначе
    else {
        digitalWrite(LED_PIN, LOW);
        if (Auto)
          mypanel.write(panelLight, Off);
    }

    soilMoistureValue = analogRead(A0);
    hum = map(soilMoistureValue, 660, 0, 100, 0);

    lum = analogRead(A5);

    // Записываем показания всех датчиков в объект mypanel
    mypanel.write(panelHumidity, hum);
    mypanel.write(panelLight, lum);

    //  Если показания успешно записаны на сайт ioControl
    if (mypanel.writeUpdate() == OK) {
        //  выводим показания в последовательный порт
        Serial.println("Updated");
        Serial.println("hum: ");
        Serial.println(hum);
        Serial.println("lum: ");
        Serial.println(lum);
    }

    // Читаем все переменные панели с сайта в объект mypanel
    mypanel.readUpdate();
}