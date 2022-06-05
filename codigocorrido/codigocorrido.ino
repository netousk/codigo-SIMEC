#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <virtuabotixRTC.h>//rtc
#include "EmonLib.h"//sensor de corrente

//armazena a corrente atual a cada 10 minutos
    double correntem10, correntem20, correntem30,
           correntem40, correntem50, correntem59;
           
//armazenar a corrente media a cada 1 hora
    double correntehweb;
    double correnteh00, correnteh01, correnteh02,
           correnteh03, correnteh04, correnteh05, 
           correnteh06, correnteh07, correnteh08, 
           correnteh09, correnteh10, correnteh11,
           correnteh12, correnteh13, correnteh14, 
           correnteh15, correnteh16, correnteh17, 
           correnteh18, correnteh19, correnteh20, 
           correnteh21, correnteh22, correnteh23;
           
//armazena a corrente media a cada 1 dia
    double correntedweb;          
    double corrented01, corrented02, corrented03,
           corrented04, corrented05, corrented06,
           corrented07, corrented08, corrented09,
           corrented10, corrented11, corrented12,
           corrented13, corrented14, corrented15,
           corrented16, corrented17, corrented18, 
           corrented19, corrented20, corrented21, 
           corrented22, corrented23, corrented00;
            
//variaveis para calcular previsao de consumo
    double previsaoweb;
    double mediadia6, mediadia60;
    double mediadia10, mediadia100; 
    double mediadia15, mediadia150;
    double mediadia24, mediadia240;
    double mediadiakwh, consumokwh;
    
//armazena o percentual de consumo
    double percentual;

//armazena hr e data
    int hvar, mvar;
    int dvar, mmvar, avar;
    
//bandeira tarifaria
    double tarifa = 0.68;
    double tarifaband;

//senso de corrente
    EnergyMonitor sensor;
    double Irms;
    double corrente = A0;   
    int tensao = 127;
    double potencia = corrente*tensao;

//esp8266
    const char* ssid = "81324VCT6_2.4G";//nome wifi
    const char* password = "03851972252";//senha wifi
    String blocosup = "blocosup" ;

//RTC
    virtuabotixRTC myRTC(5, 4, 0);

//Rele
    int rele1 = 14;
    int rele2 = 12;
    
ESP8266WebServer server(80);

void handleRoot() {

    String textoHTML = "<html><head> <meta http-equiv=\"refresh\" content=\"5\"> <title> PAINEL DE MONITORAMENTO </title></head>";
    textoHTML += "<style>body { background-color: #23282B; ";
    textoHTML += "font-family: Arial, Helvetica, Sans-Serif; ";
    textoHTML += "Color: #EDEEE9 }";
    textoHTML += "h2 { font-family: Calibri; text-align: center; font-size: 30px; }";
    textoHTML += "p { font-size: 20px; }";
    textoHTML += "#RTC {  text-align: center; }";
    textoHTML +=  "#BLOCOS { display: inline-block; }";
    textoHTML +=  ".blocosup { border-radius: 5px; margin: 3px 3px 3px; text-align: center; border: 2px solid white;  background: #3c7bb0 ;width: 200px;height: 100px; } </style>";
    textoHTML += "</head><body>";
    textoHTML += "<h2>PAINEL DE MONITORAMENTO</h2>";
    
    textoHTML += "<div id= \"RTC\" class=\"DATA\"> <b> Data: </b>";
    textoHTML += dvar;
    textoHTML += "/";
    textoHTML += mmvar;
    textoHTML += "/";
    textoHTML += avar;
    textoHTML += "</div>";
    textoHTML += "<div id= \"RTC\" class=\"HORA\"> <b> Hora: </b>";
    textoHTML += hvar;
    textoHTML += ":";
    textoHTML += mvar;
    textoHTML += "<br></div>";

    
    textoHTML += "<div id =\"BLOCOS\" class=\"blocosup\"><p>Potencia atual:</p>";
    textoHTML += potencia;
    textoHTML += "W</div><div id =\"BLOCOS\" class=\"blocosup\"><p>Consumo:</p>";
    textoHTML += consumokwh;
    textoHTML += "KWh</div><div id =\"BLOCOS\" class=\"blocosup\"><p>Consumo ultimo dia:</p>";
    textoHTML += correntedweb*tensao;
    textoHTML += "W</div><div id =\"BLOCOS\" class=\"blocosup\"><p>Previsao do consumo:</p>";
    textoHTML += mediadiakwh*tarifa;
    textoHTML += " R$</div><div id =\"BLOCOS\" class=\"blocosup\"><p>consumo c/acrescimo:</p>";
    textoHTML += tarifaband;
    textoHTML += " R$</div><div id =\"BLOCOS\" class=\"blocosup\"><p>Percentual consumo:</p>";
    textoHTML += percentual;
    textoHTML += "%</div></body></html>";
   
    server.send(200, "text/html", textoHTML);
}
void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    
    for (uint8_t i=0; i<server.args(); i++){
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
    server.send(404, "text/plain", message);
}


