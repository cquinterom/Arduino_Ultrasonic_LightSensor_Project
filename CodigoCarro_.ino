//LIBRERIAS Y OBJETOS
#include <Servo.h>
Servo MiServo;

//DefiniciÃ³n de variables
//Variables para la logica de los motores
/****************LLANTA 2*******************/
const int MotorPin1 = 44; //Conectado a la entrada 2(1A) del h bridge1
const int MotorPin2 = 45; //Conectado a la entrada 7(2A) del h bridge1

/****************LLANTA 1******************/
const int MotorPin3 = 36; //Conectado a la entrada 15 del h bridge1
const int MotorPin4 = 37 ; //Conectado a la entrada 10 del h bridge1


//Pin que permite iniciar todo por medio del bridge
 const int EnablePin = 10; // Conectado a la entrada 1 (1,2 EN) del h bridge1

//Pines para el sensor de ultrasonido
const int UltraPin1= 50;//echo recibe la onda devuelta
const int UltraPin2= 51;//trigger que envia la sonda



//Pines para el switch cambiar modo maestro a esclavo
const int inPin = 47;

//PinLed izquierdo 
const int ledpinI= 13;

//pinLed derecho
const int ledpinD = 54;
const int ledpinDgnd = 55;
 

//Variables de estado para el modo esclavo y maestro
int Estado;
int EstadoAnterior=LOW; //No presionado
int Modo=0; //1 para maestro, 0 para esclavo

//Variables para el ultrasonido
long tiempo=0;
long distancia=0;

//ANGULOS PARA EL SERVO MOTOR
int const FLeft = 0;
int const MRight = 35;
int const Mid = 87;
int const MLeft = 135;
int const FRight = 180;

//Variables para almacenar los valores de distancia
int MidL;
int Midium;
int MidR;

int dirFinal; //dirección final del servo

//Sensores de Luz 
int const L1 = A4;//izquierda
int const L2 = A5;//derecha


//Variables para guardar la luz
int light1=0;
int light2=0;
int light3=0;

/************CODIGO PRINCIPAL*************/
void setup() {
  Serial.begin(9600);
  // Definimos Pines que reciben INPUT
  pinMode(inPin,INPUT);//ESCLAVO MAESTRO
  pinMode(UltraPin1,INPUT);
  //Definimos Pines que solo funcionan como OUTPUT
  pinMode(EnablePin,OUTPUT);
  pinMode(ledpinI,OUTPUT);
  pinMode(ledpinD,OUTPUT);
  /*******LLANTAS********/
  pinMode(MotorPin1,OUTPUT);
  pinMode(MotorPin2,OUTPUT);
  pinMode(MotorPin3,OUTPUT);
  pinMode(MotorPin4,OUTPUT);
  /************************/
  pinMode(UltraPin2,OUTPUT);
  //activamos el h bridge1 y h bridge2
  digitalWrite(EnablePin,HIGH);
  
  //INICIALIZAR Servomotor
  MiServo.attach(9); // solo funciona con el pin 9 o 10
}

