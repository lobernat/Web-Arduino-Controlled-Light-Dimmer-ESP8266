
/**************


    ---------------------- OUTPUT & INPUT Pin table ---------------------

    +---------------+-------------------------+-------------------------+
    | ESP8266       | D1(IO5),    D2(IO4),    | D0(IO16),   D1(IO5),    |
    |               | D5(IO14),   D6(IO12),   | D2(IO4),    D5(IO14),   |
    |               | D7(IO13),   D8(IO15),   | D6(IO12),   D7(IO13),   |
    |               |    zero                 | D8(IO15)  pw            |
    +---------------+-------------------------+-------------------------+
  Polsador ->D6 (arduino->12) o D7 (arduino->13)
  http://192.168.10.213/?temps=5&percent=99
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <RBDdimmer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#define USE_SERIAL Serial
#define outputPin  5 // wemos D1
#define zerocross  4 // Wemos D2 for boards with CHANGEBLE input pins
const int PinPolsador = 12; //Wemos D6
const int PercentInicial = 0; //entre 0 i 8 la bombeta no sencen gens... per a que comensi per 8

dimmerLamp dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
ESP8266WebServer server(80);

void AutoApagat();
//Ticker ApagatAutomatic;
Ticker timer1; // once, immediately


void setup() {
  USE_SERIAL.begin(9600);
  pinMode(PinPolsador, INPUT);



//Config Wifi Manager
    WiFiManager wifiManager;
    //first parameter is name of access point, second is the password
    wifiManager.autoConnect("lampara");
    wifiManager.setConfigPortalTimeout(180);
  //fi config wifi


  //inici web server
  server.on("/", handle_index); //Handle Index page

  server.begin(); //Start the server
  Serial.println("Server listening");
  //fi web server



  dimmer.begin(NORMAL_MODE, OFF); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(0);
  int percent = 55;
  float temps = 10;

}



void loop() {

  server.handleClient(); //Handling of incoming client requests

  if (digitalRead(PinPolsador) == HIGH) {         // check if the input is HIGH (button released)
    BotoApretat();
  }

}



void handle_index() {

  int temps, percent, estat;

  temps = server.arg("temps").toInt();
  percent = server.arg("percent").toInt();
  estat = server.arg("estat").toInt();


  if ((temps) and (percent)) {
    String PaginaWeb;
    PaginaWeb += F("Temps:");
    PaginaWeb += temps;
    PaginaWeb += F(" Percent:");
    PaginaWeb += percent;
    server.send(200, "text/plain", PaginaWeb);
    
    EncendreDimmer();
    IncrementarLlum(percent, temps);
    AutoApagat();

  } else if (estat) {
    int PotenciaActual = dimmer.getPower();
    String PaginaWeb;
    //PaginaWeb += F("Potencia Actual: ");
    PaginaWeb += dimmer.getPower();
    server.send(200, "text/plain", PaginaWeb);

  } else {
    String PaginaWeb;
    PaginaWeb += F("Temps o percent = null \nhttp://192.168.10.3/?temps=5&percent=99\n\nGet Percent Dimmer\nhttp://192.168.10.3/?estat=1");
    USE_SERIAL.println("Temps o percent = null");
    server.send(200, "text/plain", PaginaWeb);
    ApagarDimmer();
  }

  USE_SERIAL.println(temps);
  USE_SERIAL.println(percent);
}


void BotoApretat() {
  //detectar botó apretat
  int held = 0;
  while (digitalRead(PinPolsador) == HIGH && held < 9)
  {
    delay(50);
    held++;
    Serial.println(held);
  }
  if ((held >= 3) and (held < 5)) {
    Serial.println(" - - - - - ");
    IncrementarDimmer();
  }
  else if ((held >= 5) and (held < 9)) {
    Serial.println(" atope");
    EncendreDimmer();
    IncrementarLlum(99, 5);
    //delay(600);
  }
    else if (held == 9) {
    Serial.println(" + + + + + més de mig segon");
    ApagarDimmer();
    //delay(600);
  }
  //Fi detectar boto

}







void IncrementarLlum(int percent, int temps) {
  float retard = (temps * 1000) / percent;
  int PotenciaActual = dimmer.getPower();
  for (int i = PotenciaActual; i <= percent; i++)
  {
    dimmer.setPower(i);
    delay(retard);
    USE_SERIAL.print("Percent: -> ");
    USE_SERIAL.println(i);

      if (digitalRead(PinPolsador) == HIGH) {         // cancelar obrir llum
        break;
      }
    
  }
}


void EncendreDimmer() {
  dimmer.begin(NORMAL_MODE, ON);
}

void ApagarDimmer() {
  USE_SERIAL.println("Potencia Actual: -> ");
  Serial.print(dimmer.getPower());
  int PotenciaActual = dimmer.getPower();
  for (int i = PotenciaActual; i > 0; i--)
  {
    dimmer.setPower(i);
    delay(35);
    USE_SERIAL.print("Percent: -> ");
    USE_SERIAL.println(i);
  }

  dimmer.begin(NORMAL_MODE, OFF);
  timer1.detach();
}


void IncrementarDimmer() {
  dimmer.begin(NORMAL_MODE, ON);
  USE_SERIAL.println("Potencia Actual: -> ");
  Serial.print(dimmer.getPower());
  int PotenciaActual = dimmer.getPower();
  if (PotenciaActual < 2){
      PotenciaActual=PercentInicial;
    }

   int Increment = PotenciaActual+3;
  if (PotenciaActual > 2){
      Increment = Increment + 7;
    }

  for (int i = PotenciaActual; i <= Increment; i++)
  {
    dimmer.setPower(i);
    delay(30);
    USE_SERIAL.print("Percent: -> ");
    USE_SERIAL.println(i);
  }


}


void AutoApagat() {
   timer1.attach(1200, ApagarDimmer); //600 segons -> 10 minuts
}


/*
  Funciona OK:

  #include <RBDdimmer.h>//

  #define outputPin  5 // wemos D1
  #define zerocross  4 // Wemos D2 for boards with CHANGEBLE input pins

  dimmerLamp dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards


  void setup() {
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)
  }



  void loop() {

  int percent=99;
  float temps=60;
  EncendreLlum(percent,temps); //EncendreLlum(99,10)

  }



  void EncendreLlum(int percent, int temps){
  dimmer.setPower(percent);
  float retard=(temps*1000)/percent;

  for (int i = 0; i<=percent; i++)
      {
        dimmer.setPower(i);
        delay(retard);
      }


  }


*/
