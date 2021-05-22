
//*******************************************
// Medida de tensión de la pila AA con ESP32 WROOM V2
//*******************************************

#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//oled  pines SCL --> GPIO22  SDA -->GPIO21 estos se deben usar para el OLED
//La librería estandar usamos 21 y 22. Dejo esos por defecto por lo cual GPIO21 y 22 no los puedo usar para otra cosa


#define LED_PILA_AGOTADA      4       // (LED ROJO)        Indicaremos con un led en el GPIO4 que la pila se ha agotado y ha terminado la adquisición de los datos
#define LED_ENVIANDO_FICHERO  23      // (LED AZUL)        Led para indicar que se están enviando datos 
#define LED_BORRANDO_FICHERO  2      // (LED VERDE)       Led indicar que se están borrando los datos

#define BOTON_INICIO_PARO     5       // (BOTON IZQUIERDO) Al pulsar iniciamos la captura de los datos o paramos la captura de datos
#define BOTON_ENVIAR_FICHERO  18      // (BOTÓN CENTRAL)   Enviar los datos del fichero por el puerto Serie o por bluetooth
#define BOTON_BORRAR_FICHERO  19      // (BOTÓN DERECHO)   Si queremos borrar los datos del fichero antes de iniciar la captura de datos

#define ficheroDeDatos      "/datosPila.txt"  //Nombre del fichero donde guardaremos los datos

//******************************************************************************************
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//********************************************************************************************



//************ datos de mi Wifi *************************
const char* ssid_1     = "Your ssid";
const char* password   = "Your password";
boolean conexionWifi   =  false;
//*****************************************************

//********** datos del cliente Wifi para conectar a ThingSpeak ******************************

WiFiClient clienteThingSpeak;   // Objeto Cliente de ThingSpeak
boolean conexionThingSpeak;

char direccionThingSpeak[] = "api.thingspeak.com";
String APIKey = "Your ThingSpeak API KEY ";
const int horaEn_ms = 3600*1000;               // 1 hora son 3600000 ms
uint64_t tiempoActual,tiempoAnterior;
String datosTS;

//*******************************************************************************************


const byte    puertoADC = 35;
const double    Rdescarga = 10.8;              // 10.8 ohmios como resistencia de descarga
const int     muestreo_1sg =1 * 1000;       // muestreo cada segundo
const double  tensionMinima = 0.6;         // Tensión a partir de la cual considaremos que la pila está descargada y dejaremos de tomar medidas y enviar a Thing Speak

int        datoPila;
float     tensionPila=0;
float     corriente=0;
float     potencia=0;
float     Julios=0;
float     cargaAcumulada=0;
float     cargaMuestreada=0;
float     energia=0;
float     miliWatiosHora=0;
String     datosFichero;
int        numeroDeMuestras=0;


// parado = 0 muestreando =1 enviando = 2 borrando = 3
enum estadoActual {          
  parado,
  muestreando,
  enviando,
  borrando
};

estadoActual modo = parado;             // indicar los tres posibles modos de funcionamiento

//**********************************************************************************************************
//**********************************************************************************************************
//**********************************************************************************************************

void setup() {

    
    //GPIOs *****************
    pinMode(LED_PILA_AGOTADA,OUTPUT);
    pinMode(LED_ENVIANDO_FICHERO,OUTPUT);
    pinMode(LED_BORRANDO_FICHERO,OUTPUT);
    pinMode(BOTON_INICIO_PARO,INPUT_PULLUP);
    pinMode(BOTON_ENVIAR_FICHERO,INPUT_PULLUP);
    pinMode(BOTON_BORRAR_FICHERO,INPUT_PULLUP);

    //chequeo de los LEDs
    pruebaLED(LED_PILA_AGOTADA); pruebaLED(LED_ENVIANDO_FICHERO); pruebaLED(LED_BORRANDO_FICHERO);
    apagaLED(LED_PILA_AGOTADA); apagaLED(LED_ENVIANDO_FICHERO); apagaLED(LED_BORRANDO_FICHERO);
       
    //Serial ****************
    Serial.begin(115200);   //iniciar puerto serie
    while(!Serial);       // espero a abrir el monitor Serial. No me funciona ???

    //************ OLED **************************************

    chequeaOLED();
    plantillaOLED();
    
    //********************************************************
    
    delay(300);
    
    //formateaSPIFFS();
    //iniciarFichero(ficheroDeDatos);
       
    conexionWifi=conectarAWifi();
    conexionThingSpeak=conexionConThingSpeak();
    
    tiempoAnterior=millis();              // inicio cronómetro para el envio periodico a ThingSpeak

    Serial.println(".............");Serial.println("Modo: PARADO ");Serial.println(".............");

    Serial.println(" Botón1: INICIO/PARO  Botón2: ENVIAR FICHERO   Botón3: BORRAR Y REINICIAR FICHERO ");

}
   
