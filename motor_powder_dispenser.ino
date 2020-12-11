                                                                                    //Roman Zvoda and Patrik Bandik

//libraries 
#include <TouchScreen.h> 
#include <LCDWIKI_GUI.h> 
#include <LCDWIKI_KBV.h> 
#include <stdio.h>
#include <string.h>
#include <HX711_ADC.h>
#include <EEPROM.h>

LCDWIKI_KBV my_lcd(ILI9341,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset

//color defines                     
#define BLACK        0x0000  
#define RED          0xF800  
#define GREEN        0x07E0 
#define DARKGREY     0x737373  
#define WHITE        0xFFFF   
#define PURPLE       0x780F    
#define PINK         0xF81F  
#define ORANGE       0xFF6C

//button settings
#define BUTTON_R 25 
#define BUTTON_SPACING_X 25 
#define BUTTON_SPACING_Y 5  
#define EDG_Y 5 
#define EDG_X 20 

#define YP A3  
#define XM A2 
#define YM 9   
#define XP 8   

//touch sensitivity for X
#define TS_MINX 911
#define TS_MAXX 117

//touch sensitivity for Y
#define TS_MINY 87
#define TS_MAXY 906

// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65

//touch sensitivity for press
#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//mnotor variables
const int motorPin = 44; // white motor 
const int motorPin2 = 46; //brown motor

int Speed = 0; //Variable to store Speed
int flag;

//pins for Load cell
const int HX711_dout = 20; //mcu > HX711 dout pin
const int HX711_sck = 21; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
long t;

//button struct
typedef struct _button_info
{
  uint8_t button_name[10];
  uint8_t button_name_size;
  uint16_t button_name_colour;
  uint16_t button_colour;
  uint16_t button_x;
  uint16_t button_y;

} button_info;

//number layout 
button_info phone_button[12] = 
{
  "1",3,BLACK,ORANGE,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-4*BUTTON_SPACING_Y-9*BUTTON_R-1,
  "2",3,BLACK,ORANGE,EDG_X+3*BUTTON_R+BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-4*BUTTON_SPACING_Y-9*BUTTON_R-1,
  "3",3,BLACK,ORANGE,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-4*BUTTON_SPACING_Y-9*BUTTON_R-1,
  "4",3,BLACK,ORANGE,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-3*BUTTON_SPACING_Y-7*BUTTON_R-1, 
  "5",3,BLACK,ORANGE,EDG_X+3*BUTTON_R+BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-3*BUTTON_SPACING_Y-7*BUTTON_R-1,
  "6",3,BLACK,ORANGE,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-3*BUTTON_SPACING_Y-7*BUTTON_R-1,
  "7",3,BLACK,ORANGE,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-2*BUTTON_SPACING_Y-5*BUTTON_R-1,
  "8",3,BLACK,ORANGE,EDG_X+3*BUTTON_R+BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-2*BUTTON_SPACING_Y-5*BUTTON_R-1,
  "9",3,BLACK,ORANGE,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-2*BUTTON_SPACING_Y-5*BUTTON_R-1,
  "0",3,BLACK,ORANGE,EDG_X+3*BUTTON_R+BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-BUTTON_SPACING_Y-3*BUTTON_R-1,
  "RUN",2,BLACK,GREEN,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-BUTTON_R-1,
  "DEL",2,BLACK,RED,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-BUTTON_R-1,
};

//powder select layout
button_info phone_menu[5] = 
{
  "Choose:",3,PURPLE,WHITE,EDG_X+3*BUTTON_R+BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-4*BUTTON_SPACING_Y-9*BUTTON_R-1,
  "Protein",2,BLACK,WHITE,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-3*BUTTON_SPACING_Y-7*BUTTON_R-1, 
  "Flour",2,BLACK,WHITE,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-3*BUTTON_SPACING_Y-7*BUTTON_R-1,
  "1",3,BLACK,ORANGE,EDG_X+BUTTON_R-1,my_lcd.Get_Display_Height()-EDG_Y-2*BUTTON_SPACING_Y-5*BUTTON_R-1,
  "2",3,BLACK,ORANGE,EDG_X+5*BUTTON_R+2*BUTTON_SPACING_X-1,my_lcd.Get_Display_Height()-EDG_Y-2*BUTTON_SPACING_Y-5*BUTTON_R-1,
  
};

//display string
void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,boolean mode)
{
    
  my_lcd.Set_Text_Mode(mode);
  my_lcd.Set_Text_Size(csize);
  my_lcd.Set_Text_colour(fc);
  my_lcd.Set_Text_Back_colour(bc);
  my_lcd.Print_String(str,x,y);
}

