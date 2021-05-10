//Sample using LiquidCrystal library
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
#define RUN       0
#define WAIT      1
#define SET       2
#define CAL       3
int statSys = WAIT;
float phSoll = 5.5;
float phLast;
int valueDelay = 0;
bool noPrell;

//MYMETHODS===============================================
//struct STROK {};

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

//LOOP==========================================================
void loop()
{             //get PhIst ================   
  int arr_buffer[10];
  int temp = 0;
  unsigned long int avgVal = 0;

  for (int i = 0; i < 10; i++)
  {
    arr_buffer[i] = analogRead(RX_PH);
    delay(10);
  }

  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (arr_buffer[i] > arr_buffer[j])
      {
        temp = arr_buffer[i];
        arr_buffer[i] = arr_buffer[j];
        arr_buffer[j] = temp;
      }
    }
  }

  for (int i = 2; i < 8; i++)
    avgVal += arr_buffer[i];

  float volt = (float)avgVal * 5.0 / 1024 / 6;
  float phIst = 2.19 * volt + 2.85;
  
 
  //WRITE ================
  if (phIst != phLast)
    valueDelay++;
  else
    valueDelay = 0;
  if (valueDelay > 10)       //10 andere werte nötig für aktuall
  {
    lcd.setCursor(0, 1);
    lcd.print(phIst);
    valueDelay = 0;
    phLast = phIst;
  }

  

  //CONTROLL ================
  //statSys = RUN;      //DEBUG!!!! Net vergessn!!
  switch (statSys)
  {

    case RUN:
      {
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        lcd.setCursor(12, 1);
        lcd.print("+0.5");

        if (phIst > phSoll)
        {

          if (phSoll + 0.5 < phIst)
          {
            digitalWrite(MOTORGATE, HIGH);
            Serial.println("Motor on, over Threesh");
            lcd.setCursor(8, 0);
            lcd.print("Running");
          } else
          {
            digitalWrite(MOTORGATE, LOW);
            Serial.println("Motor off, under Thressh");
            lcd.setCursor(8, 0);
            lcd.print("In Range");
          }
        }
        else
        {
          digitalWrite(MOTORGATE, LOW);
          Serial.println("Motor off");
          lcd.setCursor(8, 0);
          lcd.print("Halte");
        }
        break;
      }

    case WAIT:
      {
        digitalWrite(MOTORGATE, LOW);
        Serial.println("Wait Status");
        lcd.setCursor(8, 0);
        lcd.print("Warten..");
        lcd.setCursor(8, 1);
        lcd.print("<-Run|->");
        break;
      }

    case SET:
      {
        lcd.setCursor(8, 0);
        lcd.print("Set Soll");
        lcd.setCursor(8, 1);
        lcd.print(phSoll);
        break;
      }

    case CAL:
      {
        lcd.setCursor(8, 0);
        lcd.print("pH Cal");
        lcd.setCursor(8, 1);
        lcd.print("Soon");
        break;
      }

  }






  //BUTTONS ================
  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {

    case btnRIGHT:
      {
        if (noPrell == false)
          break;
        if (statSys >= 3)
          break;
        statSys++;
        noPrell = false;
        break;
      }

    case btnLEFT:
      {
        if (noPrell == false)
          break;
        if (statSys <= 0)
          break;
        statSys--;
        noPrell = false;
        break;
      }

    case btnUP:
      {
        if (noPrell == false)
          break;
        if (statSys != 2)
          break;
        if (phSoll >= 6.9)
          break;
        phSoll += 0.1;
        noPrell = false;
        break;
      }
    case btnDOWN:
      {
        if (noPrell == false)
          break;
        if (statSys != 2)
          break;
        if (phSoll <= 5.0)
          break;
        phSoll -= 0.1;
        noPrell = false;
        break;
      }

    case btnNONE:
      {
        noPrell = true;
        break;
      }

    case btnSELECT:   break;
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
