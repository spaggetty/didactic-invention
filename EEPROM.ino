
void writeCntToEEPROM(){
  unsigned long timeInSec = atm/1000;
  byte *x = (byte *)&cnt;                 //Счетчик: 3,4 ячейки
  EEPROM.update(3,x[0]);                      //Старший байт
  delay(4);
  EEPROM.update(4,x[1]);                      //Младший байт
  delay(4);
  x = (byte *)&timeInSec;                     //Общее время нагрева: 5,6,7 ячейки
  for (byte i = 0;i<3;i++){                   // Байты записаны от старшего к младшему
    EEPROM.update(i+5,x[i]);                  //При совпадении байта он не перезаписывается
    delay(4);
  }
  timeInSec = ltm/1000;                //Время последнего нагрева: 8,9,10 ячейки
  x = (byte *)&timeInSec;
  for (byte i = 0;i<3;i++){                   // Байты записаны от старшеuго к младшему
    EEPROM.update(i+8,x[i]);                  //При совпадении байта он не перезаписывается
    delay(4);
  }
}
void readCntFromEEPROM(){
  cnt = EEPROM.read(3)|(EEPROM.read(4)<<8);
  unsigned long timeInSec =((unsigned long)EEPROM.read(7) << 16)|((unsigned long)EEPROM.read(6) << 8) |EEPROM.read(5);
  atm = timeInSec*1000;
  timeInSec =((unsigned long)EEPROM.read(10) << 16)|((unsigned long)EEPROM.read(9) << 8) |EEPROM.read(8);
  ltm = timeInSec*1000;
 /* Serial.println("cnt from EEPROM = ");
  Serial.println(cnt);
  Serial.println("atm from EEPROM = ");
  Serial.println(atm);
  Serial.println("ltm from EEPROM = ");
  Serial.println(ltm);*/
}
