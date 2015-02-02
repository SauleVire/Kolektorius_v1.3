// ============= Solar Controller v1.21 (en) ===============================================
/*
You are free:
to Share — to copy, distribute and transmit the work
to Remix — to adapt the work
Under the following conditions:
Attribution — You must attribute the work in the manner specified by the author or licensor
(but not in any way that suggests that they endorse you or your use of the work).
Noncommercial — You may not use this work for commercial purposes.
Share Alike — If you alter, transform, or build upon this work, you may distribute
the resulting work only under the same or similar license to this one.
All code is copyright Alvydas, alvydas (at) saulevire.lt (c)2014.

Valdiklio nustatymai:
Keičiamas temperatūrų skirtumas siurblio įjungimui/išjungimui
Rankinis siurblio įjungimas/išjungimas kolektoriaus nuorinimui
Reikšmių išsaugojimas
Termostatas su šildymo/šaldymo funkcija arba išjungtas
Ekranas LCD 16x2
Keičiamas ekrano pašvietimo ryškumas
5 klavišų klaviatūra valdymui
Jungtis tinklo modulio ENC28J60 pajungimui
*/

#define Key_Pin A7    // analog pin assigned for button reading
#define BackLight_Pin 9 //LCD backlight pin (standart LCD KeeyPad use pin 10)

#define ONE_WIRE_BUS1 2 // Collector
#define ONE_WIRE_BUS2 8 // Boiler
#define ONE_WIRE_BUS3 A3 // Thermostat
//______________________________________________________________________________________//

#include <Wire.h>
#include "MenuBackend.h" 
 // Thank wojtekizk, for example
 // http://majsterkowo.pl/forum/menubackend-jak-sie-w-nim-odnalezc-t1549.html
 //  #include <LiquidCrystal.h>        
 #include <OneWire.h>
#include <DallasTemperature.h>
 #include <EEPROM.h>
#include "definitions.h"

//______________________________________________________________________________________//
// It is assumed that the LCD module is connected to
// the following pins using a levelshifter to get the
// correct voltage to the module.
//      SCK  - Pin A0
//      DIN  - Pin 4
//      DC   - Pin 5
//      RST  - Pin 7
//      CS   - Pin 6
//

#include <LCD5110_Basic.h>

LCD5110 myGLCD(A0,4,5,7,6);
extern uint8_t SmallFont[];
//extern uint8_t MediumNumbers[];
//extern uint8_t BigNumbers[];
//______________________________________________________________________________________//

/* ------------------ R T C ---------------------- */
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

boolean bBlink = true;
/* --------------------- RTC END ---------------- */
char *Nokia_LCD_string_1;                      // First string text displayed on the LCD
char *Nokia_LCD_string_2;                      // Second string text displayed on the LCD
    boolean InMenu = false;
/* --------------------- freeRam ---------------- */
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
/* --------------------- freeRam END ---------------- */
// --- create all of the options menu: ---------------------------------------
// de facto create a MenuItem class objects, which inherit the class MenuBackend
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent); // menu design
   //                        ("                ")
   MenuItem P1 =  MenuItem(" KOLEKTORIUS  ",1);
      MenuItem P11 = MenuItem("skirtumas on  ",2);
      MenuItem P12 = MenuItem("skirtumas off ",2);
      MenuItem P13 = MenuItem("siurblio ij   ",2);


   MenuItem P2 = MenuItem(" TERMOSTATAS  ",1);//"TERMOSTATAS   "
      MenuItem P21 = MenuItem("temperatura 1 ",2);//"temperatura 1 "
//      MenuItem P22 = MenuItem("temperatura 2 ",2);//"temperatura 2 "
      MenuItem P22 = MenuItem("busena        ",2);//"Busena        "


   MenuItem P3 = MenuItem("  NUSTATYMAI  ",1);
      MenuItem P31 = MenuItem("irasyti       ",2);
      MenuItem P32 = MenuItem("numatyti      ",2);
      MenuItem P33 = MenuItem("sviesumas     ",2);