void loop() {


bool boton1 = digitalRead(BOTON_INICIO_PARO);    
bool boton2 = digitalRead(BOTON_ENVIAR_FICHERO);
bool boton3 = digitalRead(BOTON_BORRAR_FICHERO);

    switch (modo){
      
      case parado:

            if (!boton1){ //si estoy parado y pulso iniciaré muestreo
              modo = muestreando; Serial.println(".................."); Serial.println("Modo: MUESTREANDO "); Serial.println(".................."); delay(300); plantillaOLED();   break;
            }
            
            if(!boton2) { modo = enviando; Serial.println("..................."); Serial.println("Modo: ENVIANDO FICHERO "); Serial.println("...................");  delay(300); plantillaOLED();  break; }
            if(!boton3) { modo=borrando; Serial.println("..................."); Serial.println("Modo: BORRANDO FICHERO "); Serial.println("...................");  delay(300);  plantillaOLED(); break; }
            
            break;
            
      case muestreando:

           if (!boton1){ // Si estoy muestreando y pulso --> pararé
              modo = parado; Serial.println("............."); Serial.println("Modo: PARADO "); Serial.println("............."); delay(300); plantillaOLED(); break;          
            }
            
            if (!boton2){ modo = enviando; Serial.println("..................."); Serial.println("Modo: ENVIANDO FICHERO "); Serial.println("..................."); delay(300); plantillaOLED(); break;}
            if (!boton3){ modo = borrando; Serial.println("..................."); Serial.println("Modo: BORRANDO FICHERO "); Serial.println("..................."); delay(300); plantillaOLED();  break;}
            
            tomaMuestras();  // mientras esté en este modo tomaré muestra...
            
            break;
            
      case enviando:

            digitalWrite(LED_ENVIANDO_FICHERO,HIGH); leerFichero(ficheroDeDatos); modo = parado; digitalWrite(LED_ENVIANDO_FICHERO,LOW); //Hago la lectura, cambio a modo parado y envio por Serial
            plantillaOLED();
            Serial.println("............."); Serial.println("Modo: PARADO "); Serial.println(".............");
                       
            break;
            
      case borrando:
      
           static byte n=1;                                                       //cuenta las veces que he entrado en el estado de borrando
        
           if( n == 1){
                 digitalWrite(LED_ENVIANDO_FICHERO,HIGH);
                //Si entro a borrado, primero advierto y si confirmo con botón de borrado una segunda vez entonces borraré
                Serial.println(" OJO! vas a borrar el fichero de datos con los siguientes datos almacenados: ");
                Serial.println(".............................................................................");
                leerFichero(ficheroDeDatos);
                Serial.println("Si vuelves a pulsar el botón de borrado se borrarán los datos ");
               
                Serial.println("............."); Serial.println("Modo: BORRANDO "); Serial.println(".............");
                digitalWrite(LED_ENVIANDO_FICHERO,LOW);
                n++;
                modo = borrando;
                plantillaOLED();
                
                break;
            } 

            
            if (!boton3){ n++; modo = borrando;  delay(300); plantillaOLED(); }                      //Si estando en modo borrando vuelvo a pulsar borrado --> borraré y pasaré a modo parado
            if (!boton1){ n=1; modo = parado;  delay(300);   plantillaOLED(); break; }        //Si estando en modo borrando pulso iniciar/parar     --> resetearé cuenta y  voy a mod parado
            if (!boton2){ n=1; modo = enviando;  delay(300); plantillaOLED(); break; }      //Si estando en modo borrando pulso enviar  --> reinicio y cambio a modo enviar

            if ( n == 3 ) {
                //formateo, borro todo, creo el fichero de nuevo vacio y vuelvo al modo PARADO

                digitalWrite(LED_BORRANDO_FICHERO,HIGH);
                
                Serial.println("Formateando sistema de Ficheros, espera...");
                
                formateaSPIFFS();
                iniciarFichero(ficheroDeDatos);
                leerFichero(ficheroDeDatos);
                
                Serial.println("Fichero reiniciado y listo para almacenar datos de cero ");
                
                Serial.println(".............");Serial.println("Modo: PARADO ");Serial.println("............."); 
                
                digitalWrite(LED_BORRANDO_FICHERO,LOW);
                
                n=1;                //reseteo el contador de veces que he entrado
                modo = parado;
                plantillaOLED();
                break;
                
            } 
            
            break;
            
      default:
               modo = parado ; 
               Serial.println(".............");Serial.println("Modo: PARADO ");Serial.println("............."); 
               break;
           
    }

 
}

