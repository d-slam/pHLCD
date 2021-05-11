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

enum {sRun, sWait, sSet, sCal};
int systemState = sWait;

enum {sRed, sYellow, sGreen};
int runState = sGreen;

float phSoll = 5.5;
float phSollOffset = 0.5;
float phIst, phLast = 0.0;

//float phIst, phLast, phSoll = 5.5, phSollOffset = 0.5;

int valueDelay = 0;
int valueRefreshRate = 1;
bool noPrell = true;

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

  redrawLCD();
}

void redrawLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ph Ist");
  lcd.setCursor(0, 1);
  lcd.print(phLast);

  switch (systemState) {
    case sRun: {
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        lcd.setCursor(12, 1);
        lcd.print("+0.5");
        //Serial.println("sRun");

        switch (runState) {
          case sRed: {
              lcd.setCursor(8, 0);
              lcd.print("Pumpt   ");
              break;
            }
          case sYellow: {
              lcd.setCursor(8, 0);
              lcd.print("In Range");
              break;
            }
          case sGreen: {
              lcd.setCursor(8, 0);
              lcd.print("Halte");
              break;
            }
        }    break;
      }
    case sWait: {
        //Serial.println("sWait /n");
        lcd.setCursor(8, 1);
        lcd.print("<-Lauf<<");
        lcd.setCursor(8, 0);
        lcd.print("Warten..");
        break;
      }
    case sSet: {
        //Serial.println("sSet");
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        lcd.setCursor(8, 0);
        lcd.print("Set Soll");
        break;
      }
    case sCal: {
        //Serial.println("sCal");
        lcd.setCursor(8, 1);
        lcd.print("Start?");
        lcd.setCursor(8, 0);
        lcd.print("pH Cal");
        break;
      }
  }
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


  

  Serial.print("phIst   ");
  Serial.println(phIst);
  Serial.print("phLast   ");
  Serial.println(phLast);


  //glättung,x onder werte sein nötig bevorer switcht, iwos zwischen 10 u 2 laft
  if (phIst != phLast)
    valueDelay++;
  else
    valueDelay = 0;

  if (valueDelay > valueRefreshRate)
  {
    valueDelay = 0;
    phLast = phIst;
    Serial.println("WECHSELT");
  }


}

//LOOP==========================================================
void loop()
{

  bufferPh();

  if (valueDelay == valueRefreshRate || (noPrell == false && adc_key_in != 1023) )
  {
    Serial.println("REDRAW");
    redrawLCD();
  }

  //Serial.println(valueDelay);

  //phLast = 6.0;


  switch (systemState)  {
    case sRun:      {
        if (phLast >= phSoll + phSollOffset) {
          runState = sRed;
          digitalWrite(MOTORGATE, HIGH);
          Serial.println("===sRed Motor On");
        }
        else if (phLast >= phSoll) {
          runState = sYellow;
          digitalWrite(MOTORGATE, LOW);
          Serial.println("===sYellow Motor Off");
        }
        else if (phLast < phSoll) {
          runState = sGreen;
          digitalWrite(MOTORGATE, LOW);
          Serial.println("===sGreen Motor Off");
        }                                    break;
      }
    case sWait:      {
        digitalWrite(MOTORGATE, LOW);        break;
      }
    case sSet:      {
        digitalWrite(MOTORGATE, LOW);        break;
      }
    case sCal:      {
        digitalWrite(MOTORGATE, LOW);        break;
      }
    default: digitalWrite(MOTORGATE, LOW);
  }

  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {
    case btnRIGHT:      {
        if (noPrell == false)          break;
        if (systemState >= 3)          break;
        systemState++;

        noPrell = false;               break;
      }
    case btnLEFT:      {
        if (noPrell == false)          break;
        if (systemState <= 0)          break;
        systemState--;

        noPrell = false;               break;
      }
    case btnUP:      {
        if (noPrell == false)          break;
        if (systemState != 2)          break;
        if (phSoll >= 7.9)             break;
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
  //  Serial.print("ph Ist     ");
  //  Serial.println(phIst);
  //
  //  Serial.print("phSoll       ");
  //  Serial.println(phSoll);
  //
  //  Serial.println("-_-_-_-_-_-_-_-_-_-_");
  //  Serial.println();


  //delay(200);

  /*
    lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
    lcd.print(millis()/1000);      // display seconds elapsed since power-up
  */

}

//to clear the LCD display, use the comment below
//lcd.clear();