/* --- Now position the menu (according to the setting specified above) ------------
add - adds vertical addRight - adds a level to the right, to the left adds addLeft
*/
void menuSetup()                       // feature class MenuBackend
{
      menu.getRoot().add(P1);          // set the root menu, which is the first option
      P1.add(P11);
        P11.add(P12);P11.addLeft(P1);  // 
        P12.add(P13);P12.addLeft(P1);  //
        P13.add(P11);P13.addLeft(P1);  //

      menu.getRoot().add(P2);
      P1.addRight(P2);                 //
     
      P2.add(P21);                     //
        P21.add(P22);P21.addLeft(P2);  //
        P22.add(P21);P22.addLeft(P2);  //
 //       P23.add(P21);P23.addLeft(P2);  //

      menu.getRoot().add(P3);
      P2.addRight(P3);                 //
       
      P3.add(P31);                     //
        P31.add(P32);P31.addLeft(P3);  //
        P32.add(P33);P32.addLeft(P3);  //
        P33.add(P31);P33.addLeft(P3);  //

      menu.getRoot().add(P1);
      P3.addRight(P1);                 //
     
}

// -----------  -----------------------------------------------------------------------
void menuUseEvent(MenuUseEvent used)      // feature class MenuBackend - after pressing OK
                                          // Here is the menu we offer for shares of handling the OK button
{
  #ifdef DEBUGSerialPrint
  Serial.print("pasirinkta:  "); Serial.println(used.item.getName()); // test and then unnecessary
  #endif
// --- Below are some of service options -----------

     /* ______________________ SETTINGS Save _______________________ */
// Save to EEPROM
//     if (used.item.getName() == "Exit          ")   // exactly the same string "Save          "
//      { menu.moveLeft(); }
      //////////////////////////////////////////////////////////////
/* __________________________Settings brithness __________________ */
  if (used.item.getName() == "sviesumas     ")
  {             
  myGLCD.print("Sviesumas", LEFT, 40); // keiciamos reikšmes pavadinimas
  myGLCD.printNumI(lcd_backlight,60,40);myGLCD.print("0%",66,40);                        // dabartine reikšme
  int  action=-1;delay(1000);                                             // pagalbinis kintamasis, kontroliuojantis while cikla
                                                                         // jei jums nereikia keisti, spauti OK po 1 sek. ir grižti i meniu  
  while(action!=4)                   // Šis ciklas bus kartojamas, kol paspausite mygtuka OK
         {
           Keyboard_change=-1; 
           action=Read_keyboard(Key_Pin);//delay(300);   // odczyt stanu klawiatury - funkcja Klaviaturos_skaitymas lub czytaj_2 lub czytaj_3
                                            // opis ponizej przy 3 róznych definicjach funkcji czytaj
           if(Keyboard_change!=action)                    // ruszamy do pracy tylko wtedy gdy Keyboard_changeienil sie stan klawiatury
             {if (action==3) {lcd_backlight++; analogWrite(BackLight_Pin,lcd_backlight*25);delay(300);}
               // jesli akcja=1 (czyli wcisnieto klawisz w góre to zwiekszono temperature
               // ustawiono max próg i wyswietlono obecna temperature
             if(action==0)  {lcd_backlight--;analogWrite(BackLight_Pin,lcd_backlight*25);delay(300);}
if (lcd_backlight > 10)  lcd_backlight = 1;
if (lcd_backlight < 1)  lcd_backlight = 10;
            if (lcd_backlight < 10) myGLCD.print(" ",54,40);
            myGLCD.printNumI(lcd_backlight,60,40);
            if (lcd_backlight == 0) 
            myGLCD.print("0% ",60,40);

               // jesli akcja=2 (czyli wcisnieto klawisz w dól to mniejszono temperature
               // ustawiono min próg i wyswietlono obecna temperature
             if(action==4) // jesli wcisnieto OK 
               {
                 myGLCD.print(">Sviesumas  OK",0,32);delay(2000); // pokazujemy OK przez 2 sek.
                 myGLCD.print("              ",0,40); // czyscimy linie
                 menu.moveDown();
               //  lcd.setCursor(1,0);lcd.print(eilute1);           // odtwarzamy poprzedni stan na LCD
               }
             } 
         } Keyboard_change=action;  // aktualizacja Keyboard_changeiennej Keyboard_change, po to aby reagowac tylko na Keyboard_changeiany stanu klawiatury
         // tu WAZNY MOMENT - konczy sie petla while i zwracamy sterowanie do glównej petli loop()
      } 
      //////////////////////////////////////////////////////////////////      
      /* ______________________ SETTINGS Save _______________________ */
// Save to EEPROM
     if (used.item.getName() == "irasyti       ")   // exactly the same string "Save          "
      {
                 SaveConfig();
                 myGLCD.print(">Irasyta OK     ", 0, 32);delay(2000); // show OK for 2 sec
                 myGLCD.print("              ", 0, 40); // clear line
                 myGLCD.print(Nokia_LCD_string_1, CENTER, 24);           // reconstruct the previous state at LCD
                 menu.moveDown();

      }
      //////////////////////////////////////////////////////////////
      /* __________________________ SETTINGS default ____________ */
// Save to EEPROM
     if (used.item.getName() == "numatyti      ")   // exactly the same string "Save          "
      {
                 myGLCD.print(">numatyti OK  ", 0, 32);delay(2000); // show OK for 2 sec
                 myGLCD.print("              ", 0, 40); // clear line
                 myGLCD.print(Nokia_LCD_string_1, CENTER, 24);           // reconstruct the previous state at LCD
                 
Pump_power_on_difference = 6; 
Pump_power_off_difference = 3;
temperature_1 = 20; 
temperature_2 = 25;
Thermostat_status = 3; // off
Manual_pump_status = false;
lcd_backlight = 5;                
                 SaveConfig();
                menu.moveDown();

      }
      //////////////////////////////////////////////////////////////
/* __________________________ Collector   _______________________ */
//  ON - the difference between the temperature      
if (used.item.getName() == "skirtumas on  ")   // exactly the same string "Difference on "
Pump_power_on_difference =  MeniuFunkcija ("Parinkti= ", Pump_power_on_difference, 25, 1, ">Skirtumas OK");
     ///////////////////////////////////////////////////////////////////
/* __________________________ Collector _______________________ */
// OFF - the difference between the temperature           
if (used.item.getName() == "skirtumas off ")   // exactly the same string "Difference off"
Pump_power_off_difference =  MeniuFunkcija ("Parinkti= ", Pump_power_off_difference, 25, 1, ">Skirtumas OK");
     ///////////////////////////////////////////////////////////////////    

/* __________________________ Collector Manual pump on ____________________________________ */
 if (used.item.getName() == "siurblio ij   ")
 {      
        myGLCD.print("siurblys", 0, 32);
        if (Manual_pump_status == true) myGLCD.print(" -on ", RIGHT, 32); //
        if (Manual_pump_status == false) myGLCD.print(" -off", RIGHT, 32); //
        int  action=-1; delay(1000);         //
                                          
        while(action!=4)                   //
         {
           Keyboard_change=-1;
           action=Read_keyboard(Key_Pin); //delay(300); 
                                           
           if(Keyboard_change!=action)          
             {
if(action==3) {Manual_pump_status = false; myGLCD.print("off", RIGHT, 32);delay(200);}
if(action==0) {Manual_pump_status = true;  myGLCD.print("on ", RIGHT, 32);delay(200);}
             if(action==4) // 0
               {
                 myGLCD.print(">siurblys OK  ", RIGHT, 32); delay(2000); // 0
                 myGLCD.print("              ", RIGHT, 40); // 0
//                 myGLCD.print(Nokia_LCD_string_1, CENTER, 24);           // 0
                 menu.moveDown();
               }
             }
         } Keyboard_change=action;
 } 
/* __________________________ Termostat temperature 1   _______________________ */
if (used.item.getName() == "temperatura 1 ")   // exactly the same string "temperature 1 "
temperature_1 =  MeniuFunkcija ("temp 1=    ", temperature_1, 99, -25, "Temperatura OK");
     ///////////////////////////////////////////////////////////////////
/* __________________________ Termostat temperature 2  _______________________ */    
//if (used.item.getName() == "temperatura 2 ")   // exactly the same string "temperature 2 "
//temperature_2 =  MeniuFunkcija ("temp 2=    ", temperature_2, 99, -25, "Temperatura OK");
     ///////////////////////////////////////////////////////////////////    
/* __________________________ Termostat status  _______________________ */    
if (used.item.getName() == "busena        ")
 {      
        //tft.write(7);    
        myGLCD.print("busena-", LEFT, 32);
        if (Thermostat_status == 1) myGLCD.print("sildymas", RIGHT, 40); // heating
        if (Thermostat_status == 2) myGLCD.print("saldymas", RIGHT, 40); // freezing
        if (Thermostat_status == 3) myGLCD.print("isjungta", RIGHT, 40); // turned off
//      myGLCD.print(Busena(Thermostat_status,termostato_status_name));
        int  action=-1; delay(1000);         //
                                          
        while(action!=4)                   //
         {
           Keyboard_change=-1;
           action=Read_keyboard(Key_Pin); //delay(300); 
                                           
           if(Keyboard_change!=action)          
             {
             if (action==3) {Thermostat_status++; if(Thermostat_status>3) Thermostat_status=1;

                                               //  myGLCD.print(Busena(Thermostat_status,termostato_status_name));
                                                 if (Thermostat_status == 1) myGLCD.print("sildymas", RIGHT, 40); // heating
                                                 if (Thermostat_status == 2) myGLCD.print("saldymas", RIGHT, 40); // freezing
                                                 if (Thermostat_status == 3) myGLCD.print("isjungta", RIGHT, 40); // turned off
                                            delay(200);}
             if(action==0)  {Thermostat_status--; if(Thermostat_status<1) Thermostat_status=3;

                                              //   myGLCD.print(Busena(Thermostat_status,termostato_status_name));
                                                 if (Thermostat_status == 1) myGLCD.print("sildymas", RIGHT, 40); // heating
                                                 if (Thermostat_status == 2) myGLCD.print("saldymas", RIGHT, 40); // freezing
                                                 if (Thermostat_status == 3) myGLCD.print("isjungta", RIGHT, 40); // turned off
                                               delay(200);}
             if(action==4) // 0
               {
                  myGLCD.print(">busena     OK", LEFT, 32); delay(2000); // 0
                  myGLCD.print("              ", LEFT, 40); // 0
                 myGLCD.print(Nokia_LCD_string_1, CENTER, 24);           // 0
                 menu.moveDown();
               }
             }
         } Keyboard_change=action;
 }
 
}
// --- Reakcja na wci�ni�cie klawisza -----------------------------------------------------------------
void menuChangeEvent(MenuChangeEvent changed)  // funkcja klasy MenuBackend
{
  menuTimer = millis(); // menu is inactive timer
  timerEnable = 1;
 
  if(changed.to.getName()==menu.getRoot())
  {
    InMenu =false;
    #ifdef DEBUGSerialPrint
    Serial.println("menuChangeEvent:- Now we are on MenuRoot");
    #endif
    myGLCD.print("              ", 0, 24);
    myGLCD.print("              ", 0, 32);
    myGLCD.print("              ", 0, 40);
   LCD_TFT_template();
   TFT_Temperature_Imaging();

  }
  /* it really is only useful here in shortkey and is used primarily to enrich the
  menu with arrow symbols depending on what is selected. Everything here is going
  on is displayed on the tft.
  */
  int c=changed.to.getShortkey();                         // shortkey charge (1,2,3, or 4)
  if(c==1)                                                // If this menu Main co                   ntacts (shortkey = 1) are:
    {InMenu =true;
    strcpy(Nokia_LCD_string_1,changed.to.getName());     // Create a string in the first line
    myGLCD.invertText(true);
    myGLCD.print(Nokia_LCD_string_1, CENTER, 24);myGLCD.invertText(false);
    myGLCD.print("              ", 0, 32);
    myGLCD.print("              ", 0, 40);// Display it
                     // Right arrow
                      // Down arrow
                     // Down arrow
    }
    if(c==2)                                              // if the submenu for the child - (shortkey = 2) are:
    {InMenu =true;
//    myGLCD.print("*");                                       // draw a star
    strcpy(Nokia_LCD_string_2,changed.to.getName());            // create a string in the first line
myGLCD.invertText(true);
myGLCD.print(Nokia_LCD_string_1, CENTER, 24);//                // print it
myGLCD.invertText(false);
//myGLCD.print(P2);
//    tft.setCursor(15,0);myGLCD.print("*");                   // draw a star
//    tft.setCursor(0,1);tft.write(6);                      // the second line and arrow return (arrowBack)
myGLCD.print(changed.to.getName(), CENTER, 32);            // display name of "child"
//    tft.setCursor(15,1);tft.write(7);                     // arrow up-down
    }
    if(c==3)                                              // if the child has a child - (shortkey = 3) are:
    {InMenu =true;
//    myGLCD.print("*");                                       // draw a star
    strcpy(Nokia_LCD_string_2,changed.to.getName());            // the name of the menu options to the variable line 2
myGLCD.print(Nokia_LCD_string_1, LEFT, 32);                     // and display the first line of
//    tft.setCursor(15,0);myGLCD.print("*");                   // draw a star
//    tft.setCursor(0,1);tft.write(6);                      // the second line and arrow arrowBack
myGLCD.print(changed.to.getName(), LEFT, 32);                // display the grandson of the second line
//    tft.setCursor(15,1);tft.write(4);                     // arrow to the right because they are the grandchildren
    }
   
    if(c==4)                                              // if grandchild (shortkey = 4) are:
    {InMenu =true;
//    myGLCD.print("*");                                       // draw a star
myGLCD.print(Nokia_LCD_string_2, LEFT, 32);                    // in the first line of the display child (or parent grandchild)
//    tft.setCursor(15,0);myGLCD.print("*");                   // draw a star
//    tft.setCursor(0,1);tft.write(6);                      // the second line and arrow arrowBack
myGLCD.print(changed.to.getName(), LEFT, 32);                 // display grandson
//    tft.setCursor(15,1);tft.write(7);                     // arrow up-down
    }
}