//*************************************************************************************************************


void leerFichero(String f){

    if(!SPIFFS.begin(true)){ 
       Serial.println("Un error ha ocurrido al montar el sistema de ficheros");
       return;
       }
      
   File file = SPIFFS.open(f,"r");   
   Serial.println(file.name());
   
   if(!file) Serial.println("Fallo al abrir el fichero ");
   else{
        Serial.println("El contenido del fichero es:");
        while(file.available()){
        Serial.write(file.read());
        }
        Serial.println();
        Serial.print("El fichero ocupa: ");Serial.print(file.size());Serial.println(" bytes");
        
       
       }
    file.close();
    
    
}

void escribirFichero(String f, String dato){

    if(!SPIFFS.begin(true)){ 
       Serial.println("Un error ha ocurrido al montar el sistema de ficheros");
       return;
       }
      
   File file = SPIFFS.open(f,"a");   
   Serial.println(file.name());
   
   if(!file) Serial.println("Fallo al abrir el fichero ");
   else{
        file.println(dato);
        }
   
   file.close();
   
    
}

void iniciarFichero(String f){

String dato = "n_muestra   ;  Tensión ( V) ; Corriente (mA) ;  Potencia (mW ) ; Carga (mAh ) ; Julios ( Joules ); (RLoad = 10.8 Ohm ) ; T muestreo = 60 sg";

    if(!SPIFFS.begin(true)){ 
       Serial.println("Un error ha ocurrido al montar el sistema de ficheros");
       return;
       }
      
   File file = SPIFFS.open(f,"a");   
   Serial.println(file.name());
   
   if(!file) Serial.println("Fallo al abrir el fichero ");
   else{
        file.println(dato);
        }
   
   file.close();
   
    
}

void tamanyoDelSistemaDeFicheros(){
 
  if(!SPIFFS.begin(true)){ 
       Serial.println("Un error ha ocurrido al montar el sistema de ficheros");
       return;
       }
  Serial.print("El sistema de ficheros tiene un tamaño reservado de: ");
  Serial.print(SPIFFS.totalBytes());
  Serial.println(" bytes ");
  Serial.print("En uso hay un total de: ");
  Serial.print(SPIFFS.usedBytes());
  Serial.println(" bytes ");
  Serial.println();
  
}

