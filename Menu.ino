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

typedef enum {SYS_RUN,  SYS_WAIT,  SYS_SET_SOLL, SYS_SET_THRES,  SYS_CAL} statesSystem;
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
float phSollThres = 0.5;
float phIst, phLast = 0.0;

int nSmooth = 20;
int incBuffer = 0;
float vecBuffer = 0;

void setup()
{
  pinMode(MOTORGATE, OUTPUT);

  Serial.begin(9600);
  Serial.println("Hüü");

  lcd.begin(16, 2);              // <== WICHTIG!!

  lcdSplashscreen("ph-Meter", 0, 0, "><(((°>", 0, 1, 1500);

  redrawLCD();
}

//LOOP==========================================================
void loop()
{
  bufferPh();
  checkButtons();     //called setMenu()

  switch (SYSstate)
  {
    case SYS_RUN :            break;
    case SYS_WAIT :           break;
    case SYS_SET_SOLL:
      if (incFlag == true)        incSoll();
      if (decFlag == true)        decSoll();
      break;
    case SYS_SET_THRES:
      if (incFlag == true)        incThres();
      if (decFlag == true)        decThres();
      break;
    case SYS_CAL:
      switch (CALstate)
      {
        case CAL_START:         break;
        case CAL_PH4:          break;
        case CAL_PH7:          break;
        case CAL_CONF:          break;
        case CAL_OK:          break;
      }
      break;
  }

  redrawLCD();
}

//MyMethodes==========================================================
void redrawLCD()
{
  lcd.clear();
  lcdWriteAtXY("ph Ist", 0, 0);
  lcdWriteFloatAtXY(phLast, 0, 1);

  switch (SYSstate)
  {
    case SYS_RUN :      lcdWriteTwoLines("sRUN", 8, 0, "phSoll", 8, 1);      break;
    case SYS_WAIT :      lcdWriteTwoLines("sWait", 8, 0, "<-Go<<", 8, 1);      break;

    case SYS_SET_SOLL :
      lcdWriteAtXY("setSoll", 8, 0);
      lcdWriteFloatAtXY(phSoll, 8, 1);
      break;

    case SYS_SET_THRES :
      lcdWriteAtXY("setThres", 8, 0);
      lcdWriteFloatAtXY(phSollThres, 8, 1);      
      break;

    case SYS_CAL :
      lcd.clear();
      switch (CALstate)
      {
        case CAL_START:         lcdWriteTwoLines("[SELECT]", 8, 0, "start Cal?", 8, 1);               break;
        case CAL_PH4 :          lcdWriteTwoLines("Cal", 8, 0, "->ph4", 8, 1);                         break;
        case CAL_PH7 :          lcdWriteTwoLines("Cal", 8, 0, "->ph7", 8, 1);                           break;
        case CAL_CONF :         lcdWriteTwoLines("[LEFT]  [SELECT]", 0, 0, "CANCEL?   apply?", 0, 1);   break;
        case CAL_OK :   lcdSplashscreen("Callibration", 0, 0, "Complette!", 6, 1, 1500);  setMenu(btnSELECT);   break; //wenni des net moch hängi in "call complette" screen bissi iwos druck....?!?!
      }
      break;
  }
}

void lcdSplashscreen(const char *stringA, uint8_t xA, uint8_t yA, const char *stringB, uint8_t xB, uint8_t yB, int tDelay)
{
  lcd.clear();
  lcd.setCursor(xA, yA);
  lcd.print(stringA);
  lcd.setCursor(xB, yB);
  lcd.print(stringB);
  delay(tDelay);
}

void lcdWriteAtXY(const char *string, uint8_t x, uint8_t y)
{
  lcd.setCursor(x, y);
  lcd.print(string);
}

void lcdWriteTwoLines(const char *stringA, uint8_t xA, uint8_t yA, const char *stringB, uint8_t xB, uint8_t yB)
{
  lcd.setCursor(xA, yA);
  lcd.print(stringA);
  lcd.setCursor(xB, yB);
  lcd.print(stringB);
}

void lcdWriteFloatAtXY(float value, uint8_t x, uint8_t y)
{
  lcd.setCursor(x, y);
  lcd.print(value);
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

void setMenu(int keyPressed)
{
  switch (SYSstate)
  {
    case SYS_RUN :      if (keyPressed == btnRIGHT)        SYSstate = SYS_WAIT;      break;
    case SYS_WAIT :
      if (keyPressed == btnRIGHT)        SYSstate = SYS_SET_SOLL;
      if (keyPressed == btnLEFT)        SYSstate = SYS_RUN;
      break;
    case SYS_SET_SOLL:
      if (keyPressed == btnRIGHT)        SYSstate = SYS_SET_THRES;
      if (keyPressed == btnLEFT)        SYSstate = SYS_WAIT;
      if (keyPressed == btnUP)        incFlag = true;
      if (keyPressed == btnDOWN)        decFlag = true;
      break;
    case SYS_SET_THRES:
      if (keyPressed == btnRIGHT)        SYSstate = SYS_CAL;
      if (keyPressed == btnLEFT)        SYSstate = SYS_SET_SOLL;
      if (keyPressed == btnUP)        incFlag = true;
      if (keyPressed == btnDOWN)        decFlag = true;
      break;
    case SYS_CAL:
      switch (CALstate)
      {
        case CAL_START:
          if (keyPressed == btnLEFT)        SYSstate = SYS_SET_THRES;
          if (keyPressed == btnSELECT)        CALstate = CAL_PH4;
          break;
        case CAL_PH4:          if (keyPressed == btnSELECT)        CALstate = CAL_PH7;          break;
        case CAL_PH7:          if (keyPressed == btnSELECT)        CALstate = CAL_CONF;          break;
        case CAL_CONF:
          if (keyPressed == btnSELECT)        CALstate = CAL_OK;
          if (keyPressed == btnLEFT)          CALstate = CAL_START;
          break;
        case CAL_OK:
          CALstate = CAL_PH4;
          SYSstate = SYS_WAIT;
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
void incThres()
{
  phSollThres += 0.1;
  incFlag = false;
}
void decThres()
{
  phSollThres += 0.1;
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