void loop() {
  digitalWrite(ledpinI,HIGH);
  digitalWrite(ledpinD,HIGH);
  /********PUSH BUTTON*********/
  Estado = digitalRead(inPin); //Push button presionado = HIGH de lo contrario LOW, estado actual
  if((Estado == HIGH) && (EstadoAnterior ==LOW)){ //Verificamos que hubo un cambio de estado manual de low a high
    Modo = 1 - Modo; 
    delay(20); //Para controlar el debounce por los resortes que genera resultados errÃ¡ticos
  }  
  EstadoAnterior = Estado; //guardamos el estado actual como anterior, asi si se mantiene presionado el boton no vamos a cambiar el modo
  /******************/
  //Serial.println(Modo);
  /********ULTRASONIDO**********/
  distancia = Ultradistancia();
  //Serial.println(distancia);
  /*********SENSORLUZ**********/
 
  light1= analogRead(L1);//IZQ
  delay(10);
  light2= analogRead(L2);//DER

  Serial.print("LUZ IZQUIERDA  ");
  Serial.println(light1);
  Serial.print("LUZ DERECHA  ");
  Serial.println(light2);
  /***************************/ 

  if(Modo==1){ //LÃ³gica del modo maestro
    //EL Maestro harÃ¡ los escaneos necesarios con el fin de definir en que direcciÃ³n ir
    if(distancia > 50){ //Derecha
      MiServo.write(Mid);
      Motor1(1);
      Motor2(1);
      }else{
        if(distancia < 50){
         //Lecturas con los angulos del servo motor para determinar direcciÃ³n
         //detenemos los motores primero
         Motor1(2);
         Motor2(2);
         dirFinal= ServoLectura();
       
         switch(dirFinal){
          case 1:
            Motor2(1); //Por una fracción de segundo arrancamos el motor designado para hacer el cruze
            delay(250);
            Motor2(2);
          break;
          case 2:
            Motor1(1);
            delay(250);
            Motor1(2);
          break;
         }
         delay(250);
         MiServo.write(Mid);//El servo vuelve a su posición inicial
         delay(1000);
        }
      }
  }else{ //LÃ³gica del modo esclavo
    //El esclavo sigue la luz del carro enfrente de el y cruzarÃ¡ dependiendo del cambio de intensidad de luz.
    if(distancia >15){
      int x = Direction(light1,light2);
      switch(x){
        case 1:
         Motor2(2);//detenemos la llanta correspondiente para hacer el cruze, por una fracción de segundo para simular cruzes en tiempo real.
         delay(250);
         Motor2(1);
        break;
        case 2:
         Motor1(2);
         delay(250);
         Motor1(1);
        break;
      }
    }else{
      if(distancia <8){//detener todo  para prevenir el choque.
        Motor1(2);
        Motor2(2);
      }else{
        Motor1(1);
        Motor2(1);
      }
    }
 }
}



/******************FUNCIONES************************/
int Direction(int l1,int l2){//Determinar la luz mas potente para indicar a donde debemos cruzar
  int dir = 1;
  if(l1<l2){
    dir = 2;
  }
  return dir;
}

int ServoLectura(){ // HACE 3 LECTURAS, SELECCIONA LA MAYOR Y Se dirige en esa dirección
  MiServo.write(MLeft);
  MidL = Ultradistancia();
  delay(1250);
  MiServo.write(MRight);
  MidR = Ultradistancia();
  delay(1250);

  int indice = 1;
  int Mayor = MidL;
  if(Mayor < MidR){
    Mayor = MidR;
    indice = 2;
  }

  return indice;
}

int Ultradistancia(){
  digitalWrite(UltraPin2,LOW); //para estabilizar
  delay(5);
  digitalWrite(UltraPin2,HIGH); //Envio del pulso
  delay(10);
  digitalWrite(UltraPin2,LOW);
  
  tiempo = pulseIn(UltraPin1,HIGH); //mide la longitud del pulso, recibe los pulsos entrantes
  distancia = int(tiempo*10/292/2);//CONVERSION
  return distancia;
}

void Motor1(int opcion){//1 arranca y 2 se detiene
  switch(opcion){
    case 1:
    digitalWrite(MotorPin1,LOW);
    digitalWrite(MotorPin2,HIGH);
    break;
    case 2:
    digitalWrite(MotorPin1,LOW);
    digitalWrite(MotorPin2,LOW);
    break;
  }
}
void Motor2(int opcion){
    switch(opcion){
    case 1:
    digitalWrite(MotorPin3,LOW);
    digitalWrite(MotorPin4,HIGH);
    break;
    case 2:
    digitalWrite(MotorPin3,LOW);
    digitalWrite(MotorPin4,LOW);
    break;
    }


     
}