//press function
boolean is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py)
{
  if((px > x1 && px < x2) && (py > y1 && py < y2))
  {
    return true;  
  } 
  else
  {
    return false;  
  }
 }

//display the main menu
void show_menu(void)
{
  uint16_t i;
  for(i = 0;i < sizeof(phone_button)/sizeof(button_info);i++)
  {
    my_lcd.Set_Draw_color(phone_button[i].button_colour);
    my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
    show_string(phone_button[i].button_name,phone_button[i].button_x-strlen(phone_button[i].button_name)*phone_button[i].button_name_size*6/2+1,phone_button[i].button_y-phone_button[i].button_name_size*8/2+1,phone_button[i].button_name_size,phone_button[i].button_name_colour,BLACK,1);
  }
  my_lcd.Set_Draw_color(BLACK);
  my_lcd.Fill_Rectangle(1, 1, my_lcd.Get_Display_Width()-2, 3);
  my_lcd.Fill_Rectangle(1, 29, my_lcd.Get_Display_Width()-2, 31);
  my_lcd.Fill_Rectangle(1, 1, 3, 31);
  my_lcd.Fill_Rectangle(my_lcd.Get_Display_Width()-4, 1, my_lcd.Get_Display_Width()-2, 31);
}

//display option menu                      
void button_menu(void)
{
    uint16_t i;
   for(i = 0;i < sizeof(phone_menu)/sizeof(button_info);i++)
   {
      my_lcd.Set_Draw_color(phone_menu[i].button_colour);
      my_lcd.Fill_Circle(phone_menu[i].button_x, phone_menu[i].button_y, BUTTON_R);
      show_string(phone_menu[i].button_name,phone_menu[i].button_x-strlen(phone_menu[i].button_name)*phone_menu[i].button_name_size*6/2+1,phone_menu[i].button_y-phone_menu[i].button_name_size*8/2+1,phone_menu[i].button_name_size,phone_menu[i].button_name_colour,BLACK,1);
   }
}
                            

