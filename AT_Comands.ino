String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            // Переменная для хранения результата
  Serial.println(cmd);                          // Дублируем команду в монитор порта
  SIM800.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
    
    if (_resp.startsWith(cmd)) {  // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
    Serial.println(_resp);                      // Дублируем ответ в монитор порта
  }
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                // Переменная для хранения результата
  unsigned long _timeout = millis();                     // Переменная для отслеживания таймаута
  while (!SIM800.available() && (millis()-_timeout) <10000 )  { // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  } 
  if (SIM800.available()) {
      
     /* while (SIM800.available()) {                     // Если есть, что считывать...
        resp = SIM800.read();
        if (resp >=0){
          _resp += char(resp);                // ... считываем и запоминаем
        }
        delay(10);
      }*/
      _resp = SIM800.readString();                // ... считываем и запоминаем 
  }
  else {                                        // Если пришел таймаут, то...
    Serial.println("Timeout...");               // ... оповещаем об этом и...
  }
  return _resp;                                 // ... возвращаем результат.
}