void setup() {
    Serial.begin(115200);
//Rele
    pinMode(rele1, OUTPUT); 
    pinMode(rele2, OUTPUT);
    
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, LOW);
  
//sensor de corrente
   sensor.current(corrente, 9.0909);

//RTC
   //segundos, minutos, horas, dia semana, dia do mes, mes, ano
   myRTC.setDS1302Time(0, 0, 1, 1, 11, 1, 2022);

//esp8266
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // espera pela conexao
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);

    server.on("/inline", [](){
      server.send(200, "text/plain", "this works as well");
    });

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {

    //esp8266
    server.handleClient();

    //sensor de corrente
    Irms = sensor.calcIrms(1500);  
    
    potencia = Irms * tensao;

    Serial.print("Corrente atual = ");
    Serial.print(Irms);
    Serial.println(" A");
    
    Serial.print("Potencia atual = ");
    Serial.print(potencia);
    Serial.println(" W");

    double consumo = (potencia/1000);
    Serial.print("consumo atual = ");
    Serial.print(consumo);
    Serial.println(" kWh");
    
//minutos do dia    
    if(myRTC.seconds==10){
      correntem10 = Irms;
    }
    if(myRTC.seconds==21){
      correntem20 = Irms;
    }
    if(myRTC.seconds==31){
      correntem30 = Irms;
    }
    if(myRTC.seconds==40){
      correntem40 = Irms;
    }
    if(myRTC.seconds==50){
      correntem50 = Irms;
    }
    if(myRTC.seconds==59){
      correntem59 = Irms;
    }

    Serial.print("corrente minuto 10 = ");
    Serial.println(correntem10);
    Serial.print("corrente minuto 20 = ");
    Serial.println(correntem20);
    Serial.print("corrente minuto 30 = ");
    Serial.println(correntem30);
    Serial.print("corrente minuto 40 = ");
    Serial.println(correntem40);
    Serial.print("corrente minuto 50 = ");
    Serial.println(correntem50);
    Serial.print("corrente minuto 59 = ");
    Serial.println(correntem59);


//horas do dia
    switch (myRTC.minutes) {
      case 00:
      correntehweb = correnteh00;
      break;
      case 01: 
      correntehweb = correnteh01;  
      break;
      case 02:
      correntehweb = correnteh02;
      break;
      case 03: 
      correntehweb = correnteh03;  
      break;
      case 04:
      correntehweb = correnteh04;
      break;
      case 05: 
      correntehweb = correnteh05;  
      break;
      case 06:
      correntehweb = correnteh06;
      break;
      case 07: 
      correntehweb = correnteh07;  
      break;
      case 8:
      correntehweb = correnteh08;
      break;
      case 9: 
      correntehweb = correnteh09;  
      break;
      case 10:
      correntehweb = correnteh10;
      break;
      case 11: 
      correntehweb = correnteh11;  
      break;
      case 12:
      correntehweb = correnteh12;
      break;
      case 13: 
      correntehweb = correnteh13;  
      break;
      case 14:
      correntehweb = correnteh14;
      break;
      case 15: 
      correntehweb = correnteh15;  
      break;
      case 16:
      correntehweb = correnteh16;
      break;
      case 17: 
      correntehweb = correnteh17;  
      break;
      case 18:
      correntehweb = correnteh18;
      break;
      case 19: 
      correntehweb = correnteh19;  
      break;
      case 20:
      correntehweb = correnteh20;
      break;
      case 21: 
      correntehweb = correnteh21;  
      break;
      case 22:
      correntehweb = correnteh22;
      break;
      case 23: 
      correntehweb = correnteh23;  
      break;
      default:
      break;
}
    if(myRTC.minutes==00){
      correnteh00 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==1){
      correnteh01 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==2){
      correnteh02 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==3){
      correnteh03 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==4){
      correnteh04 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==5){
      correnteh05 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==6){
      correnteh06 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==7){
      correnteh07 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==8){
      correnteh08 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==9){
      correnteh09 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==10){
      correnteh10 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==11){
      correnteh11 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==12){
      correnteh12 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==13){
      correnteh13 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==14){
      correnteh14 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==15){
      correnteh15 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==16){
      correnteh16 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==17){
      correnteh17 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==18){
      correnteh18 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==19){
      correnteh19 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==20){
      correnteh20 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==21){
      correnteh21 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==22){
      correnteh22 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }
    if(myRTC.minutes==23){
      correnteh23 = (correntem10+correntem20+correntem30+correntem40+correntem50+correntem59)/6;
    }

    Serial.print("corrente media hora 01 = ");
    Serial.println(correnteh01);

    Serial.print("corrente media hora 02 = ");
    Serial.println(correnteh02);

    Serial.print("corrente media hora 03 = ");
    Serial.println(correnteh03);
    
    Serial.print("corrente media hora 04 = ");
    Serial.println(correnteh04);

    Serial.print("corrente media hora 05 = ");
    Serial.println(correnteh05);

    Serial.print("corrente media hora 06 = ");
    Serial.println(correnteh06);

    Serial.print("corrente media hora 07 = ");
    Serial.println(correnteh07);

    Serial.print("corrente media hora 08 = ");
    Serial.println(correnteh08);
    
    Serial.print("corrente media hora 09 = ");
    Serial.println(correnteh09);
    
    Serial.print("corrente media hora 10 = ");
    Serial.println(correnteh10);
    
    Serial.print("corrente media hora 11 = ");
    Serial.println(correnteh11);
    
    Serial.print("corrente media hora 12 = ");
    Serial.println(correnteh12);

    Serial.print("corrente media hora 13 = ");
    Serial.println(correnteh13);
    
    Serial.print("corrente media hora 14 = ");
    Serial.println(correnteh14);
    
    Serial.print("corrente media hora 15 = ");
    Serial.println(correnteh15);
    
    Serial.print("corrente media hora 16 = ");
    Serial.println(correnteh16);
    
    Serial.print("corrente media hora 17 = ");
    Serial.println(correnteh17);

    Serial.print("corrente media hora 18 = ");
    Serial.println(correnteh18);

    Serial.print("corrente media hora 19 = ");
    Serial.println(correnteh19);

    Serial.print("corrente media hora 20 = ");
    Serial.println(correnteh20);

    Serial.print("corrente media hora 21 = ");
    Serial.println(correnteh21);

    Serial.print("corrente media hora 22 = ");
    Serial.println(correnteh22);

    Serial.print("corrente media hora 23 = ");
    Serial.println(correnteh23);

    Serial.print("corrente media hora 24 = ");
    Serial.println(correnteh00);

//dias
    switch (myRTC.hours) {
      case 01: 
      correntedweb = corrented01;  
      break;
      case 02:
      correntedweb = corrented02;
      break;
      case 03: 
      correntedweb = corrented03;  
      break;
      case 04:
      correntedweb = corrented04;
      break;
      case 05: 
      correntedweb = corrented05;  
      break;
      case 06:
      correntedweb = corrented06;
      break;
      case 07: 
      correntedweb = corrented07;  
      break;
      case 8:
      correntedweb = corrented08;
      break;
      case 9: 
      correntedweb = corrented09;  
      break;
      case 10:
      correntedweb = corrented10;
      break;
      case 11: 
      correntedweb = corrented11;  
      break;
      case 12:
      correntedweb = corrented12;
      break;
      case 13: 
      correntedweb = corrented13;  
      break;
      case 14:
      correntedweb = corrented14;
      break;
      case 15: 
      correntedweb = corrented15;  
      break;
      case 16:
      correntedweb = corrented16;
      break;
      case 17: 
      correntedweb = corrented17;  
      break;
      case 18:
      correntedweb = corrented18;
      break;
      case 19: 
      correntedweb = corrented19;  
      break;
      case 20:
      correntedweb = corrented20;
      break;
      case 21: 
      correntedweb = corrented21;  
      break;
      case 22:
      correntedweb = corrented22;
      break;
      case 23: 
      correntedweb = corrented23;  
      break;
      case 24: 
      correntedweb = corrented00;  
      break;
      default:
      break;    
}
       
   
    if(myRTC.hours==1){
      corrented01 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==2){
      corrented02 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==3){
      corrented03 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==4){
      corrented04 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==5){
      corrented05 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==6){
      corrented06 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==7){
      corrented07 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==8){
      corrented08 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }                
    if(myRTC.hours==9){
      corrented09 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==10){
      corrented10 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==11){
      corrented11 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==12){
      corrented12 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==13){
      corrented13 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==14){
      corrented14 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==15){
      corrented15 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==16){
      corrented16 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==17){
      corrented17 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==18){
      corrented18 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==19){
      corrented19 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==20){
      corrented20 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==21){
      corrented21 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==22){
      corrented22 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==23){
      corrented23 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24;
    }
    if(myRTC.hours==0){
      corrented00 = (correnteh00+correnteh01+correnteh02+correnteh03+correnteh04+
                     correnteh05+correnteh06+correnteh07+correnteh08+correnteh09+
                     correnteh10+correnteh11+correnteh12+correnteh13+correnteh14+
                     correnteh15+correnteh16+correnteh17+correnteh18+correnteh19+
                     correnteh20+correnteh21+correnteh22+correnteh23)/24; 
    }
    Serial.print("corrente media dia 01 = ");
    Serial.println(corrented01);

    Serial.print("corrente media dia 02 = ");
    Serial.println(corrented02);

    Serial.print("corrente media dia 03 = ");
    Serial.println(corrented03);

    Serial.print("corrente media dia 04 = ");
    Serial.println(corrented04);

    Serial.print("corrente media dia 05 = ");
    Serial.println(corrented05);

    Serial.print("corrente media dia 06 = ");
    Serial.println(corrented06);

    Serial.print("corrente media dia 07 = ");
    Serial.println(corrented07);

    Serial.print("corrente media dia 08 = ");
    Serial.println(corrented08);

    Serial.print("corrente media dia 09 = ");
    Serial.println(corrented09);

    Serial.print("corrente media dia 10 = ");
    Serial.println(corrented10);

    Serial.print("corrente media dia 11 = ");
    Serial.println(corrented11);

    Serial.print("corrente media dia 12 = ");
    Serial.println(corrented12);

    Serial.print("corrente media dia 13 = ");
    Serial.println(corrented13);

    Serial.print("corrente media dia 14 = ");
    Serial.println(corrented14);

    Serial.print("corrente media dia 15 = ");
    Serial.println(corrented15);

    Serial.print("corrente media dia 16 = ");
    Serial.println(corrented16);

    Serial.print("corrente media dia 17 = ");
    Serial.println(corrented17);

    Serial.print("corrente media dia 18 = ");
    Serial.println(corrented18);

    Serial.print("corrente media dia 19 = ");
    Serial.println(corrented19);

    Serial.print("corrente media dia 20 = ");
    Serial.println(corrented20);

    Serial.print("corrente media dia 21 = ");
    Serial.println(corrented21);

    Serial.print("corrente media dia 22 = ");
    Serial.println(corrented22);

    Serial.print("corrente media dia 23 = ");
    Serial.println(corrented23);

    Serial.print("corrente media dia 24 = ");
    Serial.println(corrented00);

    
//previsao/consumo  
  //previsao baseado nos 6 primeiros dias        
    if((myRTC.hours >= 6)&(myRTC.hours <= 9)){
      mediadia6 =((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06)*24*tensao);
      
      mediadiakwh = (mediadia6*5)/1000;
      mediadia60 = ((mediadia6*5)/1000)*tarifa;
      
    }  
  //previsao baseado nos 10 primeiros dias        
    if((myRTC.hours >= 10)&(myRTC.dayofmonth <= 14)){
      mediadia10 =((corrented01+corrented02+corrented03+corrented04+corrented05+
                     corrented06+corrented07+corrented08+corrented09+corrented10)*24*tensao);
      
      mediadiakwh = (mediadia10*3)/1000;
      mediadia100 = ((mediadia10*3)/1000)*tarifa;

    }
//previsao baseado nos 15 primeiros dias
    if((myRTC.hours >= 15)&(myRTC.dayofmonth <= 22)){
      mediadia15 =((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15)*24*tensao);

      mediadiakwh = (mediadia15*2)/1000;
      mediadia150 = ((mediadia15*2)/1000)*tarifa;

        }       
//previsao baseado nos 24 primeiros dias
    if((myRTC.hours >= 23)){
      mediadia24 =((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22+corrented23)*24*tensao);

      mediadiakwh = (mediadia24)/1000;
      mediadia240 = (mediadia24/1000)*tarifa;
      
      }

    switch (myRTC.hours) {
      case 6:
      previsaoweb = mediadia60;
      break;
      case 10:
      previsaoweb = mediadia100;
      break;
      case 15: 
      previsaoweb = mediadia150;  
      break;
      case 24:
      previsaoweb = mediadia240;
      break;
      default:
      break;
    } 
//percenual de consumo
      switch (myRTC.dayofmonth) {
      case 1:
      percentual = ((((corrented01)*127)/1000)/150)*100;     
      consumokwh = ((corrented01)*127)/1000;   
      break;
      case 2:
      percentual = ((((corrented01+corrented02)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02)*127)/1000; 
      break;
      case 3: 
      percentual = ((((corrented01+corrented02+corrented03)*127)/1000)/150)*100;  
      consumokwh = ((corrented01+corrented02+corrented03)*127)/1000;
      break;
      case 4:
      percentual = ((((corrented01+corrented02+corrented03+corrented04)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04)*127)/1000;  
      break;
      case 5:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05)*127)/1000;  
      break;
      case 6:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06)*127)/1000;  
      break;
      case 7: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07)*127)/1000)/150)*100; 
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07)*127)/1000;  
      break;
      case 8:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08)*127)/1000;  
      break;
      case 9: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09)*127)/1000;  
      break;
      case 10:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10)*127)/1000;  
      break;
      case 11:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11)*127)/1000;  
      break;
      case 12:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12)*127)/1000;  
      break;
      case 13: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13)*127)/1000)/150)*100;  
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13)*127)/1000;  
      break;
      case 14:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14)*127)/1000;  
      break;
      case 15:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15)*127)/1000;  
      break;
      case 16:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16)*127)/1000;  
      break;
      case 17: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17)*127)/1000)/150)*100; 
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17)*127)/1000;  
      break;
      case 18:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18)*127)/1000;  
      break;
      case 19: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19)*127)/1000;  
      break;
      case 20:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20)*127)/1000)/150)*100;
      consumokwh = ((corrented01)*127)/1000;  
      break;
      case 21:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21)*127)/1000;  
      break;
      case 22: 
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22)*127)/1000;  
      break;
      case 23:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22+corrented23)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22+corrented23)*127)/1000;  
      case 0:
      percentual = ((((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22+corrented23+corrented00)*127)/1000)/150)*100;
      consumokwh = ((corrented01+corrented02+corrented03+corrented04+corrented05+
                   corrented06+corrented07+corrented08+corrented09+corrented10+
                   corrented11+corrented12+corrented13+corrented14+corrented15+
                   corrented16+corrented17+corrented18+corrented19+corrented20+
                   corrented21+corrented22+corrented23+corrented00)*127)/1000;  
      break;             
      break;
      default:
      break;
    }
    
