#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

IPAddress    apIP(192, 168, 4, 1);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1
                                 
/* This are the WiFi access point settings. Update them to your likin */
const char *ssid = "ESP8266";
const char *password = "12345678";

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);

// Inicio Configuraciòn de puertos
const int pin_mechero = D0; // Se configura el pin de activacion de mechero
const int pin_res_rims = D1; // Se configura el pin de la resistencia electrica (sistema rims)
const int pin_recirculado = D2; // Se configura el pin del motor de bomba recirculadora
const int pin_alarma  = D5; // Se configura el pin de alarma
const int pin_DS18B20_hlt = D6; // Se configura el pin de lectura de temperaturas hlt
const int pin_DS18B20_mlt = D7; // Se configura el pin de lecturade temperaturas mlt
const int pin_led_rojo = D8; // Se configura el pin de led rojo fijo cuando esta encendido y parpadea si una etapa terminó.
const String etapas[] = {"esperando inicio de proceso", "calentando agua inicial", "primer escalon", "segundo escalón", "tercer escalón", "lavado del grano", "hirviendo el mosto"};
// Fin Configuración de puertos

// Inicio Instancia a las clases OneWire y DallasTemperature
OneWire oneWireObjeto1(pin_DS18B20_hlt);
OneWire oneWireObjeto2(pin_DS18B20_mlt);
DallasTemperature sensor_hlt(&oneWireObjeto1);
DallasTemperature sensor_mlt(&oneWireObjeto1);

//variables de control
int etapa = 0;
bool mechero_apagado = true;
bool res_rims_apagada = true;
bool parpadeo_led = false;
int temperatura_hlt = 0;
int temperatura_mlt = 0;
long tiempo_inicial = 0;
long tiempo_lectura_anterior = millis();

// Variables del Formulario
int tim = 73;
int tl = 78;
int tpe = 65;
int dpe = 60;
int tse = 0;
int dse = 0;
int tte = 0;
int dte = 0;
int te = 60;
int tpa = 0;
int tsa = 60;
int tta = 0;

String html;
long segundosalreset = millis() / 1000;
long minutosalreset = segundosalreset / 60;

void leertemperaturas(){
  sensor_hlt.requestTemperatures();
  temperatura_hlt=sensor_hlt.getTempCByIndex(0);
  sensor_mlt.requestTemperatures();
  temperatura_hlt=sensor_hlt.getTempCByIndex(0);
  }
 