void setup()
{
  float init_load;
  Serial.begin(9600); delay(10);

  pinMode(motorPin, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  my_lcd.Init_LCD();
  my_lcd.Fill_Screen(WHITE); 

  Serial.println("Starting...");
  LoadCell.begin();
}

uint16_t text_x=10,text_y=6,text_x_add = 6*phone_button[0].button_name_size,text_y_add = 8*phone_button[0].button_name_size;
uint16_t n=0;

void loop()
{ 
  //calibration of load cell
  my_lcd.Init_LCD();
  my_lcd.Fill_Screen(WHITE); 

  float calibrationValue;
  calibrationValue = 696.0;
  #if defined(ESP8266)|| defined(ESP32) //may not be needed
  #endif
  EEPROM.get(calVal_eepromAdress, calibrationValue);
  long stabilizingtime = 2000; 
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else 
  {
    LoadCell.setCalFactor(calibrationValue); 
    Serial.println("Startup is complete");
  }

  Serial.println("Start");

  // variable defines
  float init_load = 0;
  float cup_weight = 0;
  int size_of_string = 0;
  static boolean newDataReady = 0;
  const int serialPrintInterval = 20; 

  analogWrite(motorPin, Speed); //may not be needed

  //initial screen load
  my_lcd.Set_Draw_color(WHITE);
  my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);
  show_string("Place Cup",CENTER,150,3,DARKGREY, BLACK,1);
  
  //wait for cup placement
  while(init_load < 2)
  {
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady) {
      if (millis() > t + serialPrintInterval)
      {
        init_load = LoadCell.getData();
        Serial.print("Load value: ");
        Serial.println(init_load);
        newDataReady = 0;
        t = millis() + 1500;
      }
    }
  }

  delay(1000);
  cup_weight = LoadCell.getData();
  Serial.print("Cup Weight: ");
  Serial.println(cup_weight);

  //display option menu
  my_lcd.Fill_Screen(WHITE);
  button_menu();
  Serial.println("Menu shown.");
  //int amount = 0;

  // setting variables to default values
  boolean set = false;
  uint16_t i;
  uint8_t motor = 0;

  //wait for option 
  while(set == false)
  {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);  
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
   
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
      //p.x = my_lcd.Get_Display_Width()-map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(), 0);
      //p.y = my_lcd.Get_Display_Height()-map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(), 0);
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, my_lcd.Get_Display_Width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, my_lcd.Get_Display_Height());
      
      for(i=0;i<sizeof(phone_menu)/sizeof(button_info);i++)
      {
        
           //press the button
           if(is_pressed(phone_menu[i].button_x-BUTTON_R,phone_menu[i].button_y-BUTTON_R,phone_menu[i].button_x+BUTTON_R,phone_menu[i].button_y+BUTTON_R,p.x,p.y))
           {
                my_lcd.Set_Draw_color(ORANGE);
                my_lcd.Fill_Circle(phone_menu[i].button_x, phone_menu[i].button_y, BUTTON_R);
                
                if ( i == 3)
                {
                  Serial.println("First button pressed.");
                  set = true;
                  motor = 1;
                }
                else if ( i == 4)
                {
                  Serial.println("Second button pressed.");
                  set = true;
                  motor = 2;
                }
            }
      }
    }
  }
  
  //may not be needed
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

 
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

 
  my_lcd.Set_Draw_color(WHITE);
  my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);

  //display number menu
  show_menu();
  String grams = ""; //string for numbers
  float input_grams = 0.0;
  float percentage = 0.0;

  while(i != 10)
  {
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);  
    int amount = 0;
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
      
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, my_lcd.Get_Display_Width());
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, my_lcd.Get_Display_Height());
      
      for(i=0;i<sizeof(phone_button)/sizeof(button_info);i++)
      {
        
        //press the button
        if(is_pressed(phone_button[i].button_x-BUTTON_R,phone_button[i].button_y-BUTTON_R,phone_button[i].button_x+BUTTON_R,phone_button[i].button_y+BUTTON_R,p.x,p.y))
        {
          my_lcd.Set_Draw_color(DARKGREY);
          my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
          show_string(phone_button[i].button_name,phone_button[i].button_x-strlen(phone_button[i].button_name)*phone_button[i].button_name_size*6/2+1,phone_button[i].button_y-phone_button[i].button_name_size*8/2+1,phone_button[i].button_name_size,WHITE,BLACK,1);
        
          //insert entered numbers to string
          if (*phone_button[i].button_name == 49)
          {
            
            Serial.println("1");   //check for concat
            grams.concat(1); 
            Serial.println(grams);
          
          }
          else if (*phone_button[i].button_name == 50)
          {
            Serial.println("2");
            grams.concat(2);
            Serial.println(grams);
          
          }
          else if (*phone_button[i].button_name == 51)
          {
            Serial.println("3");
            grams.concat(3);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 52)
          {
            Serial.println("4");
            grams.concat(4);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 53) 
          {
            Serial.println("5");
            grams.concat(5);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 54)
          {
            Serial.println("6");
            grams.concat(6);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 55)
          {
            Serial.println("7");
            grams.concat(7);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 56)
          {
            Serial.println("8");
            grams.concat(8);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 57)
          {
            Serial.println("9");
            grams.concat(9);
            Serial.println(grams);
          }
          else if (*phone_button[i].button_name == 48)
          {
            Serial.println("0");
            grams.concat(0);
            Serial.println(grams);
          }
          
          delay(100);

          my_lcd.Set_Draw_color(phone_button[i].button_colour);
          my_lcd.Fill_Circle(phone_button[i].button_x, phone_button[i].button_y, BUTTON_R);
          show_string(phone_button[i].button_name,phone_button[i].button_x-strlen(phone_button[i].button_name)*phone_button[i].button_name_size*6/2+1,phone_button[i].button_y-phone_button[i].button_name_size*8/2+1,phone_button[i].button_name_size,phone_button[i].button_name_colour,BLACK,1);  
          
          if(i < 10)
          {
              if(n < 11)
              {
                show_string(phone_button[i].button_name,text_x,text_y,phone_button[i].button_name_size,PURPLE, BLACK,1);
                text_x += text_x_add-1;
                Serial.println("Insert");
                n++;
              }
          }

          // run pressed
          else if(10 == i) 
          { 
              Serial.println(grams);

              //display action
              my_lcd.Set_Draw_color(WHITE);
              my_lcd.Fill_Screen(WHITE);

              my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);
              show_string("Dispensing...",CENTER,144,2,DARKGREY, BLACK,1); 
              Serial.println("Run pressed");

              //calculations for final grams
              input_grams = grams.toInt();
              float together = cup_weight + input_grams;
              int add_grams = 0;

              Serial.print("Together:");
              Serial.println(together);
              delay(1000);
              LoadCell.begin();

              //calibrate scale for measurements
              float calibrationValue;
              calibrationValue = 696.0; 
              #if defined(ESP8266)|| defined(ESP32)
              #endif
              EEPROM.get(calVal_eepromAdress, calibrationValue);

              long stabilizingtime = 2000; 
              boolean _tare = true; 
              LoadCell.start(stabilizingtime, _tare);
              
              //run till desired grams are reached
              while(add_grams <= input_grams)
              {
                if (LoadCell.update()) newDataReady = true;
                if (newDataReady) 
                {
                  if (millis() > t + serialPrintInterval)
                  {
                    add_grams = LoadCell.getData();
                    Serial.print("Load value: ");
                    Serial.println(add_grams);
                    newDataReady = 0;
                    t = millis();
                  }
                }

                //motor start
                if (motor == 1)
                {
                  if (input_grams <= 6)
                  {
                    analogWrite(motorPin, 170);
                    Serial.println("Only half full all time.");
                  }
                  else if(input_grams >= 7)
                  {
                     percentage = input_grams * 0.8; //80% of input weight
                     while (add_grams <= percentage)
                     {
                      if (LoadCell.update()) newDataReady = true;
                      if (newDataReady) 
                      {
                        if (millis() > t + serialPrintInterval)
                        {
                          add_grams = LoadCell.getData();
                          Serial.print("Load value: ");
                          Serial.println(add_grams);
                          newDataReady = 0;
                          t = millis();
                        }
                      }
                      analogWrite(motorPin, 255);
                      Serial.print("Add grams: ");
                      Serial.println(add_grams);
                      Serial.print("Percentage: ");
                      Serial.println(percentage);
                      Serial.println("80 percent full.");
                     }
                     
                     
                     analogWrite(motorPin, 100); //slow down to last 20 percent
                     Serial.println("20 percent slow.");
                  }
                }
                else if (motor == 2)
                {
                   if (input_grams <= 6)
                  {
                    analogWrite(motorPin2, 170);
                    Serial.println("Only half full all time. mt2");
                  }
                  else if(input_grams >= 7)
                  {
                     percentage = input_grams * 0.8; //80% of input weight
                     while (add_grams <= percentage)
                     {
                      if (LoadCell.update()) newDataReady = true;
                      if (newDataReady) 
                      {
                        if (millis() > t + serialPrintInterval)
                        {
                          add_grams = LoadCell.getData();
                          Serial.print("Load value: ");
                          Serial.println(add_grams);
                          newDataReady = 0;
                          t = millis();
                        }
                      }
                      analogWrite(motorPin2, 255);
                      Serial.print("Add grams: ");
                      Serial.println(add_grams);
                      Serial.print("Percentage: ");
                      Serial.println(percentage);
                      Serial.println("80 percent full. mt2");
                     }
                     
                     
                     analogWrite(motorPin2, 100); //slow down to last 20 percent
                     Serial.println("20 percent slow. mt2");
                  }
                }
              }

              //motor stop
              if (motor == 1)
              {
                analogWrite(motorPin, 0);
              }
              else 
              {
                analogWrite(motorPin2, 0);
              }
              
              //display end of dispense
              my_lcd.Set_Draw_color(WHITE);  
              my_lcd.Set_Draw_color(WHITE);
              my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);
              show_string("Dispense ended.",CENTER,144,2,GREEN, BLACK,1);

              delay(2000);
              
              //display cup removal
              my_lcd.Set_Draw_color(WHITE);
              my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);
              show_string("Please,take your cup",CENTER,133,2,GREEN, BLACK,1);

              //wait for cup removal 
              while (add_grams >= 2)
                {
                  if (LoadCell.update()) newDataReady = true;
                  if (newDataReady)
                  {
                    if (millis() > t + serialPrintInterval)
                    {
                      add_grams = LoadCell.getData();
                      Serial.print("Load value: ");
                      Serial.println(add_grams);
                      newDataReady = 0;
                      t = millis();
                    }
                  }
                }
                
              size_of_string = grams.length();

              Serial.println("Numbers deleted.");

              //display goodbye
              my_lcd.Fill_Rectangle(0, 350, my_lcd.Get_Display_Width()-1, 42);
              show_string("Goodbye!",CENTER,150,3,GREEN, BLACK,1);


              //reset string
              grams = "";
              delay(3000);
              break;
          }

          //delete button press
          else if(11 == i) 
          {
            if(n > 0)
            { 
              //remove last character from string
              grams.remove(grams.length()-1);
              amount = grams.length();
              Serial.println(grams);
              Serial.print("Lenght of string: ");
              Serial.println(amount);
              
              //delete number from top line
              my_lcd.Set_Draw_color(WHITE);
              text_x -= (text_x_add-1);  
              my_lcd.Fill_Rectangle(text_x, text_y, text_x+text_x_add-1, text_y+text_y_add-2);
              n--;

              //display delete function 
              my_lcd.Set_Draw_color(WHITE);
              my_lcd.Fill_Rectangle(0, 33, my_lcd.Get_Display_Width()-1, 42);
              show_string("DELETE",CENTER,33,1,RED, BLACK,1);

              delay(100); 
              
              my_lcd.Set_Draw_color(WHITE);
              my_lcd.Fill_Rectangle(0, 33, my_lcd.Get_Display_Width()-1, 42);
            }
          }
        }      
      }
    }
  }
}