boolean conectarAWifi(){
  static byte intentos=0;
  
// Connect to Wi-Fi network with SSID and password
  Serial.println("************************************************");
  Serial.print("Conectando a: ");
  Serial.println(ssid_1);
  WiFi.begin(ssid_1, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(intentos >=100){
      Serial.println("No ha podido conectarse por Wifi... ");
      return false;
    }
    intentos++;
    Serial.println(intentos); 
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

 //**********************************Conexion a ThingSpeak*****************************************
boolean conexionConThingSpeak(){     
   if (clienteThingSpeak.connect(direccionThingSpeak,80)) {
   return (true);
   }else
   return (false);
}

//************ Configuración del paquete a enviar a la plataforma ThingSpeak *****************************************************************

void EnviarDatosThingSpeak(String datos){
 
    // Establecemos la conexión con el cliente
    if (clienteThingSpeak.connect(direccionThingSpeak,80)) {           
    
        // Línea de comienzo del mensaje (Método URI versionprotocolo)
        clienteThingSpeak.println("POST /update HTTP/1.1");               //  api.thingspeak.com/update      Envio Texto plano
 
        // Campos de cabecera
        clienteThingSpeak.print("Host: ");
        clienteThingSpeak.println(direccionThingSpeak);
        clienteThingSpeak.println("Connection: close");
        clienteThingSpeak.println("X-THINGSPEAKAPIKEY: " + APIKey);
        clienteThingSpeak.println("Content-Type: application/x-www-form-urlencoded");
        clienteThingSpeak.print("Content-Length: ");
        clienteThingSpeak.println(datos.length());
        clienteThingSpeak.println(); // Esta línea vacía indica que hemos terminado con los campos de cabecera
     
        // El cuerpo del mensaje, los datos
        clienteThingSpeak.print(datos);
 
        // Comprobamos si se ha establecido conexión con Thinspeak
        if (clienteThingSpeak.connected()) {
          Serial.println("Conectado a Thingspeak...");
        }else{
          Serial.println("No Conectado a Thingspeak...");
        }
      }
}

//*****************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************
void leerDatoDeLaPila(){                          
  
    datoPila=         analogRead(puertoADC);
    tensionPila =     3.3*datoPila/4095;                           // En voltios
    corriente =       tensionPila/Rdescarga*1000;                  // Datos en mA  miliamperios
    potencia =        tensionPila*corriente;                       // Datos en mW  miliwatios
    //Serial.println(tensionPila); Serial.println(corriente);
  
}

void medirCargaEntregada(){
  
    cargaMuestreada=   (corriente*muestreo_1sg)/horaEn_ms;         // Área medida  corriente * 1 sg y pasada  a mAh 
    cargaAcumulada =  cargaAcumulada + cargaMuestreada;            // Sumatorio de todas las cargas consumidas ( la suma de todas las Áreas ) --> medida en mAh (miliamperios hora )
    miliWatiosHora =  (potencia*muestreo_1sg)/horaEn_ms;           // Área donde mido potencia* 1sg --> y paso a mWh
    energia        =  energia + miliWatiosHora;                     // acumulo los mWh.    1KWh =3600000 Julios
    //energia        =  energia / 1000000;                            // convierto a KWh
    //energia        =  energia * 3600000;                           // convierto a Julios   ( 1KWh = 3600000 Julios )
     Julios = energia * 3.6;                                        //directamente el resumen a Julios
}
//*********************************************************************************************************************************************************************************
//********************************************************************************************************************************************************************************


String recopilarDatosParaThingSpeak(){

    String datosThingSpeak= "field1=" + String(tensionPila) + "&field2=" + String(corriente)+ "&field3=" +String(potencia)+ "&field4=" + String(cargaAcumulada) + "&field5=" + String(Julios); 
    Serial.println(datosThingSpeak);
    return datosThingSpeak;
}

//*******************************************************************************************************************************************
//Guardo los datos en un fichero que luego transmitiré por el puerto Serie...
//las mediciones las voy añadiendo por filas con los datos separados por ";" --> exportable a excel fácil  
//******************************************************************************************************************************************
String recopilarDatosParaFicheroSPIFFS(){

    String datosFichero= String(numeroDeMuestras) + ";" + String(tensionPila) + ";" + String(corriente) + ";" + String(potencia) + ";" + String(cargaAcumulada) + ";" + String(Julios); 
    Serial.println(datosFichero);
    return datosFichero;
}


//***********************************************************************************************************************
//Calcular si han pasado x milisegundos = t_muestreo
//respecto a una referencia d tiempo incial tomada = t_inicio
//Le pasaré también la variable de tiempo Actual
//***********************************************************************************************************************
boolean hanPasadoXms(int t_muestreo, uint64_t *t_inicio, uint64_t *t_actual){   //a los punteros paso las direcciones con &variable
  
   *t_actual=millis();                            //tomo el tiempo en este mismo instante
   if(*t_actual-*t_inicio >= t_muestreo){   //Si han pasado los 20 sg para poder enviar a ThingSpeak
    *t_inicio=millis();
    return true;
    }else return false;
}
//************************************************************************************************************************

void formateaSPIFFS(){

  bool formateado = SPIFFS.format();
  if ( formateado ) {
    Serial.println("SPIFFS se ha formateado correctamente");
  } else {
    Serial.println("Error al formatear el sistema de ficheros");
  }

}

boolean montarSistemaDeFicherosSPIFFS(){

  if(!SPIFFS.begin(true)){ 
       Serial.println("Un error ha ocurrido al montar el sistema de ficheros");
       return false;
       }
  else 
    return true;

}

void pruebaLED(int led)
{
    digitalWrite(led,HIGH);
    delay(500);
    digitalWrite(led,LOW);
    delay(500);
    digitalWrite(led,HIGH);
    delay(500);
    digitalWrite(led,LOW);
    
}

void apagaLED(int led){
   digitalWrite(led,LOW);  
}

void pruebaBOTON(int boton, int led){
   bool in=digitalRead(boton);
   digitalWrite(led,in);
   delay(200);
}

void tomaMuestras(){

  static byte numeroDeMuestreos=0;
  bool t_1sg;
  
  t_1sg = hanPasadoXms(muestreo_1sg,&tiempoAnterior,&tiempoActual);    //Miro si ha pasado un segundo
   
    if(t_1sg){                                                      //Si ha pasado 1 segundo
      
      leerDatoDeLaPila();                                            //leo los datos de la pila cada X tiempo de muestreo. 1 sg para ThingSpeak
      
        if( tensionPila > tensionMinima){                           // Si tengo una tensión  mayor de 0.8 V continuo midiendo y enviado a ThngSpeak
            digitalWrite(LED_PILA_AGOTADA,LOW);
            medirCargaEntregada();                                  //Mido la carga entregada cada segundo y la voy acumulando. ( integral/Área cada segundo)
            numeroDeMuestreos++;
            
              if(numeroDeMuestreos >= 60) {                         //Solo preparo los datos para thingSpeak si han pasado 60 sg ( 60 muestreos de 1 sg)
                numeroDeMuestras++;
                datosTS=recopilarDatosParaThingSpeak();            //preparo los datos por si hay que enviarlos a ThingSpeak
                datosFichero = recopilarDatosParaFicheroSPIFFS();  //Recopilar datos para fichero SPIFFS
                escribirFichero(ficheroDeDatos,datosFichero);      //Escribo los datos en el fichero
                EnviarDatosThingSpeak(datosTS);                    //Envio los datos a mi canal de ThingSpeak (han pasado 20sg)
                numeroDeMuestreos = 0;                             //reseteo la cuenta de los muestreos
              }   
        }else{
          digitalWrite(LED_PILA_AGOTADA,HIGH);                     //Cuando la tensión de la pila baje o iguale los 0.6 se encenderá el led indicando PILA AGOTADA y final de adquisición de los datos
          modo = parado;                                           // paro de muestrear
          plantillaOLED();
        }
      plantillaOLED();
    }
  
}

void chequeaOLED(){
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }
}

void plantillaOLED(){

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0); display.println("Mode: "); display.setCursor(50,0); convierteModo();
    display.setCursor(0,15);display.println("Volts: ");display.setCursor(50,15);display.println(tensionPila);
    display.setCursor(0,25);display.println("mA: ");display.setCursor(50,25);display.println(corriente);
    display.setCursor(0,40);display.println("mAh: ");display.setCursor(50,40);display.println(cargaAcumulada);
    display.setCursor(0,50);display.println("Joules: ");display.setCursor(50,50);display.println(Julios);
    display.display(); 
}

void convierteModo(){
  if(modo==0)display.println("STOPPED");
  if(modo==1)display.println("SAMPLING");
  if(modo==2)display.println("SENDING FILE");
  if(modo==3)display.println("DELETING * ");
}
