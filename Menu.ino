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


#define RX_PH     A1
#define MOTORGATE 2


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

  redrawLCD();
}

//GLOBALS===============================================
typedef enum {SYS_RUN,  SYS_WAIT,  SYS_SET,  SYS_CAL} statesSystem;
statesSystem SYSstate = SYS_WAIT;

typedef enum {CAL_START, CAL_PH4,  CAL_PH7,  CAL_CONF, CAL_OK} statesCal;
statesCal CALstate = CAL_START;

bool incFlag = false;
bool decFlag = false;

bool btnPrellFlag = false;

float calDelta = 2.19;
float calOffset = 2.85;

float tempCalDelta = 0.0;
float tempCalOffset = 0.0;

float volt4 = 0.0;
float volt7 = 0.0;

float volt = 0.0;

float phSoll = 5.5;
float phSollOffset = 0.5;
float phIst, phLast = 0.0;

int nSmooth = 20;
int incBuffer = 0;
float vecBuffer = 0;

//LOOP==========================================================
void loop()
{
  checkButtons();
  bufferPh();
  redrawLCD();

  switch (SYSstate)
  {
    case SYS_RUN :            break;
    case SYS_WAIT :           break;

    case SYS_SET:
      if (incFlag == true)        incSoll();
      if (decFlag == true)        decSoll();
      break;

    case SYS_CAL:
      switch (CALstate)
      {
        case CAL_START:          break;
        case CAL_PH4:          break;
        case CAL_PH7:          break;
        case CAL_CONF:          break;
        case CAL_OK:          break;
      }
      break;
  }
}

//MyMethodes==========================================================
void setMenu(int keyPressed)
{
  switch (SYSstate)
  {
    case SYS_RUN :
      if (keyPressed == btnRIGHT)        SYSstate = SYS_WAIT;
      break;

    case SYS_WAIT :
      if (keyPressed == btnRIGHT)        SYSstate = SYS_SET;
      if (keyPressed == btnLEFT)        SYSstate = SYS_RUN;
      break;

    case SYS_SET:
      if (keyPressed == btnRIGHT)        SYSstate = SYS_CAL;
      if (keyPressed == btnLEFT)        SYSstate = SYS_WAIT;

      if (keyPressed == btnUP)        incFlag = true;
      if (keyPressed == btnDOWN)        decFlag = true;
      break;

    case SYS_CAL:
      switch (CALstate)
      {
        case CAL_START:
          if (keyPressed == btnLEFT)        SYSstate = SYS_SET;
          if (keyPressed == btnSELECT)        CALstate = CAL_PH4;
          break;

        case CAL_PH4:
          if (keyPressed == btnSELECT)        CALstate = CAL_PH7;
          break;

        case CAL_PH7:
          if (keyPressed == btnSELECT)        CALstate = CAL_CONF;
          break;

        case CAL_CONF:
          if (keyPressed == btnSELECT)        CALstate = CAL_OK;
          if (keyPressed == btnLEFT)          CALstate = CAL_START;
          break;

        case CAL_OK:
          if (keyPressed == btnSELECT)        SYSstate = SYS_WAIT;
          CALstate = CAL_PH4;
          break;
      }
      break;
  }
}


void checkButtons()
{
  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {
    case btnRIGHT:
      if (btnPrellFlag == false) break;
      btnPrellFlag = false;
      setMenu(lcd_key);
      break;
    case btnLEFT:
      if (btnPrellFlag == false) break;
      btnPrellFlag = false;
      setMenu(lcd_key);
      break;
    case btnSELECT:
      if (btnPrellFlag == false) break;
      btnPrellFlag = false;
      setMenu(lcd_key);
      break;
    case btnUP:
      if (btnPrellFlag == false) break;
      btnPrellFlag = false;
      setMenu(lcd_key);
      break;
    case btnDOWN:
      if (btnPrellFlag == false) break;
      btnPrellFlag = false;
      setMenu(lcd_key);
      break;
    case btnNONE:      btnPrellFlag = true;      break;
  }
}

void redrawLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ph Ist");
  lcd.setCursor(0, 1);
  lcd.print(phLast);

  switch (SYSstate)
  {
    case SYS_RUN :
      lcd.setCursor(8, 0);
      lcd.print("sRUN");
      lcd.setCursor(8, 1);
      lcd.print("phSoll");
      break;

    case SYS_WAIT :
      lcd.setCursor(8, 0);
      //writeLCD("sWait");
      lcd.print("sWait");
      lcd.setCursor(8, 1);
      lcd.print("<-Lauf<<");
      break;

    case SYS_SET :
      lcd.setCursor(8, 0);
      lcd.print("sSET");
      lcd.setCursor(8, 1);
      lcd.print(phSoll);
      break;

    case SYS_CAL :
      switch (CALstate)
      {
        case CAL_START:
          lcd.clear();
          lcd.setCursor(8, 0);
          lcd.print("[SELECT]");
          lcd.setCursor(8, 1);
          lcd.print("Start Cali");
          break;

        case CAL_PH4 :
          lcd.clear();
          lcd.setCursor(8, 0);
          lcd.print("sCal");
          lcd.setCursor(8, 1);
          lcd.print("->ph4 ?");
          break;

        case CAL_PH7 :
          lcd.clear();
          lcd.setCursor(8, 0);
          lcd.print("sCal");
          lcd.setCursor(8, 1);
          lcd.print("->ph7 ?");
          break;

        case CAL_CONF :
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("[LEFT]");
          lcd.setCursor(0, 1);
          lcd.print("CANCEL");
          lcd.setCursor(8, 0);
          lcd.print("[SELECT]");
          lcd.setCursor(8, 1);
          lcd.print("apply?");
          break;

        case CAL_OK :
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Callibration");
          lcd.setCursor(8, 1);
          lcd.print("Complette");
          break;
      }
      break;
  }
}


void incSoll()
{
  phSoll += 0.1;
  incFlag = false;
}
void decSoll()
{
  phSoll += 0.1;
  decFlag = false;
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


void setPh4(float volt)
{
  volt4 = volt;
}

void setPh7(float volt)
{
  volt7 = volt;
  tempCalDelta = (7 - 4) / (volt7 - volt4);
  tempCalOffset = 4 - ( tempCalDelta * volt4);
}

void applyCallibration()
{
  calDelta = tempCalDelta;
  calOffset = tempCalOffset;
}



void writeLCD(const char *string)
{
  int inc = 0;
  while (*string != '\0')
  {
    lcd.print(inc, *string);
    inc++;
    //lcd_data(*string++);
  }
}

void setMOTORGATE(bool state) {
  if (state == true)
    digitalWrite(MOTORGATE, HIGH);
  else if (state == false)
    digitalWrite(MOTORGATE, LOW);
}
