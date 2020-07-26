#include <SoftwareSerial.h>                   // Библиотека програмной реализации обмена по UART-протоколу
#include <OneWire.h>
#include <EEPROM.h>
#include <DHT22.h>
#include <HTU21D.h>
#define DHTPIN 5
#define RELAY 8
#define ONE_WIRE_BUS 10
#define SENSCHEK 180000 //3 мин - интервал снятия температуры с сенсеров DS18B20
#define DHTUCHEK 600000 //10 мин - интервал снятия температуры с сенсеров DTH и HTU
#define PHONE1 "+79049121865"              // командный телефон 1
#define PHONE2 "+79159319659"              // командный телефон 2

DHT dht(DHTPIN);
HTU21D myHTU21D(0x00);
const byte addr1[8] = { 0x28, 0xFF, 0x81, 0x33, 0x8C, 0x16, 0x3, 0xF3 };  // насос
//const byte addr2[8] = { 0x28, 0xFF, 0x8F, 0x2A, 0x50, 0x17, 0x4, 0x1F };  // спальня 
//const byte addr3[8] = { 0x28, 0xFF, 0x5F, 0x26, 0x50, 0x17, 0x4, 0x53 };  // дальний подвал
//const byte addr4[8] = { 0x28, 0xFF, 0x5F, 0x1, 0x50, 0x17, 0x4, 0xA5 };  //  временно подвал
//const byte addr5[8] = { 0x28, 0xFF, 0xC4, 0x69, 0x71, 0x17, 0x4, 0xD1 };  // погреб
//const byte addr6[8] = { 0x28, 0xFF, 0xB9, 0x68, 0x71, 0x17, 0x4, 0xDF };  // улица
//const byte* addrs [6] = {addr1,addr2,addr3,addr4,addr5,addr6};           // массив ссылок на массивы адресов датчиков
OneWire ow(ONE_WIRE_BUS);
SoftwareSerial SIM800(2,3);                // RX, TX

unsigned long vklvr = 0;                    // момент включения нагрева
unsigned long previousMillis;               // отсчёт времени для звонка

unsigned long atm = 0;               // общая длительность нагрева
unsigned int cnt = 0;                     // кол-во включений нагрева
unsigned long ltm=0;                    // длительность последнего нагрева
int8_t mint = 4;//4;                        // температура включения обогрева
int8_t maxt = 11;//10;                      // температура отключения обогрева
//int16_t templast;                           // Переменные для хранения температуры насоса
byte data[9];                               // 9 байт, считываемых из блокнотной памяти датчиков DS18B20
int16_t temps[10];                          // Переменные для хранения температуры и влажности всех датчиков
unsigned long lastcheck, dhtuChek;          // фиксация времени последнего опроса датчиков

String dtmfComand = "";
String smsInd; 
bool query = false;
bool dhtuQuery = false;
bool ack = false;
int8_t tempn,tempx;

void setup() {
  String _response = "";                    // Переменная для хранения ответа модуля
  
  pinMode(RELAY, OUTPUT);                   //включаем управление реле
  digitalWrite(RELAY, HIGH);                //устанавливаем изначальное состояние реле - выкл.
 
  Serial.begin(9600);                       // Скорость обмена данными с компьютером
  SIM800.begin(9600);                       // Скорость обмена данными с модемом
  
  sendATCommand("AT", true);                // Автонастройка скорости модема
                                   
  dht.begin();                              //инициализируем DHT датчик  
  dht.read(true);                           //Запрос измерения DHT (должно пройти не менее 2 секунд после)
  tempQuery();                              //запускаем процедуру измерения температуры на всех датчиках DS18B20
  myHTU21D.begin();                         //инициализируем HTU датчик

  uint8_t r = 0;
  do{                                               //ждём включения модема, его регистрации в сети   
     _response = sendATCommand("AT+CPAS",true);
     if(_response.indexOf("0")<0) delay(3000);
  }while(_response.indexOf("0")<0 && ++r<100);

  sendATCommand("ATI", true);  
  sendATCommand("AT+DDET=1,0,1,0", true);          // Включаем DTMF с выводом клавиши и длительности
  r = 0;
  do {
    _response = sendATCommand("AT+CLIP=1", true);  // Включаем АОН
    _response.trim();                              // Убираем пробельные символы в начале и конце
    if (_response != "OK"){delay(1000);}
  } while (_response != "OK" && ++r<5);            // Не пускать дальше, пока модем не вернет ОК
  if(_response == "OK")Serial.println("CLIP активна");            // Информируем, что АОН включен
  sendATCommand("AT+VTD=3", true);                 // Устанавливаем длительность DTMF-сигнала в сотнях миллисекунд
  getDS18B20();                                   // Читаем температуру со всех датчиков DS18B20
  //templast = temps[0];
  mint = data[3];                                 //восстанавливаем mint и maxt из EEPROM датчика насоса
  maxt = data[2];
  readCntFromEEPROM();                            //считываем счетчик и время включений из EEPROM Arduino
  dtmfComand ="78*";                               // собираем информацию для отсылки сообщения о перезагрузке
  lastcheck = dhtuChek = millis();
  
  sendATCommand("ATD+79049121865;",true);         //звоним себе на телефон
  previousMillis=millis();
  //getHTU();                                       //Читаем данные с HTU
  getDHT();                                       //Читаем данные с DHT
  printTempsToSerial();                           //выводим температуру всех датчиков в сериал
  while(millis()-previousMillis<9000){}                // Ждем 9 секунд для успешного прохождения звонка
  sendATCommand("ATH",true);
}

