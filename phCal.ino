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

enum {cal7, cal4, calSet};
int calState = cal7;

float calDelta = 2.19;
float calOffset = 2.85;

float volt = 0.0;

float phSoll = 5.5;
float phSollOffset = 0.5;
float phIst, phLast = 0.0;

int nSmooth = 20;
int incBuffer = 0;
float vecBuffer = 0;

int incStateCheck = 0;

bool noPrell = true;

//SETUP===============================================
void setup()
{
  pinMode(MOTORGATE, OUTPUT);

  Serial.begin(9600);
  Serial.println("Hüü");

  lcd.begin(16, 2);              // <== WICHTIG!!
  lcd.setCursor(0, 0);
  lcd.print("ph-Meter");
  lcd.setCursor(0, 1);
  lcd.print("><(((°>");
  delay(1500);
  lcd.clear();

  redrawLCD();
}

//LOOP==========================================================
void loop()
{
  bufferPh();

  if (incBuffer >= nSmooth - 1 || (noPrell == false && adc_key_in != 1023) )
    redrawLCD();

  incStateCheck++;
  if (systemState == sRun && incStateCheck >= 50)
    checkState();

  checkButtons();
}
//MyMethodes==========================================================
void checkButtons()
{
  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {
    case btnRIGHT:      {
        if (noPrell == false)          break;
        if (systemState >= 3)          break;
        noPrell = false;               break;
        systemState++;
      }
    case btnLEFT:      {
        if (noPrell == false)          break;
        if (systemState <= 0)          break;
        noPrell = false;               break;
        systemState--;
      }
    case btnUP:      {
        if (noPrell == false)          break;
        if (systemState != 2)          break;
        if (phSoll >= 7.9)             break;
        noPrell = false;               break;
        phSoll += 0.1;
      }
    case btnDOWN:      {
        if (noPrell == false)          break;
        if (systemState != 2)          break;
        if (phSoll <= 5.0)             break;
        noPrell = false;               break;
        phSoll -= 0.1;
      }
    case btnNONE:      {
        noPrell = true;                break;
      }
    case btnSELECT:                   {

        if (noPrell == false)          break;
        if (systemState != 3)          break;
        if (calState >= 2)             break;
        calState++;

        break;
      }
  }
}


void checkState()
{
  incStateCheck = 0;
  switch (systemState)  {
    case sRun:        {
        if (phLast >= phSoll + phSollOffset) {
          runState = sRed;
          digitalWrite(MOTORGATE, HIGH);
        }
        else if (phLast >= phSoll) {
          runState = sYellow;
          digitalWrite(MOTORGATE, LOW);
        }
        else if (phLast < phSoll) {
          runState = sGreen;
          digitalWrite(MOTORGATE, LOW);
        } break;
      }
    case sWait:      {
        digitalWrite(MOTORGATE, LOW);
        break;
      }
    case sSet:      {
        digitalWrite(MOTORGATE, LOW);
        break;
      }
    case sCal:      {
        digitalWrite(MOTORGATE, LOW);

        float tempCalDelta = 0.0;
        float tempCalOffset = 0.0;
        float volt7 = 0.0;
        float volt4 = 0.0;

        switch (calState) {

          case cal7: {
              volt7 = volt;
              break;
            }

          case cal4: {
              volt4 = volt;
              tempCalDelta = (7 - 4) / (volt7 - volt4);
              tempCalOffset = 4 - ( tempCalDelta * volt4);

              break;
            }

          case calSet: {


              break;
            }
            

            break;
        }

        break;
      }
    default: digitalWrite(MOTORGATE, LOW);
  }
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
        lcd.setCursor(8, 1);
        lcd.print("<-Lauf<<");
        lcd.setCursor(8, 0);
        lcd.print("Warten..");
        break;
      }
    case sSet: {
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        lcd.setCursor(8, 0);
        lcd.print("Set Soll");
        break;
      }
    case sCal: {

        switch (calState)
        {
          case calWait: {
              lcd.setCursor(8, 1);
              lcd.print("Start?");
              lcd.setCursor(8, 0);
              lcd.print("pH Cal");

              break;
            }

          case cal7: {
              lcd.setCursor(8, 1);
              lcd.print("Set?");
              lcd.setCursor(8, 0);
              lcd.print("ph 7");

              break;
            }

          case cal4: {
              lcd.setCursor(8, 1);
              lcd.print("Set?");
              lcd.setCursor(8, 0);
              lcd.print("ph 4");

              break;
            }

          case calSet: {

              lcd.setCursor(8, 1);
              lcd.print("Save?");
              lcd.setCursor(8, 0);
              lcd.print("Enter New Cal");

              break;
            }
            break;
        }
        break;
      }
  }
}


void bufferPh()
{
  int sampleBuffer[10];
  int temp = 0;
  unsigned long int avgVal = 0;

  for (int i = 0; i < 10; i++)  {
    sampleBuffer[i] = analogRead(RX_PH);
    delay(10);
  }

  for (int i = 0; i < 9; i++)                 {
    for (int j = i + 1; j < 10; j++)          {
      if (sampleBuffer[i] > sampleBuffer[j])  {
        temp = sampleBuffer[i];
        sampleBuffer[i] = sampleBuffer[j];
        sampleBuffer[j] = temp;
      }
    }
  }
  for (int i = 2; i < 8; i++)
    avgVal += sampleBuffer[i];

  volt = ((float)avgVal * 5.0 / 1024 / 6);

  phIst = calDelta * volt + calOffset;

  vecBuffer += phIst;
  incBuffer++;
  if (incBuffer >= nSmooth)
  {
    phLast = vecBuffer / nSmooth;
    incBuffer = 0;
    vecBuffer = 0;
  }
}

//to clear the LCD display, use the comment below
//lcd.clear();
