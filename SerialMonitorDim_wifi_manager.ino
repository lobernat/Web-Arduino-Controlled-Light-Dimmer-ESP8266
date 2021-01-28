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

  No funciona amb les llibreries ESP8266 noves
  funciona ok amb la 2.4.2
  
  Polsació molt llarga, puja la persiana amb un GET
  
*/
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <RBDdimmer.h>
#include <DNSServer.h>
#define USE_SERIAL Serial
#define outputPin  5 // wemos D1
#define zerocross  4 // Wemos D2 for boards with CHANGEBLE input pins
const int PinPolsador = 12; //Wemos D6
const int PercentInicial = 0; //entre 0 i 8 la bombeta no sencen gens... per a que comensi per 8

int dobleclick = 0;
unsigned long iniciMillis = 0;
int duradaApretat = 0;
int parasit = 0;

dimmerLamp dimmer(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
ESP8266WebServer server(80);

void AutoApagat();
//Ticker ApagatAutomatic;
Ticker timer1; // once, immediately


void setup() {
  USE_SERIAL.begin(9600);
  pinMode(PinPolsador, INPUT);



  //Config Wifi
  WiFi.hostname("Lampara");
  WiFi.mode(WIFI_STA);
  WiFi.begin("honeypot", "internetdelescoses");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
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
    PaginaWeb += F("<html>Temps o percent = null <br> <a href=\"http://192.168.10.3/?temps=5&percent=99\">http://192.168.10.3/?temps=5&percent=99  </a>\n\n<br><p> Get Percent Dimmer\nhttp://192.168.10.3/?estat=1");
    USE_SERIAL.println("Temps o percent = null");
    server.send(200, "text/html", PaginaWeb);
    ApagarDimmer();
  }

  USE_SERIAL.println(temps);
  USE_SERIAL.println(percent);
}


void BotoApretat() {

  //mirar si s'ha fet dobleclick
  if ( (millis() - iniciMillis < 400) && (millis() - iniciMillis > 20)&& parasit == 0) {
    dobleclick = 1;
  } else {
    dobleclick = 0;
  }
  
  
  iniciMillis = millis();
  while (digitalRead(PinPolsador) == HIGH)
  {
    delay(1);
  }
  duradaApretat = millis() - iniciMillis;
  
  //Serial.println(duradaApretat);

  if (duradaApretat < 20 ) {
       parasit = 1;
        delay(5);
    return;
  }else{
    parasit = 0;
    }

  //mirar dobleclick i encendre a tope
  if (dobleclick == 1 && duradaApretat > 30 && duradaApretat < 500) {
    Serial.println("Doble click");
    EncendreDimmer();
    IncrementarLlum(99, 5);
    dobleclick = 0;
    return;
  }

  //click sol encendre un punt
  if (dobleclick == 0 && duradaApretat > 30 && duradaApretat < 500) {
    Serial.println("Click");
    IncrementarDimmer();
    return;
  }

  //apretat llarg -- apagar llum
  if (dobleclick == 0 && duradaApretat > 500 && duradaApretat < 4000 ) {
    Serial.println("Llarg");
    ApagarDimmer();
    return;
  }


  //apretat moltllarg -- pujar persiana
  if (dobleclick == 0 && duradaApretat > 4000 ) {
    Serial.println("molt Llarg");
    PujarPersiana();
    return;
  }

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
      delay(20);
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
  if (PotenciaActual < 2) {
    PotenciaActual = PercentInicial;
  }

  int Increment = PotenciaActual + 2;
  if (PotenciaActual > 1) {
    Increment = Increment + 7;
  }

  for (int i = PotenciaActual; i <= Increment; i++)
  {
    dimmer.setPower(i);
    delay(30);
    USE_SERIAL.print("Percent: -> ");
    USE_SERIAL.println(i);

    //per millorar el dobleclick
  //  if (digitalRead(PinPolsador) == HIGH) {
    //  BotoApretat();
   // }
  }


}


void AutoApagat() {
  timer1.attach(1200, ApagarDimmer); //600 segons -> 10 minuts
}


void PujarPersiana() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://192.168.10.2/?temps=25&dormitori=1&accio=1");  //Specify request destination
    int httpCode = http.GET();                                  //Send the request

    if (httpCode > 0) { //Check the returning code

      String payload = http.getString();   //Get the request response payload
      //Serial.println(payload);             //Print the response payload

    }

    http.end();   //Close connection

  }
}
