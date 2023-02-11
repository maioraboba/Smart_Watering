//  Подключаем стандартную библиотеки для работы с Ethernet Shield'ом по шине SPI
#include <SPI.h>
#include <UIPEthernet.h> // подключаем библиотеку для Ethernet модуля

// определяем конфигурацию сети
byte mac[] = {0x28, 0x6C, 0x07, 0x98, 0x5C, 0x22}; // MAC-адрес
byte ip[] = {192, 168, 31, 255}; // IP-адрес
byte myDns[] = {192, 168, 31, 1}; // адрес DNS-сервера
byte gateway[] = {192, 168, 31, 1}; // адрес сетевого шлюза
byte subnet[] = {255, 255, 255, 0}; // маска подсети

EthernetServer server(2000); // создаем сервер, порт 2000
EthernetClient client;

void setup(){
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // инициализация контроллера
  server.begin(); // включаем ожидание входящих соединений
  pinMode(3, OUTPUT);
}

void loop(){
  client = server.available(); // ожидаем объект клиент
  if (client){
    if (client.available() > 0){
      char chr = client.read();
      if (chr == "0") digitalWrite(3, LOW);
      if (chr == "1") digitalWrite(3, HIGH);
      client.println(millis());
    }
  }
}