// --- 5 analog buttons keyboard scan version DFRobot --------------------------------------
volatile int Read_keyboard(int analog)
{
int stan_Analog = analogRead(analog);delay(30);
//   int stan_Analog = analogRead(analog);delay(30);//Serial.println(stan_Analog);
   if (stan_Analog > 1000) return -1; // limit
   if (stan_Analog < 50)   return 3;  // right
//   if (stan_Analog < 200)  return 1;  // up
//   if (stan_Analog < 400)  return 2;  // down
   if (stan_Analog < 600)  return 0;  // left
   if (stan_Analog < 800)  return 4;  // OK
   return -1;                         // Not pressed
}
// ============================================================================================
char buffer[10];
//
void setup()
{
   pinMode(BackLight_Pin, OUTPUT);
  digitalWrite(BackLight_Pin,HIGH);
    analogWrite(BackLight_Pin,255);
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myGLCD.print("SauleVire.lt",CENTER,8);
 
    myGLCD.print("v1.3",CENTER,24);
  delay(5000);
 LoadConfig();

  /* ********************************************************* */

 
  analogWrite(BackLight_Pin,lcd_backlight*25);
  Nokia_LCD_string_1=new char[16];
  Nokia_LCD_string_2=new char[16];
                       
   
Serial.begin(9600);
   Collector_sensor.begin();Boiler_sensor.begin();
   Thermostat_sensor.begin();
  
  pinMode(Relay_Collector,OUTPUT);pinMode(Relay_Thermostat,OUTPUT);
  digitalWrite(Relay_Collector,HIGH);digitalWrite(Relay_Thermostat,HIGH);
  menuSetup();
  Temperature_measurements_1();
 
    LCD_switching_on_Time = millis();
    temperature_measurement_time_1 = millis();
//---------------------------------------------------------



LCD_TFT_template();
  delay(500);
  myGLCD.clrScr();

//---------------------------------------------------------
setSyncProvider(RTC.get); 
  }  // setup() ...************ END **************...
  // ************************ START void loop() *******************************
