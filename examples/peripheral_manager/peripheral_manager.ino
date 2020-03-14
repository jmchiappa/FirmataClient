#include <FirmataClient.h>

HardwareSerial SerialFirmata(PA_3, PA_2);  // USART 2;

// feature definition
// Select any define below to activate some sysex commands

// #define PROXISENSOR_VL6180X
//#define PROXISENSOR_VL53L0X
//#define STM_STEPPER
//#define WS2812_SCREEN
//#define TCS34725_COLORVIEW
//#define LSM6DSL_ACCGYR
#define OLED
// 16 bits-map that defines which peripheral is present, auto-generated by application
#define REGISTERED_PERIPHERAL  0x0063

#if defined(PROXISENSOR_VL53L0X) && defined(PROXISENSOR_VL6180X)
#error "You can't set both proximity sensors. Please select only one."
#endif

#if !defined(PROXISENSOR_VL53L0X) && !defined(PROXISENSOR_VL6180X)
#warning "You didn't selected a proximity sensor. This sensor will not be able."
#endif

// End of feature definition

#ifdef PROXISENSOR_VL6180X
#pragma message ( "VL6180X library selected" )
#include <VL6180X.h>
VL6180X ProximitySensor;
#endif

#if defined(PROXISENSOR_VL53L0X) || defined(LSM6DSL_ACCGYR)
TwoWire WIRE1(PB11, PB10);  //SDA=PB11 & SCL=PB10
bool is_pin_protected(byte p){
  return ((p==38)||(p==39)||(p==33)||(p==34));
}
#endif

#ifdef PROXISENSOR_VL53L0X
#pragma message ( "VL53l0X library selected" )
#include <vl53l0x_class.h>
VL53L0X ProximitySensor(&WIRE1, PC6, PC7); //XSHUT=PC6 & INT=PC7
#endif

#ifdef STM_STEPPER
#pragma message ( "stepper library selected" )
#include <Stepper.h>
Stepper MoteurGauche;
Stepper MoteurDroite;
#else
#pragma message ( "stepper library not selected" )
#endif

#ifdef WS2812_SCREEN
#pragma message ( "ws2812b library selected" )
#include <ws2812.h>
WS2812B WS2812Screen;
static uint8_t BitmapBuffer[MATRIX_NB_COLUMN * MATRIX_NB_ROW * 4];
#endif

#ifdef TCS34725_COLORVIEW
#pragma message ( "tcs34725 library selected" )
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X,NON_BLOCKING, SDA,SCL);
#endif

#ifdef LSM6DSL_ACCGYR
#pragma message ( "LSM6DSL library selected" )
#include "LSM6DSLSensor.h"
LSM6DSLSensor *AccGyr;
// TwoWire *dev_i2c;
// #define I2C2_SCL    PB10
// #define I2C2_SDA    PB11
#endif

#ifdef OLED
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);
#endif

#endif

bool StartPeripheral=false;
HardwareSerial dbg(D47,D46);

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/

typedef enum
  {
    NOT_ASSIGNED = 0,
    DIST,
    GYRO,
    COLOR
  } ident_t;

typedef struct
{
  uint32_t range[3];
  uint32_t value32[5];
  float    valuef[5];
  uint32_t color[3];
  int32_t  accelerometer[3];
} element_t;

element_t list;
uint32_t cnt=0;

uint16_t getRegisteredPeripheral(void){
  return (uint16_t)REGISTERED_PERIPHERAL;
}