void loop() {
  String _response = "";                          // Переменная для хранения ответа модуля
    
  //------------------------
    
    if (SIM800.available())   {                   // Если модем, что-то отправил...
      _response = waitResponse();                 // Получаем ответ от модема для анализа
      _response.trim();                           // Убираем лишние пробелы в начале и конце
      //Serial.println(_response);                  // Если нужно выводим в монитор порта
      int index = -1;
      do  {                                       // Перебираем построчно каждый пришедший ответ
          index = _response.indexOf("\r\n");        // Получаем идекс переноса строки
          String submsg = "";
          if (index > -1) {                         // Если перенос строки есть, значит
            submsg = _response.substring(0, index); // Получаем первую строку
            _response = _response.substring(index + 2); // И убираем её из пачки
          }else {                                    // Если больше переносов нет
            submsg = _response;                     // Последняя строка - это все, что осталось от пачки
            _response = "";                         // Пачку обнуляем
          }
          submsg.trim();                            // Убираем пробельные символы справа и слева
          if (submsg != "") {                       // Если строка значимая (не пустая), то распознаем уже её
            if (submsg.startsWith("+DTMF:")) {      // Если ответ начинается с "+DTMF:" тогда:
              String symbol = submsg.substring(7, 8);  // Выдергиваем символ с 7 позиции длиной 1 (по 8)
              processingDTMF(symbol);               // Логику выносим для удобства в отдельную функцию
            }
            else if (submsg.indexOf("+CLIP:")>-1) {   // При входящем звонке...
              if (submsg.indexOf(PHONE1)>-1||submsg.indexOf(PHONE2)>-1){
                sendATCommand("ATA", true);           // ...отвечаем (поднимаем трубку)
                if (dtmfComand != ""){                // Если готова инфа для отправки
                    DoCommand(dtmfComand);             // посылаем DTMF-код
                    dtmfComand = "";                  // обнуляем переменную хранения DTMF
                }
                
              }else{
                sendATCommand("ATH", true);        // Если нет, то отклоняем вызов
              }
            }
            else if (submsg.indexOf("+CMTI:")>-1) { // Пришло сообщение о приеме SMS
               uint8_t ind = submsg.indexOf(",")+1;
               smsInd = submsg.substring(ind)+",";
            }
          }
      } while (index > -1);                       // Пока индекс переноса строки действителен
    
    }else{
       if(smsInd!=""){                                                  // Если пришли СМС
        int8_t r = smsInd.length()*2;                                 // Ограничиваем кол-во попыток затереть все СМС: 
        do{                                                            // числом 4*кол-во СМС
            String cmd = smsInd.substring(0, smsInd.indexOf(','));     // Читаем первый адрес СМС 
            smsInd = smsInd.substring(smsInd.indexOf(',')+1);          // Убираем его из очереди
            if (cmd!=""){
              _response = sendATCommand(("AT+CMGR=" + cmd),false);        // Пытаемся читать первое СМС
              int8_t sh = 0;
              while((_response.indexOf("OK")==-1)&&(++sh<4)){    
                 _response = waitResponse();
              }
               _response = sendATCommand("AT+CMGD="+ cmd+",0",true);    // Пытаемся удалить его
              if(_response.indexOf("OK")==-1) smsInd +=cmd+",";        // При неудаче записываем его адрес обратно в очередь последним
              
            }
        }while (smsInd !="" && ((--r)>-1));                              // Повтор, пока не удалим все, или закончится счетчик неудачных попыток
      }
    }
   //-------------------------------------------------------
   if (Serial.available())  {                    // Ожидаем команды по Serial...
        if(Serial.peek()=='c'){
            Serial.read();
            printTempsToSerial();
        }else{
          SIM800.write(Serial.read());                // ...и отправляем полученную команду модему
        }
   } 
  
   //-------------------------------------------------------
    
    if (query){
        getDS18B20();
        query = false;
        if ((temps[0]<(mint*10))&&(digitalRead(RELAY)==HIGH)){
          vklvr = millis();
          digitalWrite(RELAY, LOW);
            if(dtmfComand.indexOf("90*")<0){
              dtmfComand +="90*";
            }
            
        }
        if ((temps[0]>(maxt*10))&& temps[0]<2000 &&(digitalRead(RELAY)==LOW)){
          ltm = millis()-vklvr;
          digitalWrite(RELAY, HIGH);
          atm += ltm;
          writeCntToEEPROM();                            //Записываем значение счетчика и времени включения в EEPROM
        }
    }
    if (dhtuQuery){
          while(millis()-dhtuChek<2000){}; 
          getDHT();                                       //Читаем данные с DHT
          getHTU();                                       //Читаем данные с HTU
          dhtuQuery=false;
    }
    if (millis()-dhtuChek>DHTUCHEK){
        dht.read(true);                                 //Запрос измерения DHT (должно пройти не менее 2 секунд после)
        dhtuQuery = true;
        dhtuChek=millis();
    }
    if (millis()-lastcheck>SENSCHEK){
      tempQuery();                              //запускаем измерение температуры на всех датчиках
      query = true;
      lastcheck = millis();
    } 

}