void loop()   
{
  

// If the menu is inactive for some time, it returns to the main program 
  if(timerEnable == 1 && millis() - menuTimer >=Auto_return_from_meniu_1){
    #ifdef DEBUGSerialPrint
    Serial.println(F("Timer Clear........."));
     delay(30);
    #endif
menu.moveLeft();
menu.moveUp();
menu.getRoot();
    timerEnable = 0;
    LCD_TFT_template();
    TFT_Temperature_Imaging();
  } 
 
 
// measured temperature specified time intervals (Temperature_measurement_interval)
/* +++++++++++++++++++++++++++ First level ++++++++++++++++++++++++++++++++++++ */
if (millis() > temperature_measurement_time_1 ) {
  temperature_measurement_time_1 = millis() + Temperature_measurement_interval_1;
  Temperature_measurements_1();}

   
// if the screen, without application of the button illuminates more than the tasks, backlight off
      if (millis()- LCD_switching_on_Time > The_LCD_light_Break) {
      analogWrite(BackLight_Pin, 0);
       pinMode(13,OUTPUT);digitalWrite(13,LOW); // only for test
      Backlighting = false;
      LCD_switching_on_Time = millis();}
 // When you press any key, the screen backlight is turned on when it is turned off
if ((buttonPressed != -1) && (Backlighting == false)){ analogWrite(BackLight_Pin,lcd_backlight*25);
                                            digitalWrite(13,HIGH); // only for test
                                            Backlighting = true;}


//********************************************************************
  buttonPressed=Read_keyboard(Key_Pin);delay(30);       // read the state of the keyboard:
  if(Keyboard_change!=buttonPressed)     {              // if there was a change in the state are:
  navMenu();
  }
Keyboard_change=buttonPressed;                 //Assign the value of x variable amended so that the long pressing the

//********************************************************************
// If you are not currently within the menu is of a continuous program
if (InMenu == false){
  
 //********************************************************************
if(hour() < 10) {myGLCD.print("0", 0, 24); myGLCD.printNumI(hour(),6, 24);}
  else myGLCD.printNumI(hour(),0, 24);
bBlink = ((bBlink) ? false : true);
if (bBlink)
       myGLCD.print(":", 12, 24);
    else
       myGLCD.print(" ", 12, 24);
if(minute() < 10) {myGLCD.print("0", 18, 24); myGLCD.printNumI(minute(), 24, 24);}
  else {myGLCD.printNumI(minute(), 18, 24);}
//********************************************************************
  // time interval used for the LCD refresh
  if (millis() > LCD_Update_Time ) {
  LCD_Update_Time = millis() + LCD_Update_Interval;
  LCD_TFT_template();
  TFT_Temperature_Imaging();

  //#ifdef DEBUGSerialPrint
//Serial.println("Temperate_measurement");
//unsigned long start = millis();
//#endif
 
#ifdef DEBUGSerialPrint
//unsigned long stop = millis();
//Serial.print("Temperature measurement time: ");  Serial.println(stop - start);
Serial.print("K/ ");Serial.print(K);Serial.print(" B/ ");Serial.print(B);Serial.print(" T/ ");Serial.println(T);
Serial.println("----");
Serial.print("Thermostat_status- ");Serial.println(Thermostat_status);
Serial.print("temperature_1- ");Serial.print(temperature_1);
Serial.print("  temperature_2- ");Serial.println(temperature_2);
Serial.println("----");
Serial.print("Pump_power_on_difference- ");Serial.print(Pump_power_on_difference);
Serial.print("  Pump_power_off_difference- ");Serial.println(Pump_power_off_difference);

Serial.print("millis- ");Serial.println(millis()/1000);
Serial.println(freeRam());

#endif
Serial.print("millis- ");Serial.println(millis()/1000);
Serial.println(freeRam());
  }
}


//------------------ collector pump and thermostat control -----------------------//
if (millis() > Relay_switching_time )
 {
   Relay_switching_time=millis()+Relay_switching_interval;
if (Manual_pump_status == true)  {digitalWrite(Relay_Collector,LOW);}
else{
   if (K-B>=Pump_power_on_difference) {digitalWrite(Relay_Collector,LOW);
    }
   if (K-B<=Pump_power_off_difference) {digitalWrite(Relay_Collector,HIGH);
 }
    }
  if (Thermostat_status == 1)
   {// If the mode= heating (Thermostat_status = 1)
    if (T <= temperature_1) digitalWrite(Relay_Thermostat,LOW); 
      else digitalWrite(Relay_Thermostat,HIGH);
Serial.print(T);Serial.print("-*-");Serial.print(temperature_1);Serial.println(" Status=1 , sildymas ");
   }
   if (Thermostat_status == 2)
    {// If the mode= freezing (Thermostat_status = 2)
     if (T >= temperature_1) digitalWrite(Relay_Thermostat,LOW); 
       else digitalWrite(Relay_Thermostat,HIGH);
Serial.print(T);Serial.print("-*-");Serial.print(temperature_1);Serial.println(" Status=2 ");
    }
    if (Thermostat_status == 3){
     // If you do not need a second relay mode= off
     digitalWrite(Relay_Thermostat,HIGH);
Serial.print(T);Serial.print("-*-");Serial.print(temperature_1);Serial.println(" Status=3 ");}
 }
}
// === END ===========================================================
////////////////////////////////////////////////////////////////////////
void Temperature_measurements_1(){
  //____________________________ Start Sensor 1 _________________________________
#ifdef SetWaitForConversionFALSE
  Collector_sensor.setWaitForConversion(false);  // makes it async
#endif
  Collector_sensor.requestTemperatures(); // Send the command to get temperatures
  K=Collector_sensor.getTempCByIndex(0);
//_____________________________ Stop Sensor 1 ___________________________________
  //______________________ Start Sensor 3 ________________________________________
  #ifdef SetWaitForConversionFALSE
  Thermostat_sensor.setWaitForConversion(false);  // makes it async
#endif
  Thermostat_sensor.requestTemperatures(); // Send the command to get temperatures
T=Thermostat_sensor.getTempCByIndex(0);
//___________________ Stop Sensor 3 ______________________________________________
//__________________________________________ Start Sensor 2 _____________________
#ifdef SetWaitForConversionFALSE
  Boiler_sensor.setWaitForConversion(false);  // makes it async
#endif
  Boiler_sensor.requestTemperatures(); // Send the command to get temperatures
  B=Boiler_sensor.getTempCByIndex(0);
//_____________________________________ Stop Sensor 2 ____________________________
}




