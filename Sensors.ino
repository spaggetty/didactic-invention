void tempQuery(){
  ow.reset();                               // Отправляем импульс сброса и ждем подтвержение присутствия
  ow.skip();                                  // Пропускаем идентификацию датчика. Следующее измерение для всех датчиков на линии
  ow.write(0x44);                           // Convert T - Отправляем команду запуска измерений в режиме внешнего питания
  
}
boolean readData(const byte addr[]){            // Считывание данных с датчика DS18B20
  int8_t i=0;
  boolean check = false;
  do {
      ow.reset();                               // Отправляем импульс сброса и ждем подтвержение присутствия
      ow.select(addr);                          // Отправляем идентификатор для определения устройства
      ow.write(0xBE);                           // Read Scratchpad - Команда чтения ScratchPad (блокнотной памяти)
      for (int i = 0; i < 9; i++) {             // Нужно считать все 9 байт блокнотной памяти (0 и 1 байт - данные о температуре)
        
        data[i] = ow.read();
      }
      check = (OneWire::crc8(data, 8) == data[8]);
  }while (!check && ++i<3);
  return check;
}
int16_t getTemp(){                            // Целое, обозначающее температуру в четвертях градуса
    //             highByte        lowByte
    int16_t raw = (data[1] << 8) | data[0];   // Конвертируем два байта температуры в 16-битное целое
    raw >>= 2;                                // Сдвигаем на два незначащих для точности до 1/4 градуса (10 бит) разряда
    return raw;                               // Получаем целое, обозначающее реальное значение в четвертях градуса
}
void writeToDSEEPROM(const byte addr[]) {           // Запись maxt и mint в EEPROM датчика
  data[2] = byte(maxt);
  data[3] = byte(mint);
  ow.reset();                               // Отправляем импульс сброса и ждем подтвержение присутствия
  ow.select(addr);                          // Отправляем идентификатор для определения устройства
  ow.write(0x4E);                           // Write Scratchpad - Команда записи в ScratchPad следующих 3 байт:
  //-------------
  ow.write(data[2]);                        // TH - верхний триггер, температура отключения обогрева
  ow.write(data[3]);                        // TL -нижний триггер, температура включения обогрева
  ow.write(0x3F);                           // Регистр конфигурации, точность измерения до 1/4 градуса (10bit)
  //-------------
  delay (10);                               // задержка для гарантированной записи предыдущих 3 байт (min: 3 x 1,52ms) до сброса 
  ow.reset();                               // Отправляем импульс сброса и ждем подтвержение присутствия
  ow.select(addr);                          // Отправляем идентификатор для определения устройства
  ow.write(0x48);                           // Copy Scratchpad - Копирование 3 байт из ScratchPad в EEPROM
  Serial.println("3 bytes has been written to EEPROM of DS18B20.");
}


void getDS18B20(){     // заполняем массив температуры DS18B20, в десятых долях градуса
const byte addr2[8] = { 0x28, 0xFF, 0x8F, 0x2A, 0x50, 0x17, 0x4, 0x1F };  // спальня 
const byte addr3[8] = { 0x28, 0xFF, 0x5F, 0x26, 0x50, 0x17, 0x4, 0x53 };  // дальний подвал
const byte addr4[8] = { 0x28, 0xFF, 0x44, 0x36, 0x8B, 0x16, 0x3, 0xF8 };  // подвал
const byte addr5[8] = { 0x28, 0xFF, 0xC4, 0x69, 0x71, 0x17, 0x4, 0xD1 };  // погреб
const byte addr6[8] = { 0x28, 0xFF, 0xB9, 0x68, 0x71, 0x17, 0x4, 0xDF };  // улица
const byte* addrs [6] = {addr1,addr2,addr3,addr4,addr5,addr6}; // массив ссылок на массивы адресов датчиков
  while (ow.read()!=255){};
  for (int8_t i =5; i>-1;i--){
    if (readData(addrs[i])){                         //считываем t всех датчиков DS18B20 поочередно
       temps[i] = getTemp();                            
       switch (i){                                       // поправки на калибровку
        case 2:
        temps[i] +=1; 
          break;
        case 1:
        case 5:
          temps[i] -=2; 
          break;
       }
       temps[i] *=5;
       temps[i] >>=1;
    }else{
     temps[i] = 2550;
    }
  }
}
void getHTU(){                                // заполняем массив температуры HTU, в десятых градуса, процента
  temps[8] = myHTU21D.readTemperature();             //читаем температуру с HTU 
  temps[8]/= 10;
  temps[9] = myHTU21D.readHumidity();                   //читаем влажность с HTU - считаем влажность образцовой
  temps[9] = myHTU21D.getCompensatedHumidity(temps[8],temps[9]);
  temps[9]/= 10;
}
void getDHT(){                                // заполняем массив температуры DHT, в десятых градуса, процента
  if(dht.read(true)){                              //считываем данные DHT из памяти датчика
    temps[6] = dht.readTemperature()+1;                   //читаем температуру с DHT+ поправка на калибровку
    temps[7] = dht.readHumidity()-60;//-47;                      //читаем влажность с DHT + поправка на калибровку
  }else{
    temps[6] =2550;
    temps[7] = 2550;
  }
}