void sysex_callback_extended(byte command, byte argc, byte *argv){
  switch(command) {
  #ifdef PROXISENSOR_VL6180X
  // if a proximity sensor has been defined, this command is allowed. otherwhise, not
    case SYSEX_CMD_PROXIMITY: // Special command id, VL6180X I2C access or VL53X0 Time of Flight
    if((argv[1]&0x01)==0x01)
    {
      if(StartPeripheral==false)
      {
        ProximitySensor.init();
        delay(1);
        ProximitySensor.configureDefault();
        ProximitySensor.setScaling(SCALING);
        ProximitySensor.setTimeout(500);
        StartPeripheral=true;
      }
    }
    else
      StartPeripheral=false;
      break;      
  #endif

  #ifdef PROXISENSOR_VL53L0X
  // if a proximity sensor has been defined, this command is allowed. otherwhise, not
    case SYSEX_CMD_PROXIMITY: // Special command id, VL6180X I2C access or VL53X0 Time of Flight
    if((argv[1]&0x01)==0x01)
    {
      if(StartPeripheral==false)
      {
        // enableI2CPins();
        StartPeripheral=true;
      }
    }
    else
      StartPeripheral=false;
      break;      
  #endif

  #ifdef STM_STEPPER
    case SYSEX_CMD_STEPPER: // special command for step motor DRV8825
      // for(uint8_t len=0;len<argc;len++)
      // {
      //   Serial1.print(" 0x");
      //   Serial1.print(argv[len], HEX);
      // }
      // Serial1.println();
      argv = &(argv[1]);  // supprime le deuxième byte inutile de la commande qui est argv[0] 
      argc=decodeByteStream(argc,argv); // decode le stream
      // Serial1.println(argc);
      // for(uint8_t len=0;len<argc;len++)
      // {
      //   Serial1.print(" 0x");
      //   Serial1.print(argv[len], HEX);
      // }
      // Serial1.println();
      switch(argv[0])
      {
        case 0x00: {
          // data[2] : Nb step per rotation droite MSB
          // data[3] : nb step LSB
          // data[3] : EnaMoteurDroite
          // data[4] : StepMoteurDroite
          // data[5] : DirMoteurDroite
          // data[6] : M0MoteurDroite
          // data[7] : M1MoteurDroite
          // data[8] : Nb step per rotation gauche MSB
          // data[9] : Nb step LSB
          // data[9] : EnaMoteurGauche
          // data[10] : StepMoteurGauche
          // data[11] : DirMoteurGauche
          // data[12] : M0MoteurGauche
          // data[13] : M1MoteurGauche

          setPinModeCallback(argv[3], OUTPUT); //EnaDroite
          setPinModeCallback(argv[4], OUTPUT); //StepDroite
          setPinModeCallback(argv[5], OUTPUT); //DirDroite
          setPinModeCallback(argv[6], OUTPUT); //M0Droite
          setPinModeCallback(argv[7], OUTPUT); //M1Droite
          setPinModeCallback(argv[10], OUTPUT); //EnaGauche
          setPinModeCallback(argv[11], OUTPUT); //StepGauche
          setPinModeCallback(argv[12], OUTPUT); //DirGauche
          setPinModeCallback(argv[13], OUTPUT); //M0Gauche
          setPinModeCallback(argv[14], OUTPUT); //M1Gauche
          uint16_t nbStepR=(((uint16_t)argv[1])<<8) | ((uint16_t)argv[2]);
          uint16_t nbStepL=(((uint16_t)argv[8])<<8) | ((uint16_t)argv[9]);
          // Serial1.print("step droit=");
          // Serial1.println(nbStepR);
          // Serial1.print("step gauche=");
          // Serial1.println(nbStepL);
          MoteurDroite.attach(nbStepR, argv[3], argv[4], argv[5], argv[6] ,argv[7] );
          MoteurGauche.attach(nbStepL, argv[10],argv[11],argv[12],argv[13],argv[14]);
          // StepMotor_Test();  

          break;
        }
        case 0x01: {
          // déplacement en nombre de pas
          // data[2] : moteur : 0 : droite / 1 : gauche
          // data[3] : num steps, bits 0-6
          // data[4] : num steps, bits 7-13
          // data[5] : num steps, bits 14-20
          // data[6] : num steps, bits 21-27
          // data[7] : num steps, bits 28-32
          // Serial1.print("i");
          // Serial1.print( *(int32_t *)(&argv[2]) );
          // Serial1.print(" m");
          // Serial1.println(argv[1]);
          if(argv[1]==0) // Applicable au moteur droite
          {

            MoteurDroite.Move(*(int32_t *)(&argv[2]));
            // MoteurDroite.Move(1200);
          }
          else
          {
            MoteurGauche.Move(*(int32_t *)(&argv[2]));
            // MoteurGauche.Move(1200);
          }
          break;
        }
        case 0x02: {
          // Restart
          // Serial1.print("m=");
          // Serial1.print(argv[1]);
          if(argv[1]==0)
          {
            MoteurDroite.Move();
          }
          else
          {
            MoteurGauche.Move();
          }

          break;
        }
        case 0x03 : {
          // Set speed motor
          // Serial1.print("m=");
          // Serial1.print(argv[1]);
          // Serial1.print(" s=");
          // Serial1.println( *(uint16_t *)(&argv[2]) );
          if(argv[1]==0)
          {
            MoteurDroite.SetSpeed(*(uint16_t *)(&argv[2]));
            // MoteurDroite.SetSpeed(300);
          }
          else
          {
            MoteurGauche.SetSpeed(*(uint16_t *)(&argv[2]));
            // MoteurGauche.SetSpeed(300);
          }
          break;
        }
        case 0x04 : { // SetDirPolarity
          // Serial1.print("polarity=");
          // Serial1.println( *(uint8_t *)(&argv[2]) );
          if(argv[1]==0)
          {
            MoteurDroite.SetDirPolarity(argv[2]);
          }
          else
          {
            MoteurGauche.SetDirPolarity(argv[2]);
          }
          break;
        }
        case 0x05 : {// Stop immediately
          // Serial1.print("stop motor ");
          // Serial1.println( *(uint8_t *)(&argv[1]) );
          if(argv[1]==0)
          {
            MoteurDroite.Stop();
          }
          else
          {
            MoteurGauche.Stop();
          }
          break;
        }
        case 0x06 : {//Deactivate
          // Serial1.print("deactivate motor ");
          // Serial1.println( *(uint8_t *)(&argv[1]) );
          if(argv[1]==0)
          {
            MoteurDroite.Deactivate();
          }
          else
          {
            MoteurGauche.Deactivate();
          }
          break;
        }
        default:
          // Serial1.println("unknown step motor command");
          break;
    }
  #endif    

  #ifdef WS2812_SCREEN
    case SYSEX_CMD_STRIPLED : // special ID for ws2812 strip LED as 8x8 matrix
      argv = &(argv[1]);  // supprime le deuxième byte inutile de la commande     
  //      argc=decodeByteStream(argc,argv); // decode le stream

      switch(argv[0])     // traite la commande demandée
      {
        case 0x00 : // Clear background
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.Clear();
          break;

        case 0x01 : // Clear a layer
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.Clear(argv[1]);
          break;
        case 0x02 :  // Fill
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.Fill(argv[1],argv[2],argv[3],argv[4],argv[5]);
          break;
        case 0x03 : // SetPixelAt
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.SetPixelAt(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);
          break;
        case 0x04 : // SetBrightness
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.SetBrightness(argv[1],argv[2]);
          break;
        case 0x05 : // Rectangle
          argc=decodeByteStream(argc,argv); // decode le stream
          WS2812Screen.Rectangle(argv[1],(int8_t)argv[2],(int8_t)argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9]);
          break;
        case 0x06 : // Bitmap
          memcpy(BitmapBuffer, &(argv[6]), min((int)argv[4]*argv[5]*4,(int)sizeof(BitmapBuffer)));
          WS2812Screen.Bitmap(argv[1],(int8_t)argv[2],(int8_t)argv[3],argv[4],argv[5],BitmapBuffer);
      }
    break;
  #endif

  #ifdef OLED
  case SYSEX_CMD_OLED:
/*    dbg.begin(115200);
*/
    argv = &(argv[1]);  // supprime le deuxième byte inutile de la commande qui est argv[0] 
    argc=decodeByteStream(argc,argv); // decode le stream
    pinMode(13,OUTPUT);
/*    for(uint8_t i=0;i<16;i++)
      dbg.println((int)argv[i]);
*/
    u8g2.clearBuffer();
//    digitalWrite(13, HIGH);
    switch(argv[0]) {
      case 0 : // affiche du texte
        uint8_t y = 15;
        uint8_t len = argv[1];
//        argv[2+len]=0; // end of line 
        String s = (char*) &argv[2]; // pointer to start of string
        for(uint8_t n=0;n<50;n+=10){
          if(n<len)
            u8g2.drawStr(0, y, s.substring(n,min( (int)len, (int)(n+10))).c_str() );
          y+=18;
        }
        break;
    }
    u8g2.sendBuffer();
    break;
  #endif
  }
}

