/* @@@@@@@@@@@@@@@@@@@@@@ for testing @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#define SetWaitForConversionFALSE // accelerated DS18B20 temperature measurement
#define DEBUGSerialPrint // for temperature measurement testing 


/* ********************** Laikai *************************************** */
unsigned long Menu_time_spent_inactive; //Meniu_praleistas_neaktyvus_laikas
#define inactive_menu 20000
unsigned long temperature_measurement_time_1 = 0;
unsigned long Relay_switching_time = 0;
unsigned long temperature_measurement_time_3 = 0;
#define Temperature_measurement_interval_1 5000
#define Relay_switching_interval 5000
unsigned long menuTimer = millis();              
byte timerEnable = 1;
unsigned long  Auto_return_from_meniu_1 = 15000;
unsigned long  Auto_return_from_meniu_2 = 10000;

unsigned long LCD_switching_on_Time;
unsigned long  The_LCD_light_Break = 300000;
unsigned long LCD_Update_Time = 0;
#define LCD_Update_Interval 5000


/* ************** Keyboard variables ************************************* */
volatile int Keyboard_change =-1;  // Check or change the keyboard value
volatile int buttonPressed=-1;                 // 
volatile int stan_Analog;          // 
/* ********** LCD description ******************* */
int lcd_backlight = 9; // lcd_backlight * 25 = MAX LCD backlight
//byte LCD_brightness = 10; // lcd_pasvietimas * 25 = MAX LCD apsviestumas
boolean Backlighting = true; // mark on the screen backlight
// The pump on indicator (arrow up)
byte arrow_up[8]={ B00100,B01110,B11111,B00100,B00100,B00100,B00100,B00100};
// Pump shut-off symbol (arrow in the bottom)
byte arrow_down[8]={ B00100,B00100,B00100,B00100,B00100,B11111,B01110,B00100};

/* ******************** Relay *********************************** */
#define Relay_Collector A1 // Collector
#define Relay_Thermostat A2 // Thermostas
/* ************************** davikliai *********************** */

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire K_t(ONE_WIRE_BUS1); // Collector
OneWire B_t(ONE_WIRE_BUS2); // Boiler
OneWire T_t(ONE_WIRE_BUS3); // Thermostat

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature Collector_sensor(&K_t);
DallasTemperature Boiler_sensor(&B_t);
DallasTemperature Thermostat_sensor(&T_t);

// variables to store the measured temperature values
float K,B,T; 

byte Pump_power_on_difference = 5;
byte Pump_power_off_difference = 3;
boolean k_uzsalimas = false;
boolean S_C_pump_manual_control = false; // Mark  manual pump control 
/* ********** Thermostat variables ******************* */
byte temperature_1 = 20;
byte temperature_2 = 25;
byte Thermostat_status =3;
/* ********** Pump variables ******************* */
boolean Manual_pump_status = false; //false- manual off, true- manual-on