//calculo bandeira tarifaria
    if((mediadiakwh >= 10)&&(mediadiakwh < 20)){
      tarifaband = previsaoweb+3.971;
    }
    if((mediadiakwh >= 20)&&(mediadiakwh < 30)){
      tarifaband = previsaoweb+7,942;
    }
    if((mediadiakwh >= 30)&&(mediadiakwh < 40)){
      tarifaband = previsaoweb+11,913;
    }
    if((mediadiakwh >= 40)&&(mediadiakwh < 50)){
      tarifaband = previsaoweb+15,884;
    }
    if((mediadiakwh >= 50)&&(mediadiakwh < 60)){
      tarifaband = previsaoweb+19,855;
    }
    if((mediadiakwh >= 60)&&(mediadiakwh < 70)){
      tarifaband = previsaoweb+23,826;
    }else{
      tarifaband = previsaoweb;
    }
    
    if((myRTC.minutes >= 40)&&(myRTC.minutes < 50)){
      digitalWrite(rele1, HIGH);//desliga rele
    }else{
      digitalWrite(rele1, LOW);//liga rele
    }
    if((myRTC.minutes >= 1)&&(myRTC.minutes < 10)){
      digitalWrite(rele2, HIGH);//liga rele
    }else{
      digitalWrite(rele2, LOW);//liga rele
    }

//RTC  
    myRTC.updateTime();

    Serial.print("data e hora atual ");
    Serial.print(myRTC.dayofmonth);
    Serial.print("/");
    Serial.print(myRTC.month);
    Serial.print("/");
    Serial.print(myRTC.year);
    Serial.print(" ");
    Serial.print(myRTC.hours);
    Serial.print(":");
    Serial.print(myRTC.minutes);
    Serial.print(":");
    Serial.println(myRTC.seconds);
    Serial.println("**");
    Serial.println("**");
    hvar = myRTC.hours;
    mvar = myRTC.minutes;
    dvar = myRTC.dayofmonth;
    mmvar = myRTC.month;
    avar = myRTC.year;
    delay(1000);
  }
