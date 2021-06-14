

// Probar IRZF44N en conmutación para medir la tensión de la pila en vacio y con carga
// sabiendo las dos tensiones y la corriente que circula por el circuito sabré la Resistencia interna de la pila
// Prueba de concepto..

void setup() {
  
}

void loop() {
  // put your main code here, to run repeatedly:


  
 dacWrite(26,255);     // Mosfet en ON ... mido la tensión en carga                 
 delay(4000);       
 dacWrite(26,0);       // Mosfet en OFF ...  mido la tensión en vacio =  V de thevenin
 delay(4000);
 
  
}
