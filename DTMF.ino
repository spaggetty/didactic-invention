void SendDTMF(String dtmf){
  String _resp = "";
  String _dtmf="";
  unsigned long _timeout =0;
  uint8_t r = 0;
  
  do{
      if (dtmf.length()>31){
          _dtmf = dtmf.substring(0, 31);
          dtmf = dtmf.substring(32);
      }else {
        _dtmf = dtmf;
        dtmf="";
      }
      for(int s=0;s<(_dtmf.length()-1);s+=2){
        if (_dtmf.charAt(s)==_dtmf.charAt(s+2)){
          if (dtmf=="")dtmf = _dtmf.substring(s+2);
          else dtmf = _dtmf.substring(s+2)+","+dtmf;
          _dtmf = _dtmf.substring(0,s+1);
          break;
        }
      }
   
      int dlit = (_dtmf.length()+1)*200;  // [Кол-во DTMF-сигналов =(Кол-во знаков +1)/ 2] * ([пауза между сигн. = 100] +[длит. сигн. = 300]) 
      _dtmf = "AT+VTS=\""+_dtmf;
      _dtmf += "\"";
   
      dlit +=1000;
      String _resp = "";                            // Переменная для хранения результата
         
      SIM800.println(_dtmf);                          // Отправляем команду модулю
      _timeout = millis();                     // Переменная для отслеживания таймаута
      while ((millis()-_timeout)< dlit){};
      do{      
            if (SIM800.available()) {
                _resp = SIM800.readString();                // ... считываем и запоминаем 
                //if (_resp.indexOf("AT")> -1) Serial.println(_resp); 
                Serial.println(_resp);  
            }
      } while ((_resp.indexOf("OK")<0)&&((millis()-_timeout)< dlit*3));
      if (_resp.indexOf("OK")> -1) {
                Serial.println(_resp);                      // Дублируем ответ в монитор порта
      }else {                                        // Если пришел таймаут, то...
        Serial.println("Timeout...");               // ... оповещаем об этом и...
      }
  }while(dtmf.length()>0);
}

void processingDTMF(String symbol) {             // Отдельная функция для обработки логики DTMF
   
  Serial.println("Key: " + symbol);             // Выводим в Serial для контроля, что ничего не потерялось
  dtmfComand += symbol;
  if(symbol=="*") {
    DoCommand(dtmfComand);
    dtmfComand = "";
  }
}
