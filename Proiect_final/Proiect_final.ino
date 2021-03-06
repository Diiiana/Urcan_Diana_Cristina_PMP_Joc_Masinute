#include "avr/interrupt.h"
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// scor si afisare
int scor = 0;
int prev = 0;

// caracter liber
byte empty[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// masinuta utilizatorului
byte car[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x1F, 0x0A};

// masinuta de pe banda cealalta
byte carop[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x0A};

// scor
byte sc[8] = {0x1B, 0x12, 0x0A, 0x1B, 0x00, 0x00, 0x00, 0x00};
byte o[8] = {0x0E, 0x0A, 0x0A, 0x0E, 0x00, 0x00, 0x00, 0x00};
byte r[8] = {0x18, 0x14, 0x18, 0x14, 0x00, 0x00, 0x00, 0x00};

// inimioara - viata utilizatorului
byte hrt[8] = {0x0A, 0x1F, 0x0E, 0x04, 0x00, 0x00, 0x00, 0x00};
byte hrti[8] = {0x00, 0x00, 0x00, 0x00, 0x0A, 0x1F, 0x0E, 0x04};

byte obstacol1[8] = {0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0C, 0x18};

// iesirea potentiometrului
int potPin = A1;

// valoare potentiometru
int currentSpeed = 0; 

// butonul de start/stop
int startPin = 21;
int stopPin = 20;

//butonul de schimbare directie masina
int backPin = 19;
int roadPin = 18;

// jocul a inceput
volatile boolean inGame = false;

// se doreste finalizarea jocului
volatile boolean endingGame = false;

// pozitionarea masinutei utilizatorului
int posX = 0;

// pentru a inapoia cu masina
boolean backwards = false;

// pentru a schimba banda
volatile int banda = 1;

// semafor
int redPin = 11;
int yellowPin = 10;
int greenPin = 9;

// 5s pentru semafor
unsigned long startMillis = 0;
const long interval = 5000;

// marcheaza inceputul jocului pentru setare culoare semafor
boolean starting = true;
// daca semaforul este pornit sau nu
boolean semafor = false;
// daca utilizatorul deplaseaza masina
boolean driving = false;
// daca a fost penalizat deja, in acelasi interval de timp
boolean alreadyPassed = false;
// daca a mai fost rosu
boolean semafor_prec = false;

// numar vieti
int life = 3;

// obstacole
struct obstacol{
  int x;
  int y;
};

obstacol obstacole[5];

// vieti bonus
int heartx=-1, hearty=-1;

// bonusuri 
int coinx=-1, coiny=-1;

// penalizare pentru bonusuri
int appearTime = 0;

// inceputul jocului
unsigned int startTime = 0;

void setup() {
  // contrast lcd
  analogWrite(8, 30);
  lcd.begin(16, 2);
  
  // creare caractere CGROM
  lcd.createChar(0, car);
  lcd.createChar(1, carop);
  lcd.createChar(2, sc);
  lcd.createChar(3, o);
  lcd.createChar(4, r);
  lcd.createChar(5, hrt);
  lcd.createChar(6, empty);
  lcd.createChar(7, hrti);
  
  // led verificare start
  pinMode(13, OUTPUT);
  
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.write(byte(0));

  lcd.setCursor(10, 0);
  lcd.write(byte(1));
  
  lcd.setCursor(6, 0);
  lcd.print("Cars");
  lcd.setCursor(3, 1);
  lcd.print("START GAME");
  delay(1500);

  // setare pini semafor
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  
  cli();
  // butonul de start/stop intrerupe jocul
  pinMode(startPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  // butonul de schimbare directie masina
  pinMode(backPin, INPUT_PULLUP);
  // butonul de schimbare banda
  pinMode(roadPin, INPUT_PULLUP);

  // intreruperile pentru cele 4 butoane
  attachInterrupt(digitalPinToInterrupt(startPin), startGame, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopPin), endGame, FALLING);
  attachInterrupt(digitalPinToInterrupt(backPin), goBack, FALLING);
  attachInterrupt(digitalPinToInterrupt(roadPin), roadChange, FALLING);
  
  sei();
}

void loop()
{ 
  // daca a apasat butonul de start, incepe jocul
  if(inGame){
      gameCore();
  }else{
    // daca a pierdut/a apasat stop, jocul se termina
    if(endingGame){
      // resetare
      life = 0;
      lifeStatus();
      lcd.clear();
    }
  }
}

void gameCore(){
  while(inGame){
      // activare conditie
      isDriving();
      // setare semafor
      semaforFunc();
      // verificare trecere pe rosu
      redPass();
      // masina utilizatorului
      lcd.clear();
      lcd.setCursor(posX, banda);
      lcd.write(byte(0)); 

      // calcul obstacole la pozitii random
      randomSeed(1);
      for(int i=0; i < 3; i++){ 
        int xVal;
        if(obstacole[i].x == 0){
          xVal = 11 + random(2, 10);
          for(int j=0; j <i; j++){
          while(obstacole[j].x == xVal){
          xVal = 16 + random(3, 8);
          }
        }
        obstacole[i].x = xVal;
        obstacole[i].y = random(0, 2);
      }
      }
      // scriere obstacole
      for(int i=0; i < 3; i++){
        obstacole[i].x -= 1;
        if(obstacole[i].x < 16){
          lcd.setCursor(obstacole[i].x, obstacole[i].y);
          lcd.write(byte(1));
        }

        if(obstacole[i].y == banda){
          if(obstacole[i].x == posX){
            life--;
          }
        }
      }
      // afisare scor si situatie
      scorAfisCalcul();
      lifeStatus();

      // citire viteza potentiometru
      currentSpeed = analogRead(potPin);
      int getNewY = moveSquares(currentSpeed);
      // deplasare masinuta
      posX += getNewY; 
      if(posX <= 0){
        posX = 0;
      }
      lcd.setCursor(12, 1);
      lcd.print(currentSpeed);
    delay(300);
  }
}
// afisare scor si calcul
void scorAfisCalcul(){
  scor++;
  if(prev == 0){
    prev = millis();
  }
  if(millis() - prev >= 3000){
     lcd.setCursor(13, 0);
     lcd.write(6);
     lcd.setCursor(14, 0);
     lcd.write(6);
     lcd.setCursor(15, 0);
     lcd.write(6);

     lcd.setCursor(13, 0);
     lcd.print(scor);
     prev = 0;
  }else{
      lcd.setCursor(13, 0);
      lcd.write(2);
      lcd.setCursor(14, 0);
      lcd.write(3);
      lcd.setCursor(15, 0);
      lcd.write(4);
  }
}

// intreruperea pentru inceperea jocului
void startGame()
{
  if(digitalRead(startPin) == 0){
      posX = 0;
      banda=1;
      inGame = true;
      endingGame = false;
  }
}

// intreruperea pentru finalizarea jocului
void endGame()
{
  if(digitalRead(stopPin) == 0){
      endingGame = true;
      inGame = false;
  }
}

// intreruperea pentru a schimba directia masinii
void goBack(){
  if(digitalRead(backPin) == 0){
    backwards = !backwards;
  }
}

// functia ce transforma viteza de la potentiometru in patratele
int moveSquares(int potVal){
  int sign = 1;
  if(backwards){
    sign = -1;
  }
    if(potVal < 100){
      return 0;
    }else{
      if(potVal < 400){
        return 1*sign;
      }else{
          if(potVal < 600){
            return 2*sign;
          }else{
            if(potVal < 900){
              return 3*sign;
            }else{
              if(potVal < 1000){
                return 4*sign;
              }
            }
          }
      }
    }
}

// setare banuti si vieti bonus
void lifeStatus(){
  // doar un banut poate fi afisat o data
  if(coinx==-1 && coiny ==-1){
     coinx = random(0, 16);
     coiny = random(0, 2);
     appearTime = millis();
  }else{
    lcd.setCursor(coinx, coiny);
    lcd.print("$");
  }
  // daca a atins banutul
  if(posX == coinx && banda == coiny){
      lcd.setCursor(coinx, coiny);
      lcd.write(byte(6));
      coinx = -1;
      coiny = -1;
      scor+=10;
    }
  // timpul de expirare pentru 
  if(millis() - appearTime >= 20000 && coinx!=-1 && coiny !=-1){
    scor -= 50;
    lcd.setCursor(coinx, coiny);
    lcd.write(byte(6));
    coinx = -1;
    coiny = -1;
  }
  // afisare vieti
  for(int i=0; i< life; i++){
      lcd.setCursor(i,0);
      lcd.write(5);
  }
  // finalizare joc
  if(life == 0){
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print(">");
      lcd.setCursor(3, 0);
      lcd.print("Game ended");
      lcd.setCursor(13, 0);
      lcd.print("<");
      lcd.setCursor(3, 1);
      lcd.print("Scor");
      lcd.setCursor(10, 1);
      lcd.print(scor);
      delay(10000);
      lcd.clear();
      inGame = false;
  }
  // generare vieti random
  if(life < 3){
    if(heartx ==-1 && hearty ==-1){
      heartx = random(0, 16);
      hearty = random(0, 2);
    }else{
      lcd.setCursor(heartx, hearty);
      lcd.write(byte(7));
    }
    // o singura viata o data
    if(posX == heartx && banda == hearty){
      lcd.setCursor(heartx, hearty);
      lcd.write(byte(6));
      heartx = -1;
      hearty = -1;
      life++;
    }
  }
}

// functia ce seteaza semaforul
void semaforFunc(){
  int randomVal = random(10, 2000);
  if(!semafor){
    if(!starting){
      if(startTime == 0){
        startTime = millis();
      }
      if(millis() - startTime <= 5000 && startTime != 0){
        digitalWrite(greenPin, LOW);
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, HIGH);
      }else{
        digitalWrite(greenPin, HIGH);
        digitalWrite(redPin, LOW);
        digitalWrite(yellowPin, LOW);
        startTime = 0;
      }
    }else{
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, LOW);
    }
    // verificare daca este momentul generarii unui led rosu
    int toCheck = millis()%100;
    if(toCheck == randomVal%100){
      semafor_prec = true;
      semafor = true;
      startMillis = millis();
      starting = false;
    }
  }else{
    unsigned long currentS = millis();
    // daca se doreste activarea ledului rosu, se asteapta 6s dupa cel galben
    if((currentS - startMillis >= 2000) && (currentS - startMillis <= 6000) && semafor_prec){
       digitalWrite(greenPin, LOW);
       digitalWrite(redPin, LOW);
       digitalWrite(yellowPin, HIGH);
    }
    // daca au trecut 5s de la activarea semnalului galben
    if(currentS - startMillis >= 5000){
     digitalWrite(greenPin, LOW);
     digitalWrite(redPin, HIGH);
     digitalWrite(yellowPin, LOW);
    }
    // semaforul se poate face rosu la minim 11s dupa inceperea jocului
    if(currentS - startMillis >= 11000){
        semafor_prec = false;
        semafor = false;
        alreadyPassed=false;
    }
  }
}

// verificare condus
void isDriving(){
  if(currentSpeed >= 100){
    driving = true;
  }else{
    driving = false;
  }
}

// verificare daca este rosu semaforul si conduce
void redPass(){
  if(driving && semafor && !alreadyPassed){
    life--;
    alreadyPassed=true;
  }
}

// intreruperea pentru schimbarea benzii
void roadChange()
{
  if(digitalRead(roadPin) == 0){
      banda = (banda==0)?1:0;
  }
}
