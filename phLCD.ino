#include <LiquidCrystal.h>

//LCD_INIT============================================
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
int read_LCD_buttons() {
  adc_key_in = analogRead(0);      // read the value from the sensor
  if (adc_key_in > 1000) return btnNONE;
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}

//GLOBALS===============================================
#define RX_PH     A1
#define MOTORGATE 2

enum {sRUN, sWait, sSet, sCal};
int systemState = sWait;

enum {red, yellow, green};
int runState;

float phSoll = 5.5;
float phIst, phLast;
int valueDelay = 0;
bool noPrell;

//SETUP===============================================
void setup()
{
  pinMode(MOTORGATE, OUTPUT);

  Serial.begin(9600);
  Serial.println("Ready");

  lcd.begin(16, 2);              // <== WICHTIG!!
  lcd.setCursor(0, 0);
  lcd.print("ph-Meter");
  lcd.setCursor(0, 1);
  lcd.print("><(((°>");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ph Ist");
}

void bufferPh()
{
  int arr_buffer[10];
  int temp = 0;
  unsigned long int avgVal = 0;

  for (int i = 0; i < 10; i++)  {
    arr_buffer[i] = analogRead(RX_PH);
    delay(10);
  }

  for (int i = 0; i < 9; i++)                 {
    for (int j = i + 1; j < 10; j++)          {
      if (arr_buffer[i] > arr_buffer[j])      {
        temp = arr_buffer[i];
        arr_buffer[i] = arr_buffer[j];
        arr_buffer[j] = temp;
      }
    }
  }

  for (int i = 2; i < 8; i++)
    avgVal += arr_buffer[i];

  float volt = (float)avgVal * 5.0 / 1024 / 6;
  phIst = 2.19 * volt + 2.85;

  //10 andere werte nötig für neun wert in phLast
  if (phIst != phLast)    valueDelay++;
  else                    valueDelay = 0;
  
  if (valueDelay > 10)   {
    valueDelay = 0;
    phLast = phIst;
  }
}

void checkPhValue ()
{
  lcd.setCursor(8, 0);
  
  if (phLast > phSoll)
  {
    if (phSoll + 0.5 < phLast)
    {      
      //overThreshold red
      runState = red;      
      digitalWrite(MOTORGATE, HIGH);
      lcd.print("Pumpt   ");
      Serial.println("Motor on, over Threesh");
    } else    {
      
      //underThreshold yellow
      runState = yellow;
      digitalWrite(MOTORGATE, LOW);
      lcd.print("In Range");
      Serial.println("Motor off, under Thressh");
    }
  }  else  {
    
    //unterSoll green
    runState = green;
    digitalWrite(MOTORGATE, LOW);
    lcd.print("Halte");
    Serial.println("Motor off");
  }
}

void resetLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ph Ist");
  lcd.setCursor(0, 1);
  lcd.print(phIst);
}
//LOOP==========================================================
void loop()
{
  bufferPh();

  lcd.setCursor(0, 1);
  lcd.print(phLast);

  //systemState = sCal;

  lcd.setCursor(8, 1);
  switch (systemState)
  {
    case sRUN:
      {
        checkPhValue();
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        lcd.setCursor(12, 1);
        lcd.print("+0.5");
        break;
      }

    case sWait:
      {
        digitalWrite(MOTORGATE, LOW);
        Serial.println("Wait Status");
        lcd.print("<-Lauf<<");
        lcd.setCursor(8, 0);
        lcd.print("Warten..");

        break;
      }

    case sSet:
      {
        lcd.print(phSoll);
        lcd.setCursor(8, 0);
        lcd.print("Set Soll");

        break;
      }

    case sCal:
      {

        lcd.print("Start?");
        lcd.setCursor(8, 0);
        lcd.print("pH Cal");

        break;
      }
  }

  //BUTTONS ================
  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {

    case btnRIGHT:      {
        if (noPrell == false)          break;
        if (systemState >= 3)          break;
        systemState++;
        resetLCD();
        noPrell = false;               break;
      }

    case btnLEFT:      {
        if (noPrell == false)          break;
        if (systemState <= 0)          break;
        systemState--;
        resetLCD();
        noPrell = false;               break;
      }

    case btnUP:      {
        if (noPrell == false)          break;
        if (systemState != 2)          break;
        if (phSoll >= 6.9)             break;
        phSoll += 0.1;
        noPrell = false;               break;
      }

    case btnDOWN:      {
        if (noPrell == false)          break;
        if (systemState != 2)          break;
        if (phSoll <= 5.0)             break;
        phSoll -= 0.1;
        noPrell = false;               break;
      }

    case btnNONE:      {
        noPrell = true;                break;
      }

    case btnSELECT:                    break;
  }

  //DEBUG SERIAL ================
  Serial.print("ph Ist     ");
  Serial.println(phIst);

  Serial.print("phSoll       ");
  Serial.println(phSoll);

  Serial.println("-_-_-_-_-_-_-_-_-_-_");
  Serial.println();


  //delay(200);

  /*
    lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
    lcd.print(millis()/1000);      // display seconds elapsed since power-up
  */

}

//to clear the LCD display, use the comment below
//lcd.clear();