void LCD_TFT_template(){
myGLCD.print("Kolektor. ", 0, 0);
myGLCD.print("Boileris  ", 0, 8);
myGLCD.print("TermStat. ", 0, 16);
myGLCD.invert(true);
//myGLCD.print("______________", 0, 24);
myGLCD.invert(false);
myGLCD.print("Siurblys", 0, 32);
   if (K-B>=Pump_power_on_difference) myGLCD.print("ON ", 60, 32);
   if (K-B<=Pump_power_off_difference) myGLCD.print("OFF", 60, 32);
//myGLCD.print("freeRam", 0, 36);myGLCD.printNumI(freeRam(), 54, 36);
myGLCD.print("T- ", 0, 40); 
    if (Thermostat_status == 1) myGLCD.print("sildymas", 18, 40); // heating
    if (Thermostat_status == 2) myGLCD.print("saldymas", 18, 40); // freezing
    if (Thermostat_status == 3) myGLCD.print("isjungta", 18, 40); // turned off
}

void TFT_Temperature_Imaging(){
//  myGLCD.print("SauleVire.lt",CENTER,0);
  myGLCD.printNumF(K,1,RIGHT,0);
  myGLCD.printNumF(B,1,RIGHT,8);
  myGLCD.printNumF(T,1,RIGHT,16);
//   myGLCD.print(temp.read_temp(),2);
}
   int MeniuFunkcija (String text_1, int  Converted_Value, int Max_Value, int Min_Value, String text_2)
            {
        //tft.write(7);    
       /* tft.setCursor(1,1);*/myGLCD.print(text_1,0,40); //("Nustatyta=   ");
        myGLCD.printNumI( Converted_Value,70,40); // shows the current value
        int  action=-1; delay(1000);         //
                                          
        while(action!=4)                   //
         {
           Keyboard_change=-1;
           action=Read_keyboard(Key_Pin); //delay(300); 
//             action=go_out_from_menu(); 
           if(Keyboard_change!=action)          
             {
             if (action==3) { Converted_Value++; if( Converted_Value>Max_Value)  Converted_Value=Max_Value; 
                                                    if( Converted_Value<10) myGLCD.print(" ",77,40);
                                                      myGLCD.printNumI( Converted_Value,70,40); delay(200);}
             if(action==0)  { Converted_Value--; if( Converted_Value<Min_Value)  Converted_Value=Min_Value; 
                                                     if( Converted_Value<10) myGLCD.print(" ",77,40);
                                                       myGLCD.printNumI( Converted_Value,70,40); delay(200);}
             if(action==4) // 0
               {
                  myGLCD.print(text_2,LEFT,32); delay(2000); // 0
                  myGLCD.print("                ",0,40); // 0
                  myGLCD.print(Nokia_LCD_string_1,CENTER,24);           // reconstruct the previous state at LCD
                 menu.moveDown();
               }
             }
         } Keyboard_change=action;  // Keyboard_change update, in order to react only Keyboard_change keyboard status
         // This is an important moment - while loop ends and turn the control to the main loop loop ()
         return  Converted_Value;
      }
