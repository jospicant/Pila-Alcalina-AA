

byte a;             //
int  b;
float c;
double d;
long   i;

uint8_t e;
uint16_t f;
uint32_t g;
uint64_t h;

unsigned long j;

void setup() {

  
  Serial.begin(115200);
  delay(100);
  Serial.println("hola");
  Serial.println(sizeof(a));
  Serial.println(sizeof(b));
  Serial.println(sizeof(c));
  Serial.println(sizeof(d));
  Serial.println(sizeof(i));
  Serial.println(sizeof(long));
  Serial.println("**************");
  
  Serial.println(sizeof(e));
  Serial.println(sizeof(f));
  Serial.println(sizeof(g));
  Serial.println(sizeof(h));

  Serial.println(sizeof(j));
  
}

void loop() {
    

}