bool is_pin_protected(uint8_t p){
  return (p==D12);
}

extern void enableI2CPins(void);

void reset_callback_extended(){

  #ifdef WS2812_SCREEN
    WS2812Screen.begin();
  #endif


// #ifdef PROXISENSOR_VL6180X
//   enableI2CPins();
//   Wire.begin();
// #endif

  #if defined(PROXISENSOR_VL6180X) || defined(PROXISENSOR_VL53L0X) || defined(LSM6DSL_ACCGYR) || defined(TCS34725_COLORVIEW)
  enableI2CPins();
  WIRE1.begin();
  #endif

  #ifdef TCS34725_COLORVIEW
  if(!tcs.begin()){
    WS2812Screen.SetPixelAt(4,0,0,255,255,0,0);
    
  }
  #endif

  #ifdef PROXISENSOR_VL53L0X
  ProximitySensor.VL53L0X_Off();
  if(ProximitySensor.InitSensor(0x10))
    WS2812Screen.SetPixelAt(4,1,0,255,255,0,0);
  #endif

  #ifdef LSM6DSL_ACCGYR
  // Initlialize components.
  AccGyr = new LSM6DSLSensor(&WIRE1, LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW);
  AccGyr->Enable_X();
  AccGyr->Enable_G();  
  #endif

  #ifdef OLED
   enableI2CPins();
   Wire.begin();
   u8g2.begin();
   u8g2.setFont(u8g2_font_10x20_mf);
  #endif
  StartPeripheral=false;
}

