
void DoCommand(String result){
    unsigned long tempTime = 0;
    result.trim();
    String dtmfCmd ="";
    if (result == "78*"){ //Послать температуру насоса
      dtmfCmd +="7,#,";     //Была перезагрузка
      dtmfCmd +=TempForDTMF(temps[0])+"#";    // t насоса
    }else if(result.startsWith("1")&&(result.length()> 2)){ //послать температуру
      String nmbSens = String(result.charAt(1));
      uint8_t dig = nmbSens.toInt();
      if(result.length()== 4 && result.charAt(2)=='1'){
         if (dig<6){
            tempQuery();
            getDS18B20();
         }else if (dig<8) {
            dht.read(true);
            tempTime = millis();
            while ((millis()-tempTime)<2000){};
            getDHT();
         }else{
            getHTU();
         }
      }
      dtmfCmd +=String(dig);
      dtmfCmd +=",*,";
      dtmfCmd +=TempForDTMF(temps[dig])+"#";
    }else if(result=="34*"){                  //Послать время и кол-во включений
      dtmfCmd +=TimeForDTMF(cnt)+"#,";        //Счетчик
      tempTime = atm/1000;
      dtmfCmd +=TimeForDTMF(tempTime)+"#,";   //Общая длительность включения
      tempTime =ltm/1000;
      dtmfCmd +=TimeForDTMF(tempTime)+"#";    //Длительность последнего включения
    }else if(result=="56*"){                  //Послать mint и maxt
      dtmfCmd +=TempForDTMF((int16_t)mint);        
      dtmfCmd +=TempForDTMF((int16_t)maxt)+"#";
    }else if(result=="90*"||result == "78*90*"){ //Всё послать
      if(result == "78*90*") dtmfCmd +="7,#,";   //Была перезагрузка
      dtmfCmd +="1,#,";     //Включился нагрев 
      dtmfCmd +=TempForDTMF(temps[0])+"#,";   // t насоса
      dtmfCmd +=TimeForDTMF(cnt)+"#,";        //Счетчик
      tempTime = atm/1000;
      dtmfCmd +=TimeForDTMF(tempTime)+"#,";   //Общая длительность включения
      tempTime =ltm/1000;
      dtmfCmd +=TimeForDTMF(tempTime)+"#,";   //Длительность последнего включения
      dtmfCmd +=TempForDTMF((int16_t)mint)+"#,";  // mint     
      dtmfCmd +=TempForDTMF((int16_t)maxt)+"#";   // maxt
      
      
    }else if(result.startsWith("65")&&(result.length() == 7)){                    //Записать новые mint и maxt
      String integ = result.substring(2,4);
      tempn =integ.toInt();
      integ = result.substring(4,6);
      tempx =integ.toInt();
      if ((tempn>-1)&&(tempn<40)&&(tempx>0)&&(tempx<40)&&(tempn<tempx)){
        ack = true;
        dtmfCmd +=TempForDTMF(tempn);        
        dtmfCmd +=TempForDTMF(tempx)+"#";
      }else{
        ack = false;
        dtmfCmd += "*,#";
      }
      
    }else if(ack &&(result=="1*")){
      mint = tempn;
      maxt = tempx;
      writeToDSEEPROM(addr1);
      
      dtmfCmd +=TempForDTMF(mint);        
      dtmfCmd +=TempForDTMF(maxt)+"#";
    }else{
      dtmfCmd += "*,#";
    }
    SendDTMF(dtmfCmd);
    
}
