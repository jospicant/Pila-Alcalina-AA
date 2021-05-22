

double t_inicio,t_actual;
double t1,t2;

bool estadoLed1=0;
bool estadoLed2=0;


void setup() {

    Serial.begin(115200);
    
    //GPIOs *****************
    pinMode(21,OUTPUT);
    pinMode(12,OUTPUT);
 
    t_inicio=millis();              // inicio cronómetro para el envio periodico a ThingSpeak

}
   
void loop() {

    bool t_1sg,t_5sg;
    
    t_1sg = hanPasadoXms(1000,&t_inicio,&t_actual);                     //Miro si ha pasado un segundo
    if(t_1sg){                                                         //Si ha pasado 1 segundo

      estadoLed1=!estadoLed1;
      Serial.println(estadoLed1);
      digitalWrite(21,estadoLed1);
      
    }
    t_5sg =hanPasadoXms(5000,&t1,&t2);
    if(t_5sg){
      estadoLed2=!estadoLed2;
      Serial.println(estadoLed2);
      digitalWrite(12,estadoLed2);
    }
       
}


//***********************************************************************************************************************
//Calcular si han pasado x milisegundos = t_muestreo
//respecto a una referencia d tiempo incial tomada = t_inicio
//Le pasaré también la variable de tiempo Actual
//***********************************************************************************************************************
boolean hanPasadoXms(int t_muestreo, double *t_inicio, double *t_actual){   //a los punteros paso las direcciones con &variable
  
   *t_actual=millis();                            //tomo el tiempo en este mismo instante
   if(*t_actual-*t_inicio >= t_muestreo){   //Si han pasado los 20 sg para poder enviar a ThingSpeak
    *t_inicio=millis();
    return true;
    }else return false;
}
//************************************************************************************************************************