void handleRoot() {
if (server.args()>0)
{
 tim = server.arg("tim").toInt();
 tl = server.arg("tl").toInt();
 tpe = server.arg("tpe").toInt();
 dpe = server.arg("dpe").toInt();
 tse = server.arg("tse").toInt();
 dse = server.arg("dse").toInt();
 tte = server.arg("tte").toInt();
 dte = server.arg("dte").toInt();
 te = server.arg("te").toInt();
 tpa = server.arg("tpa").toInt();
 tsa = server.arg("tsa").toInt();
 tta = server.arg("tta").toInt();
}

// Build an HTML page to display on the web-server root address
html = "<!DOCTYPE html>\
<html lang='es'>\
  <head>\
    <meta charset='utf-8'>\
    <meta http-equiv='Refresh' content='30'>\
    <title>Cocina cervecera Automatizada....</title>\
  </head>\
  <body>\
  <div style='float: left; width:50%; background-color:#bfff00;'>\
  <form>\
   <fieldset>\
    <legend>Configuración del HLT</legend>\
    <p>\
      <label for='tim'>Temp. Inicio Maceración</label>\
      <input type='number' name='tim' id='tim' value='";
html = html + String(tim) + "' />";
html = html + "    </p>\
    <p>\
      <label for='tl'>Temp. Lavado</label>\
      <input type='number' name='tl' id='tl' value='";
html = html + String(tl);
html = html + "' />\
    </p>\
   </fieldset>\
   <fieldset>\
    <legend>Configuración del MLT</legend>\
     <br>\
    <fieldset>\
    <legend>Primer Escalón</legend>  \
    <p>\
      <label for='tpe'>Temperatura</label>\
      <input type='number' name='tpe' id='tpe' value='";
html = html + String(tpe);
html = html + "' />\
    </p>\
    <p>\
      <label for='dpe'>Duración</label>\
      <input type='number' name='dpe' id='dpe' value='";
html = html + String(dpe);
html = html + "' />\
    </p>\
    </fieldset>\
     <br>\
    <fieldset>\
    <legend>Segundo Escalón</legend>\
    <p>\
      <label for='tse'>Temperatura</label>\
      <input type='number' name='tse' id='tse' value='";
html = html + String(tse);
html = html + "' />\
    </p>\
    <p>\
      <label for='dse'>Duración</label>\
      <input type='number' name='dse' id='dse' value='";
html = html + String(dse);
html = html + "' />\
    </p>\
    </fieldset>\
    <br>\
    <fieldset>\
    <legend>Tercer Escalón</legend>  \
    <p>\
      <label for='tte'>Temperatura</label>\
      <input type='number' name='tte' id='tte' value='";
html = html + String(tte);
html = html + "' />\
    </p>\
    <p>\
      <label for='dte'>Duración</label>\
      <input type='number' name='dte' id='dte' value='";
html = html + String(dte);
html = html + "' />\
    </p>\
    </fieldset>\
   </fieldset>\
   <fieldset>\
    <legend>Configuración del Bk</legend>\
    <p>\
      <label for='te'>Tiempo de ebullición</label>\
      <input type='number' name='te' id='te' value='";
html = html + String(te);
html = html + "' />\
    </p>\
   <fieldset>\
    <legend>Adiciones de Lúpúlo</legend>\
    <p>\
      <label for='tpa'>Tiempo de la adición</label>\
      <input type='number' name='tpa' id='tpa' value='";
html = html + String(tpa);
html = html + "' />\
    </p>\
    <p>\
      <label for='tsa'>Tiempo de la adición</label>\
      <input type='number' name='tsa' id='tsa' value='";
html = html + String(tsa);
html = html + "' />\
    </p>\
    <p>\
      <label for='tta'>Tiempo de la adición</label>\
      <input type='number' name='tta' id='tta' value='";
html = html + String(tta);
html = html + "' />\
    </p>\
   </fieldset>\
   </fieldset>\
   <center><input type='submit' value='Guardar'></center></br>\
  </form>\
  </div>\
  <div style='float: right; width:50%; background-color:#f55000;'><h1>Tiempo total: " + String(((millis() / 1000)/60) - minutosalreset) + " minutos.</h1></div>\
  <div style='float: right; width:50%; background-color:#ff8000;'>";
  if(etapa == 0)
  {
    html = html + "<form action='/inical'>\
   <fieldset>\
    <legend>Estado del proceso actual</legend>";
    html = html + "<h2>Listo para iniciar el calentamiento</h2><p>Asegurese de encender el piloto del mechero principal y luego precione el botón para dar inicio al proceso de calentamiento del hlt</p>";
    html = html + "<input type='submit' value='Iniciar Calentamiento'>\
    </fieldset>\
    </form>";
  }
  if(etapa == 1)
  {
    html = html + "<form action='/inimac'>\
   <fieldset>\
    <legend>Estado del proceso actual</legend>";
    html = html + "<h2>Calentando agua en HLT</h2><p>Asegurese de que elmechero principal encendió correctamente, el sistema le avisará con una señal sonora cuando la temperatura del hlt haya alcanzado los <b>"+tim+"</b> grados según se ha configurado en el formulario principal.</p><h2>Temperatura actual "+temperatura_hlt+"°C</h2><h2>Tiempo de calentamiento "+tiempo_transcurrido(tiempo_inicial)+"</h2>";
    if(temperatura_hlt>=tim)
    {
      html = html + "<input type='submit' value='Iniciar Maceración'>";
      parpadeo_led = true;
    }
     html = html + "</fieldset>\
    </form>";
  }
  if (etapa == 2 || etapa == 3 || etapa == 4)
  {
    html = html + "<form action='/iniherb'>\
   <fieldset>\
    <legend>Estado del proceso actual</legend>";
    html = html + "<h2>Macerando en MLT en ";
    html = html + etapas[etapa];
    html = html +".</h2><p>Asegurese de que la resirculación continua esté funcionando, el sistema le avisará con una señal sonora de alarma cuando el proceso termine. \
    La temperatura configurada para esta etapa es de <b>"+tpe+"</b> grados °C.</p><h2>Temperatura actual "+temperatura_mlt+"°C</h2><h2>Tiempo de calentamiento\
    "+tiempo_transcurrido(tiempo_inicial)+"</h2>";
     html = html + "</fieldset>\
    </form>";
  }
  if (etapa == 5)
  {
    ...
  }
  html = html + "</div>\
  </body>\
</html>";
server.send ( 200, "text/html", html );
}

void alarma(int piezoPin) //Funcion para hacer sonar una alarma tipo despertador
{
      delay(500);
      tone(piezoPin, 2093, 62);
      delay(125);
      tone(piezoPin, 2093, 62);
      delay(125);
      tone(piezoPin, 2093, 62);
      delay(125);
      tone(piezoPin, 2093, 62);
      delay(125);
}

String tiempo_transcurrido(long ti)
{
    long totalSeconds = ((millis()-ti)/1000);
    int hours        = totalSeconds / 3600;
    int minutes      = totalSeconds % 3600 / 60;
    int seconds      = totalSeconds % 60;
    String r="";
    if (hours<10)
    {
      r="0";
      }
    r=r+String(hours)+":";
    if (minutes<10)
    {
      r=r+"0";
      }
    r=r+String(minutes)+":";
    if (seconds<10)
    {
      r=r+"0";
      }
    r=r+String(seconds);
    return r;
}

