
#define LED 23
#define INICIO 4

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  pinMode(INICIO, INPUT_PULLUP);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {

  static int n=0;          //static para que no vuelva a poner a 0 en el siguente ciclo que solo tome 0 la primera vez
  bool a=digitalRead(INICIO);
  digitalWrite(LED,a);
  
  if(!a){
    digitalWrite(LED,a);
    n++;
    Serial.println(n);
    delay(200); //debounce    
  }
  
   
}