// Scan settings
boolean LoadConfig(){
  if ((EEPROM.read(0) == 27) && (EEPROM.read(1) == 28) &&
     (EEPROM.read(2) == 13) && (EEPROM.read(3) == 18)) {

    if (EEPROM.read(4) == EEPROM.read(5)) Pump_power_on_difference = EEPROM.read(4); 
    if (EEPROM.read(6) == EEPROM.read(7)) Pump_power_off_difference = EEPROM.read(6);
    if (EEPROM.read(8) == EEPROM.read(9)) temperature_1 = EEPROM.read(8); 
    if (EEPROM.read(10) == EEPROM.read(11)) temperature_2 = EEPROM.read(10);
    if (EEPROM.read(12) == EEPROM.read(13)) Thermostat_status = EEPROM.read(12);
    if (EEPROM.read(14) == EEPROM.read(15)) lcd_backlight = EEPROM.read(14);
    return true;
  }
  return false;
}
// Write settings
void SaveConfig(){
  EEPROM.write(0,27);
  EEPROM.write(1,28);
  EEPROM.write(2,13);
  EEPROM.write(3,18);
  EEPROM.write(4,Pump_power_on_difference);EEPROM.write(5,Pump_power_on_difference);  //
  EEPROM.write(6,Pump_power_off_difference); EEPROM.write(7,Pump_power_off_difference);  //
  EEPROM.write(8,temperature_1);EEPROM.write(9,temperature_1);  //
  EEPROM.write(10,temperature_2); EEPROM.write(11,temperature_2);  //
  EEPROM.write(12,Thermostat_status); EEPROM.write(13,Thermostat_status);  //
  EEPROM.write(14,lcd_backlight); EEPROM.write(15,lcd_backlight);  //

}