void handleInical(){
  etapa = 1;
  tiempo_inicial = millis();
  digitalWrite(pin_mechero, LOW);
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handleInimac(){
  etapa = 2;
  tiempo_inicial = millis();
  digitalWrite(pin_mechero, HIGH);  
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}
//********************************************** void setup  ********************************************//
void setup() {
 
  pinMode ( pin_mechero, OUTPUT );
  digitalWrite ( pin_mechero, HIGH );
  pinMode (pin_recirculado,OUTPUT);
  digitalWrite ( pin_recirculado, HIGH );
  pinMode (pin_res_rims,OUTPUT);
  digitalWrite ( pin_res_rims, HIGH );
  pinMode ( pin_led_rojo, OUTPUT );
  digitalWrite ( pin_led_rojo, HIGH );
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00
 
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password, 4);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 
  server.on ( "/", handleRoot );
  server.on ( "/inical", handleInical);//Proceso iniciador de calentamiento, pasa la variable estado de 0 -> 1
  server.onNotFound ( handleNotFound );
  //sensor_hlt.begin();
  //sensor_mlt.begin();
  server.begin();
  Serial.println("HTTP server started");

}

//********************************************** void loop  ********************************************//

void loop() {
  server.handleClient();
  if (millis()>tiempo_lectura_anterior+1000)
  {
    leertemperaturas();
    tiempo_lectura_anterior = millis();
    if (parpadeo_led) 
    {
      if (digitalRead(pin_led_rojo)==HIGH)
      {
        digitalWrite(pin_led_rojo, LOW);
      }
      else
      {
        digitalWrite(pin_led_rojo, HIGH);
      }
    }
  }
  
  switch (etapa) {
  case 1: // etapa de calentamiento
  if(temperatura_hlt>=tim and !mechero_apagado)
  {
    digitalWrite ( pin_mechero, HIGH);
    mechero_apagado=true;
    alarma(pin_alarma);
    parpadeo_led=true;
  }
  if(temperatura_hlt<tim-1 and mechero_apagado) // el (-1) es un margen de temperatura inferior para que varie entre tim y (tim -1)
  {
    digitalWrite ( pin_mechero, LOW );
    mechero_apagado=false;
    tone(pin_alarma, 2093, 100);
    parpadeo_led=false;
  }
  break; 
  case 2: // primer escalón
  if(millis()>=(tiempo_inicial+dpe))
  {etapa=3;
  tone(pin_alarma, 2093, 500);
  }
  else
  {
  if(temperatura_mlt>=tpe and  !res_rims_apagada)
  {
    digitalWrite ( pin_res_rims, HIGH);
    res_rims_apagada=true;
    tone(pin_alarma, 2093, 100);  
  }
  if(temperatura_mlt<tpe-1 and res_rims_apagada) // el (-1) es un margen de temperatura inferior para que varie entre tpe y (tpe -1)
  {
    digitalWrite ( pin_res_rims, LOW );
    res_rims_apagada=false;
    tone(pin_alarma, 2093, 100);
  }
  break;
  }
  case 3: // segundo escalón
  if(millis()>=(tiempo_inicial+dpe+dse))
  {etapa=4;
  tone(pin_alarma, 2093, 500);
  }
  else
  {
  if(temperatura_mlt>=tse and !res_rims_apagada)
  {
    digitalWrite ( pin_res_rims, HIGH);
    res_rims_apagada=true;
    tone(pin_alarma, 2093, 100);  
  }
  if(temperatura_mlt<tse-1 and res_rims_apagada) // el (-1) es un margen de temperatura inferior para que varie entre tse y (tse -1)
  {
    digitalWrite ( pin_res_rims, LOW );
    res_rims_apagada=false;
    tone(pin_alarma, 2093, 100);
  }
  break;
  }  
  case 4: // tercer escalón
  if(millis()>=(tiempo_inicial+dpe+dse+dte))
  { etapa=5;
    alarma(pin_alarma);
  }
  else
  {
  if(temperatura_mlt>=tse and !res_rims_apagada)
  {
    digitalWrite ( pin_res_rims, HIGH);
    res_rims_apagada=true;
    tone(pin_alarma, 2093, 100);  
  }
  if(temperatura_mlt<tse-1 and res_rims_apagada) // el (-1) es un margen de temperatura inferior para que varie entre tse y (tse -1)
  {  digitalWrite ( pin_res_rims, LOW );
     res_rims_apagada=false;
     tone(pin_alarma, 2093, 100);
  }
  break;
  }  
 
}
}
