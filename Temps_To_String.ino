void printTempsToSerial(){
      String strTemp;
      char* places[] = {"насос","спальня","подвал2","подвал","погреб","улица","столовая t","столовая h","баня t","баня h"};                            // именованияя датчиков
      for(byte i = 0;i<10;i++){
        strTemp= TempToStr(temps[i]);
        Serial.print(places[i]);
        Serial.print(" = ");
        Serial.println(strTemp);
      }
}

String TempToStr(int16_t temp){
  int temp1 = temp/10;
  int temp2 = abs(temp%10);               
  String rez = String(temp1)+","+String(temp2);
  return rez;
}

String TempForDTMF(int16_t temp){
  String rez ="0,";
  if(temp<0){
    rez = "1,";
    temp = abs(temp);
  }
  int temp1 = temp/1000;
  if(temp1>0) rez += String(temp1)+",";
  temp1 = (temp%1000)/100;
  if((temp1>0)||(rez.length()>2))rez += String(temp1)+",";
  temp1 = (temp%100)/10;
  if((temp1>0)||(rez.length()>2))rez += String(temp1)+",";
  temp1 = temp%10;
  rez += String(temp1)+",";
  return rez;
}
String TimeForDTMF(unsigned long tmr){
  String rez = "";
  String temp = String(tmr);
  for(int n =0;n<temp.length();n++){
    rez += temp.charAt(n);
    rez += ",";
  }
  return rez;
}