//void flashbacklight() {
//  digitalWrite(BackLight_Pin, LOW);  delay(150);
//  digitalWrite(BackLight_Pin, HIGH); delay(150);
//}
/////////////////////////////////////////
void navMenu(){
  //MenuItem currentMenu=menu.getCurrent();
  switch (buttonPressed){
  case 4: //select    
  //The current item has an element right, it's a sub menu so nav right.
    if (menu.getCurrent().getRight() != 0){  menu.moveRight();
    #ifdef DEBUGSerialPrint
                  Serial.print(menu.getCurrent().getName()); Serial.println(" has menu right");
     #endif
   }
    else{  //otherwise, menu has no child and has been pressed. enter the current menu
          menu.use();} break;     //select   
  case 3: menu.moveDown(); break; //right
  case 0:
        if (menu.getCurrent().getRight() != 0){  menu.moveUp();
        #ifdef DEBUGSerialPrint
                  Serial.print(menu.getCurrent().getName());
                Serial.println(" has SUMmenu right");
              //USART_putstring(" has SUMmenu right\n");
        #endif
            }
    else{  //otherwise, menu has no child and has been pressed. enter the current menu
 
  menu.moveLeft();}break;    //left
  }
}
////////////////////////////////////////////
int go_out_from_menu()
 {  if(timerEnable == 1 && millis() - menuTimer >=Auto_return_from_meniu_1){
return 4;
 } else {
  return Read_keyboard(Key_Pin); 
 }
 } 
 ///////////////////////////////////////////////////
 
//void printDigits(int digits, char skirtukas)

//{
  // utility function for digital clock display: prints preceding colon and leading 0
//  Serial.print("simbolis");
//    myGLCD.print(skirtukas, 7, 32);
//  if(digits < 10) myGLCD.print("0", 7, 32);
//  myGLCD.printNumI(digits, 7, 32);

//}