void execute_extended(void) {
  uint16_t r,g,b,c;

  if(StartPeripheral)
  {
  #ifdef PROXISENSOR_VL6180X      
    list.range[0] = (uint16_t)(ProximitySensor.readRangeSingleMillimeters());
  #endif
  #ifdef PROXISENSOR_VL53L0X
    ProximitySensor.GetDistance(&list.range[0]);
  #endif       
  }

  list.value32[0] = cnt++;
  #ifdef STM_STEPPER    
  list.value32[3] = (((uint32_t)MoteurDroite.read()) <<16) | ((uint32_t)MoteurGauche.read() &0xFFFF);
  list.value32[4] = (MoteurDroite.Ready()     & 0x01)<<3 | 
                    (MoteurGauche.Ready()     & 0x01)<<2 | 
                    (MoteurDroite.Completed() & 0x01)<<1 |
                     MoteurGauche.Completed() & 0x01;
  #endif                       

  #ifdef TCS34725_COLORVIEW
  if(tcs.isIntegrationTimeCompleted())
  {
    tcs.getRawData(&r,&g,&b,&c);
    list.color[0]=r;
    list.color[1]=g;
    list.color[2]=b;
    tcs.StartIntegrationTime();
  }
    
  #endif

  #ifdef LSM6DSL_ACCGYR
  AccGyr->Get_X_Axes(&(list.accelerometer[0]));
  #endif

  #if defined(TCS34725_COLORVIEW) || defined(LSM6DSL_ACCGYR) || defined(PROXISENSOR_VL6180X) || defined(PROXISENSOR_VL53L0X)
  Firmata.sendSysex(0x01, sizeof(list) , (byte *)&list);
  #endif
}




void setup() {
    firmata_begin(SerialFirmata,115200);
}

void loop() {
    firmata_execute();
}