#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Define Software Serial ports
#define Intergas_TXport                D6
#define Intergas_RXport                D7
//
// Define Wifi for esp8266 settings
const char*     ssid          = "XXX";
const char*     password      = "XXX";
//
// Define mqtt settings
const char*     mqtt_server   = "XXX";
const int       mqtt_port     =  1883;
const char*     Gl_Topic1     =  "InterGasGW";
//
// Init
WiFiClient espClient;
PubSubClient client(espClient);

#define Nothing                         0  // Build
#define ic2_Status                      1  // Build
#define StatusExtra                     2
#define ic2_BurnerVersion               3  // Build
#define ic2_Burner                      4  // Build
#define ic2_OperatingHours              5  // Build
#define ic2_Faults                      6  // Build
#define ic2_Parameters                  7  // Build
#define parametersExtra                 8
#define infoblok                        9
#define infoblokExtra                  10


const byte Gl_NoOfTelegrams = 11;


struct TelegramObjects {
  const char*           Item;
  const char*           Units;
};

struct TelegramObjects GL_ic2_Status[31] = {
    { "t1"                  , "(  °C )" },
    { "t2"                  , "(  °C )" },
    { "t3"                  , "(  °C )" },
    { "t4"                  , "(  °C )" },
    { "t5"                  , "(  °C )" },
    { "t6"                  , "(  °C )" },
    { "ch_pressure"         , "( Bar )" },
    { "temp_set"            , "(  °C )" },
    { "fanspeed_set"        , "( RPM )" },
    { "fan_speed"           , "( RPM )" },
    { "fan_pwm"             , "( --- )" },
    { "io_curr"             , "( --- )" },
    { "display_code"        , "( --- )" },
    { "gp_switch"           , "( --- )" },
    { "tap_switch"          , "( --- )" },
    { "roomtherm"           , "( --- )" },
    { "pump"                , "( --- )" },
    { "dwk"                 , "( --- )" },
    { "alarm_status"        , "( --- )" },
    { "ch_cascade_relay"    , "( --- )" },
    { "opentherm"           , "( --- )" },
    { "gasvalve"            , "( --- )" },
    { "spark"               , "( --- )" },
    { "io_signal"           , "( --- )" },
    { "ch_ot_disabled"      , "( --- )" },
    { "low_water_pressure"  , "( --- )" },
    { "pressure_sensor"     , "( --- )" },
    { "burner_block"        , "( --- )" },
    { "grad_flag"           , "( --- )" }
};

struct TelegramObjects GL_ic2_Code[15] = {
    { "opentherm"           , "( --- )" },
    { "boiler_ext"          , "( --- )" },
    { "frost"               , "( --- )" },
    { "central_heating_rf"  , "( --- )" },
    { "tapwater_int"        , "( --- )" },
    { "sensortest"          , "( --- )" },
    { "zone_heating"        , "( --- )" },
    { "standby"             , "( --- )" },
    { "postrun_boiler"      , "( --- )" },
    { "service"             , "( --- )" },
    { "tapwater"            , "( --- )" },
    { "postrun_ch"          , "( --- )" },
    { "boiler_int"          , "( --- )" },
    { "buffer"              , "( --- )" },
};

struct TelegramObjects Gl_ic2_BurnerVersion[4] = {
    { "h_version"           , "( --- )" },
    { "s_version"           , "( --- )" },
    { "checksum1"           , "( --- )" },
    { "checksum2"           , "( --- )" },
};

struct TelegramObjects Gl_ic2_Burner[7] = {
    { "interrupt_time"      , "( --- )" },
    { "interrupt_load"      , "(   % )" },
    { "main_load"           , "(   % )" },
    { "net_frequency"       , "(  Hz )" },
    { "voltage_reference"   , "(   V )" },
    { "checksum1"           , "( --- )" },
    { "checksum2"           , "( --- )" },
};

struct TelegramObjects Gl_ic2_Faults[32] = {
    { "Fault00"             , "( --- )" },
    { "Fault01"             , "( --- )" },
    { "Fault02"             , "( --- )" },
    { "Fault03"             , "( --- )" },
    { "Fault04"             , "( --- )" },
    { "Fault05"             , "( --- )" },
    { "Fault06"             , "( --- )" },
    { "Fault07"             , "( --- )" },
    { "Fault08"             , "( --- )" },
    { "Fault09"             , "( --- )" },
    { "Fault10"             , "( --- )" },
    { "Fault11"             , "( --- )" },
    { "Fault12"             , "( --- )" },
    { "Fault13"             , "( --- )" },
    { "Fault14"             , "( --- )" },
    { "Fault15"             , "( --- )" },
    { "Fault16"             , "( --- )" },
    { "Fault17"             , "( --- )" },
    { "Fault18"             , "( --- )" },
    { "Fault19"             , "( --- )" },
    { "Fault20"             , "( --- )" },
    { "Fault21"             , "( --- )" },
    { "Fault22"             , "( --- )" },
    { "Fault23"             , "( --- )" },
    { "Fault24"             , "( --- )" },
    { "Fault25"             , "( --- )" },
    { "Fault26"             , "( --- )" },
    { "Fault27"             , "( --- )" },
    { "Fault28"             , "( --- )" },
    { "Fault29"             , "( --- )" },
    { "Fault30"             , "( --- )" },
    { "Fault31"             , "( --- )" },
};

struct TelegramObjects Gl_ic2_Parameters[30] = {
    { "heater_on"           , "( --- )" },
    { "comfort_mode"        , "( --- )" },
    { "ch_set_max"          , "( --- )" },
    { "dhw_set"             , "( --- )" },
    { "eco_days"            , "( --- )" },
    { "comfort_set"         , "( --- )" },
    { "dhw_at_night"        , "( --- )" },
    { "ch_at_night"         , "( --- )" },
    { "param_1"             , "( --- )" },
    { "param_2"             , "( --- )" },
    { "param_3"             , "( --- )" },
    { "param_4"             , "( --- )" },
    { "param_5"             , "( --- )" },
    { "param_6"             , "( --- )" },
    { "param_7"             , "( --- )" },
    { "param_8"             , "( --- )" },
    { "param_9"             , "( --- )" },
    { "param_A"             , "( --- )" },
    { "param_b"             , "( --- )" },
    { "param_C"             , "( --- )" },
    { "param_d"             , "( --- )" },
    { "param_E"             , "( --- )" },
    { "param_Edot"          , "( --- )" },
    { "param_F"             , "( --- )" },
    { "param_H"             , "( --- )" },
    { "param_n"             , "( --- )" },
    { "param_o"             , "( --- )" },
    { "param_P"             , "( --- )" },
    { "param_r"             , "( --- )" },
    { "param_Fdot"          , "( --- )" },
};

struct TelegramObjects Gl_ic2_OperatingHours[8] = {
    { "line_power_connected"    , "(   h )" },
    { "line_power_disconnect"   , "(   h )" },
    { "ch_function"             , "( --- )" },
    { "dhw_function"            , "( --- )" },
    { "burnerstarts"            , "( --- )" },
    { "ignition_failed"         , "( --- )" },
    { "flame_lost"              , "( --- )" },
    { "reset"                   , "( --- )" },
};


struct ParameterRecord {
  const byte            Telegram;
  const char*           SerialCommand;
  const bool            ic2;
  const bool            ic3;
  const byte            TelegramLength;
  const unsigned long   TelegramReadSchedule;
  const byte            TelegramMaxDuration;
  const char*           TelegramTypeName;
  unsigned long         TelegramLastTimeRun;
  bool                  TelegramRunDuringBoot;
  long                  TelegramErrors;
  int                   TelegramMaxRetrieveTime;
  }; 


struct  ParameterRecord Gl_ParameterList[Gl_NoOfTelegrams] =
   {
    
    {  0, ""       ,  true,  true,  0,        0,     0, "Nothing"            , 1, 0, 0}, // Not used
    {  1, "S?\r"   ,  true, false, 32,     1000,   500, "ic2_Status"         , 1, 0, 0}, // Max 150 ms, Prefered setting 160
    {  2, "S2\r"   , false,  true, 32,999999999,   500, "StatusExtra"        , 1, 0, 0}, // To be done
    {  3, "REV"    ,  true, false, 32,  3600000,   500, "ic2_BurnerVersion"  , 1, 0, 0}, // Max 135 ms, Prefered setting 145
    {  4, "CRC"    ,  true, false, 32,    60000,   500, "ic2_Burner"         , 1, 0, 0}, // Max 100 ms, Prefered setting 110
    {  5, "HN\r"   ,  true, false, 32,    60000,   500, "ic2_OperatingHours" , 1, 0, 0}, // Max 130 ms, Prefered setting 140
    {  6, "EN\r"   ,  true, false, 32,    10000,   500, "ic2_Faults"         , 1, 0, 0}, // Max 150 ms, Prefered setting 160
    {  7, "V?\r"   ,  true, false, 32,    60000,   500, "ic2_Parameters"     , 1, 0, 0}, // Max 135 ms, Prefered setting 145
    {  8, "LX?"    ,  true,  true, 32,999999999,   500, "parametersExtra"    , 1, 0, 0}, // To be done
    {  9, "B?\r"   ,  true,  true, 32,999999999,   500, "infoblok"           , 1, 0, 0}, // To be done
    { 10, "B2\r"   ,  true,  true, 32,999999999,   500, "infoblokExtra"      , 1, 0, 0}  // To be done
    };

const byte Gl_SerialRecieveTimeoutExtraDubugTime          = 200;

struct ic2_StatusResult{
  double t1;
  double t2;
  double t3;
  double t4;
  double t5;
  double t6;  
  double ch_pressure;
  double temp_set;
  double fanspeed_set;
  double fan_speed;
  double fan_pwm;
  double io_curr;
  double display_code;
  bool   gp_switch;
  bool   tap_switch;
  bool   roomtherm;
  bool   pump;
  bool   dwk;
  bool   alarm_status;
  bool   ch_cascade_relay;
  bool   opentherm;
  bool   gasvalve;
  bool   spark;
  bool   io_signal;
  bool   ch_ot_disabled;
  bool   low_water_pressure;
  bool   pressure_sensor;
  bool   burner_block;
  bool   grad_flag;
};

struct ic2_CodeResult{
  bool opentherm;
  bool boiler_ext;
  bool frost;
  bool central_heating_rf;
  bool tapwater_int;
  bool sensortest;
  bool zone_heating;
  bool standby;
  bool postrun_boiler;
  bool service;
  bool tapwater;
  bool postrun_ch;
  bool boiler_int;
  bool buffer;
};

struct ic2_BurnerVersionResult{
  char h_version[18];
  char s_version[6];
  int checksum1; 
  int checksum2;
};

struct ic2_BurnerResult{
  float interrupt_time;
  float interrupt_load;
  float main_load;
  float net_frequency;
  float voltage_reference;
  byte  checksum1;
  byte  checksum2;
};


struct ic2_OperatingHoursResult{
  int line_power_connected;
  int line_power_disconnect;
  int ch_function;
  int dhw_function;
  int burnerstarts;
  int ignition_failed;
  int flame_lost;
  int reset;
};

struct ic2_FaultsResult{
  byte  Fault00;
  byte  Fault01;
  byte  Fault02;
  byte  Fault03;
  byte  Fault04;
  byte  Fault05;
  byte  Fault06;
  byte  Fault07;
  byte  Fault08;
  byte  Fault09;
  byte  Fault10;
  byte  Fault11;
  byte  Fault12;
  byte  Fault13;
  byte  Fault14;
  byte  Fault15;
  byte  Fault16;
  byte  Fault17;
  byte  Fault18;
  byte  Fault19;
  byte  Fault20;
  byte  Fault21;
  byte  Fault22;
  byte  Fault23;
  byte  Fault24;
  byte  Fault25;
  byte  Fault26;
  byte  Fault27;
  byte  Fault28;
  byte  Fault29;
  byte  Fault30;
  byte  Fault31;
};

struct ic2_ParametersResult{
  byte heater_on;
  byte comfort_mode;
  byte ch_set_max;
  byte dhw_set;
  byte eco_days;
  byte comfort_set;
  byte dhw_at_night;
  byte ch_at_night;
  byte param_1;
  byte param_2;
  byte param_3;
  byte param_4;
  byte param_5;
  byte param_6;
  byte param_7;
  byte param_8;
  byte param_9;
  byte param_A;
  byte param_b;
  byte param_C;
  byte param_c;
  byte param_d;
  byte param_E;
  byte param_Edot;
  byte param_F;
  byte param_H;
  byte param_n;
  byte param_o;
  byte param_P;
  byte param_r;
  byte param_Fdot;
};

struct ic2_StatusResult             Gl_ic2_StatusResultNew;
struct ic2_StatusResult             Gl_ic2_StatusResultOld;
//
struct ic2_CodeResult               Gl_ic2_CodeResultNew;
struct ic2_CodeResult               Gl_ic2_CodeResultOld;
//
struct ic2_BurnerVersionResult      Gl_ic2_BurnerVersionResultNew;
struct ic2_BurnerVersionResult      Gl_ic2_BurnerVersionResultOld;
//
struct ic2_BurnerResult             Gl_ic2_BurnerResultNew;
struct ic2_BurnerResult             Gl_ic2_BurnerResultOld;
//
struct ic2_OperatingHoursResult     Gl_ic2_OperatingHoursResultNew;
struct ic2_OperatingHoursResult     Gl_ic2_OperatingHoursResultOld;
//
struct ic2_FaultsResult             Gl_ic2_FaultsResultNew;
struct ic2_FaultsResult             Gl_ic2_FaultsResultOld;
//
struct ic2_ParametersResult         Gl_ic2_ParametersResultNew;
struct ic2_ParametersResult         Gl_ic2_ParametersResultOld;



//Debug Parameters
bool          Gl_DebugSerialRxTx                      = 0;
bool          Gl_DebugSerialStatus                    = 1;

bool          Gl_DebugWriteTelegramResult             = 1;

bool          Gl_ValidTelegram                        = 0;

byte          Gl_Request                              = 0;

unsigned char Gl_RecievedArray[64];    

bool          Gl_ic2                                  = 0;
bool          Gl_ic3                                  = 0;

//Define Serial port
SoftwareSerial InterGasSerial(Intergas_RXport, Intergas_TXport); 

void setup() {
  pinMode(Intergas_RXport, INPUT);
  pinMode(Intergas_TXport, OUTPUT);
  //
  // Open serial port to Intergas boiler
  InterGasSerial.begin(9600);
  delay(100);
  //
  // Open serial port for debug output
  Serial        .begin(115200);
  delay(100);
  //
  //Wait until serial port is ready
  while (!Serial){
    //Do nothing
    }
  //
  //Setup Wifi
  setup_wifi();
  delay(100);
  //
  //Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  reconnect();
  delay(1000);
  //
  //Get version of the burner (ic2 or ic3) because some telegrams are not available if you have a ic2 burner)
  SendRequestToSerialPort     (ic2_BurnerVersion);
  GetDataFromSerialPort       (ic2_BurnerVersion);
  ParseDataFromSerialPort     (ic2_BurnerVersion);
  DisplayDataFromSerialPort   (ic2_BurnerVersion);
  PublishDataFromSerialPort   (ic2_BurnerVersion);
  PublishSerialStats          (ic2_BurnerVersion);

    
  switch (Gl_ic2_BurnerVersionResultNew.h_version[2]){
    case '2':
      Gl_ic2 = true;
      Gl_ic3 = false;
    break;
    case '3':
      Gl_ic2 = false;
      Gl_ic3 = true;
    break;
  }
}


void loop() {
                // Empty the serial buffer if filled
                EmptySerialBuffer();
                // Determine which telegram needs to be fetched based on time
  Gl_Request =  DetermineRequest();
                // Send data to the serial port
                SendRequestToSerialPort     (Gl_Request);
                GetDataFromSerialPort       (Gl_Request);
                ParseDataFromSerialPort     (Gl_Request);
                DisplayDataFromSerialPort   (Gl_Request);
                PublishDataFromSerialPort   (Gl_Request);
                PublishSerialStats          (Gl_Request);
}

byte DetermineRequest (){
  unsigned long loc_Time     = millis() ; // seconds to milli seconds
  byte          loc_Request  = 0;

  for (byte counter = 1; ((counter < Gl_NoOfTelegrams) and (loc_Request == 0)); counter++) {
    if ( (loc_Time > (Gl_ParameterList[counter].TelegramReadSchedule + Gl_ParameterList[counter].TelegramLastTimeRun)) and (Gl_ParameterList[counter].ic2 == Gl_ic2) and (Gl_ParameterList[counter].ic3 == Gl_ic3) ){
      loc_Request = counter;
    }
  }
  return loc_Request;
}


void SendRequestToSerialPort (byte loc_Status){
  unsigned long loc_Time     = millis() ; // seconds to milli seconds
  if (loc_Status != Nothing){
    //
    Gl_ParameterList[loc_Status].TelegramLastTimeRun = loc_Time;
    //
    InterGasSerial.write(Gl_ParameterList[loc_Status].SerialCommand);
    //
    if (Gl_DebugSerialRxTx == 1){
      Serial.print   ("--  Tx  --");
      Serial.print   ("  Value : ");
      Serial.println (Gl_ParameterList[loc_Status].SerialCommand);
    }
  }
}

void GetDataFromSerialPort (byte loc_Status){
  unsigned char loc_ByteRecieved;
  byte          loc_ArrayIndex;

  unsigned long loc_TimeStartRecievingTelegram;
  unsigned long DurationRecievingTelegram;

  int           loc_TelegramMaxDuration;
  byte          loc_TelegramLength;

  byte          loc_BytesInSerialBuffer;
  byte          loc_DebugArray[32][3];

  if (loc_Status != Nothing){
    //
    loc_TelegramMaxDuration   = Gl_ParameterList[loc_Status].TelegramMaxDuration;
    loc_TelegramLength        = Gl_ParameterList[loc_Status].TelegramLength;
    //    
    if (Gl_DebugSerialRxTx == 1) {
      loc_TelegramMaxDuration = loc_TelegramMaxDuration + Gl_SerialRecieveTimeoutExtraDubugTime;
    }
    //
    loc_TimeStartRecievingTelegram = millis();
    //  
    do {
      loc_BytesInSerialBuffer = InterGasSerial.available();
      if (loc_BytesInSerialBuffer > 0) {
        loc_ByteRecieved = InterGasSerial.read();
        if (Gl_DebugSerialRxTx == 1){
          loc_DebugArray[loc_ArrayIndex][0] = loc_BytesInSerialBuffer;
          loc_DebugArray[loc_ArrayIndex][1] = loc_ArrayIndex;
          loc_DebugArray[loc_ArrayIndex][2] = loc_ByteRecieved;
        }
      Gl_RecievedArray[loc_ArrayIndex] = loc_ByteRecieved;
      loc_ArrayIndex++;
      }
    } while ((millis() <= (loc_TimeStartRecievingTelegram + loc_TelegramMaxDuration)) and (loc_ArrayIndex <= (loc_TelegramLength-1)));
    //
    DurationRecievingTelegram = (millis()-loc_TimeStartRecievingTelegram);
    
    if (Gl_ParameterList[loc_Status].TelegramMaxRetrieveTime < DurationRecievingTelegram) {
      Gl_ParameterList[loc_Status].TelegramMaxRetrieveTime = DurationRecievingTelegram;  
    }
    
    //
    
    if ((loc_ArrayIndex) == loc_TelegramLength){
      Gl_ValidTelegram = 1;
      }
    else {
      Gl_ValidTelegram = 0;
      Gl_ParameterList[loc_Status].TelegramErrors = Gl_ParameterList[loc_Status].TelegramErrors + 1;
      }

    //
    //
    if (Gl_DebugSerialRxTx == 1){
      for (byte counter = 0; counter < loc_ArrayIndex; counter++ ){
          Serial.print  ("- Rx -");
          Serial.print  ("  Buf : ");
          Serial.print  (loc_DebugArray[counter][0]);
          Serial.print  ("  ArrIx : ");
          Serial.print  (loc_DebugArray[counter][1]);
          Serial.print  ("  Val : ");
          Serial.println(loc_DebugArray[counter][2]);
      }
    }
    //
    
    if (Gl_DebugSerialStatus == 1){
      if ((loc_ArrayIndex) == loc_TelegramLength){
          Serial.printf ("Type: %-20s Telegram: %4s   RecievingTime: %4s ms\n", String(Gl_ParameterList[loc_Status].TelegramTypeName).c_str() , "Good", String(DurationRecievingTelegram).c_str() );
        }
      else {
          Serial.printf ("Type: %-20s Telegram: %4s   RecievingTime: %4s ms\n", String(Gl_ParameterList[loc_Status].TelegramTypeName).c_str() , "Bad" , String(DurationRecievingTelegram).c_str() );
        }
    }
 }
}

void ParseDataFromSerialPort (byte loc_Status){
  byte loc_VPosition     = 0;
  char loc_h_version[18] = "";
  char loc_s_version[6]  = "";
      
  
  if (Gl_ValidTelegram == true){
    switch(loc_Status){
      case Nothing:
      break;

      case ic2_Status:
        Gl_ic2_StatusResultNew.t1                               =   getDouble(Gl_RecievedArray[1] , Gl_RecievedArray[0]);
        Gl_ic2_StatusResultNew.t2                               =   getDouble(Gl_RecievedArray[3] , Gl_RecievedArray[2]);
        Gl_ic2_StatusResultNew.t3                               =   getDouble(Gl_RecievedArray[5] , Gl_RecievedArray[4]);
        Gl_ic2_StatusResultNew.t4                               =   getDouble(Gl_RecievedArray[7] , Gl_RecievedArray[6]);
        Gl_ic2_StatusResultNew.t5                               =   getDouble(Gl_RecievedArray[9] , Gl_RecievedArray[8]);
        Gl_ic2_StatusResultNew.t6                               =   getDouble(Gl_RecievedArray[11], Gl_RecievedArray[10]);
        Gl_ic2_StatusResultNew.ch_pressure                      =   getDouble(Gl_RecievedArray[13], Gl_RecievedArray[12]);
        Gl_ic2_StatusResultNew.temp_set                         =   getDouble(Gl_RecievedArray[15], Gl_RecievedArray[14]);
        Gl_ic2_StatusResultNew.fanspeed_set                     =   getDouble(Gl_RecievedArray[17], Gl_RecievedArray[16]) * 100;
        Gl_ic2_StatusResultNew.fan_speed                        =   getDouble(Gl_RecievedArray[19], Gl_RecievedArray[18]) * 100;
        Gl_ic2_StatusResultNew.fan_pwm                          =   getDouble(Gl_RecievedArray[21], Gl_RecievedArray[20]) * 10;
        Gl_ic2_StatusResultNew.io_curr                          =   getDouble(Gl_RecievedArray[21], Gl_RecievedArray[22]);
        Gl_ic2_StatusResultNew.display_code                     =   Gl_RecievedArray[24];
        Gl_ic2_StatusResultNew.gp_switch                        =   (Gl_RecievedArray[26] & 0x01) == 0x01 ? true : false;
        Gl_ic2_StatusResultNew.tap_switch                       =   (Gl_RecievedArray[26] & 0x02) == 0x02 ? true : false;
        Gl_ic2_StatusResultNew.roomtherm                        =   (Gl_RecievedArray[26] & 0x04) == 0x04 ? true : false;
        Gl_ic2_StatusResultNew.pump                             =   (Gl_RecievedArray[26] & 0x08) == 0x08 ? true : false;
        Gl_ic2_StatusResultNew.dwk                              =   (Gl_RecievedArray[26] & 0x10) == 0x10 ? true : false;
        Gl_ic2_StatusResultNew.alarm_status                     =   (Gl_RecievedArray[26] & 0x20) == 0x20 ? true : false;
        Gl_ic2_StatusResultNew.ch_cascade_relay                 =   (Gl_RecievedArray[26] & 0x40) == 0x40 ? true : false;
        Gl_ic2_StatusResultNew.opentherm                        =   (Gl_RecievedArray[26] & 0x80) == 0x80 ? true : false;
        Gl_ic2_StatusResultNew.gasvalve                         =   (Gl_RecievedArray[28] & 0x01) == 0x01 ? true : false;
        Gl_ic2_StatusResultNew.spark                            =   (Gl_RecievedArray[28] & 0x02) == 0x02 ? true : false;
        Gl_ic2_StatusResultNew.io_signal                        =   (Gl_RecievedArray[28] & 0x04) == 0x04 ? true : false;
        Gl_ic2_StatusResultNew.ch_ot_disabled                   =   (Gl_RecievedArray[28] & 0x08) == 0x08 ? true : false;
        Gl_ic2_StatusResultNew.low_water_pressure               =   (Gl_RecievedArray[28] & 0x10) == 0x10 ? true : false;
        Gl_ic2_StatusResultNew.pressure_sensor                  =   (Gl_RecievedArray[28] & 0x20) == 0x20 ? true : false;
        Gl_ic2_StatusResultNew.burner_block                     =   (Gl_RecievedArray[28] & 0x40) == 0x40 ? true : false;
        Gl_ic2_StatusResultNew.grad_flag                        =   (Gl_RecievedArray[28] & 0x80) == 0x80 ? true : false;

        Gl_ic2_CodeResultNew.opentherm                          =   (Gl_RecievedArray[24])        ==   00 ? true : false;
        Gl_ic2_CodeResultNew.boiler_ext                         =   (Gl_RecievedArray[24])        ==   15 ? true : false;
        Gl_ic2_CodeResultNew.frost                              =   (Gl_RecievedArray[24])        ==   24 ? true : false;
        Gl_ic2_CodeResultNew.central_heating_rf                 =   (Gl_RecievedArray[24])        ==   37 ? true : false;
        Gl_ic2_CodeResultNew.tapwater_int                       =   (Gl_RecievedArray[24])        ==   51 ? true : false;
        Gl_ic2_CodeResultNew.sensortest                         =   (Gl_RecievedArray[24])        ==   58 ? true : false;
        Gl_ic2_CodeResultNew.zone_heating                       =   (Gl_RecievedArray[24])        ==  102 ? true : false;
        Gl_ic2_CodeResultNew.standby                            =   (Gl_RecievedArray[24])        ==  126 ? true : false;
        Gl_ic2_CodeResultNew.postrun_boiler                     =   (Gl_RecievedArray[24])        ==  153 ? true : false;
        Gl_ic2_CodeResultNew.service                            =   (Gl_RecievedArray[24])        ==  170 ? true : false;
        Gl_ic2_CodeResultNew.tapwater                           =   (Gl_RecievedArray[24])        ==  204 ? true : false;
        Gl_ic2_CodeResultNew.postrun_ch                         =   (Gl_RecievedArray[24])        ==  231 ? true : false;
        Gl_ic2_CodeResultNew.boiler_int                         =   (Gl_RecievedArray[24])        ==  240 ? true : false;
        Gl_ic2_CodeResultNew.buffer                             =   (Gl_RecievedArray[24])        ==  255 ? true : false;
      break;

      case ic2_Parameters:
        Gl_ic2_ParametersResultNew.heater_on                    =   Getsigned(Gl_RecievedArray[0]);
        Gl_ic2_ParametersResultNew.comfort_mode                 =   Getsigned(Gl_RecievedArray[1]);
        Gl_ic2_ParametersResultNew.ch_set_max                   =   Getsigned(Gl_RecievedArray[2]);
        Gl_ic2_ParametersResultNew.dhw_set                      =   Getsigned(Gl_RecievedArray[3]);
        Gl_ic2_ParametersResultNew.eco_days                     =   Getsigned(Gl_RecievedArray[4]);
        Gl_ic2_ParametersResultNew.comfort_set                  =   Getsigned(Gl_RecievedArray[5]);
        Gl_ic2_ParametersResultNew.dhw_at_night                 =   Getsigned(Gl_RecievedArray[6]);
        Gl_ic2_ParametersResultNew.ch_at_night                  =   Getsigned(Gl_RecievedArray[7]);
        Gl_ic2_ParametersResultNew.param_1                      =   Getsigned(Gl_RecievedArray[8]);
        Gl_ic2_ParametersResultNew.param_2                      =   Getsigned(Gl_RecievedArray[9]);
        Gl_ic2_ParametersResultNew.param_3                      =   Getsigned(Gl_RecievedArray[10]);
        Gl_ic2_ParametersResultNew.param_4                      =   Getsigned(Gl_RecievedArray[11]);
        Gl_ic2_ParametersResultNew.param_5                      =   Getsigned(Gl_RecievedArray[12]);
        Gl_ic2_ParametersResultNew.param_6                      =   Getsigned(Gl_RecievedArray[13]);
        Gl_ic2_ParametersResultNew.param_7                      =   Getsigned(Gl_RecievedArray[14]);
        Gl_ic2_ParametersResultNew.param_8                      =   Getsigned(Gl_RecievedArray[15]);
        Gl_ic2_ParametersResultNew.param_9                      =   Getsigned(Gl_RecievedArray[16]);
        Gl_ic2_ParametersResultNew.param_A                      =   Getsigned(Gl_RecievedArray[17]);
        Gl_ic2_ParametersResultNew.param_b                      =   Getsigned(Gl_RecievedArray[18]);
        Gl_ic2_ParametersResultNew.param_C                      =   Getsigned(Gl_RecievedArray[19]);
        Gl_ic2_ParametersResultNew.param_c                      =   Getsigned(Gl_RecievedArray[20]);
        Gl_ic2_ParametersResultNew.param_d                      =   Getsigned(Gl_RecievedArray[21]);
        Gl_ic2_ParametersResultNew.param_E                      =   Getsigned(Gl_RecievedArray[22]);
        Gl_ic2_ParametersResultNew.param_Edot                   =   Getsigned(Gl_RecievedArray[23]);
        Gl_ic2_ParametersResultNew.param_F                      =   Getsigned(Gl_RecievedArray[24]);
        Gl_ic2_ParametersResultNew.param_H                      =   Getsigned(Gl_RecievedArray[25]);
        Gl_ic2_ParametersResultNew.param_n                      =   Getsigned(Gl_RecievedArray[26]);
        Gl_ic2_ParametersResultNew.param_o                      =   Getsigned(Gl_RecievedArray[27]);
        Gl_ic2_ParametersResultNew.param_P                      =   Getsigned(Gl_RecievedArray[28]);
        Gl_ic2_ParametersResultNew.param_r                      =   Getsigned(Gl_RecievedArray[29]);
        Gl_ic2_ParametersResultNew.param_Fdot                   =   Getsigned(Gl_RecievedArray[30]);
        break;    

      case ic2_OperatingHours:
        Gl_ic2_OperatingHoursResultNew.line_power_connected     =   getInt(Gl_RecievedArray[1] , Gl_RecievedArray[0] + Gl_RecievedArray[30] * 65536);
        Gl_ic2_OperatingHoursResultNew.line_power_disconnect    =   getInt(Gl_RecievedArray[3] , Gl_RecievedArray[2]);
        Gl_ic2_OperatingHoursResultNew.ch_function              =   getInt(Gl_RecievedArray[5] , Gl_RecievedArray[4]);
        Gl_ic2_OperatingHoursResultNew.dhw_function             =   getInt(Gl_RecievedArray[7] , Gl_RecievedArray[6]);
        Gl_ic2_OperatingHoursResultNew.burnerstarts             =   getInt(Gl_RecievedArray[9] , Gl_RecievedArray[8] + Gl_RecievedArray[31] * 65536);
        Gl_ic2_OperatingHoursResultNew.ignition_failed          =   getInt(Gl_RecievedArray[11], Gl_RecievedArray[10]);
        Gl_ic2_OperatingHoursResultNew.flame_lost               =   getInt(Gl_RecievedArray[13], Gl_RecievedArray[12]);
        Gl_ic2_OperatingHoursResultNew.reset                    =   getInt(Gl_RecievedArray[15], Gl_RecievedArray[14]);
      break;

      case ic2_BurnerVersion:
        for (byte Position = 0; Position < 32; Position++) {
          if ((char(Gl_RecievedArray[Position])) == 'V') {
            loc_VPosition = Position;
          }
        }    

        // Retrieve Hardware version
        for (byte Position = 0; Position < loc_VPosition; Position++) {
          Gl_ic2_BurnerVersionResultNew.h_version[Position]     = char(Gl_RecievedArray[Position]);
        }

        // Retrieve Software version
        if ( (char(Gl_RecievedArray[loc_VPosition + 1]) > 3) or ( char(Gl_RecievedArray[2]) == 3 )){
          Gl_ic2_BurnerVersionResultNew.s_version[0]            =   char(Gl_RecievedArray[loc_VPosition    ]);
          Gl_ic2_BurnerVersionResultNew.s_version[1]            =   char(Gl_RecievedArray[loc_VPosition + 1]);
          Gl_ic2_BurnerVersionResultNew.s_version[2]            =   char(Gl_RecievedArray[loc_VPosition + 2]);
          Gl_ic2_BurnerVersionResultNew.s_version[3]            =   char(Gl_RecievedArray[loc_VPosition + 3]);
          Gl_ic2_BurnerVersionResultNew.s_version[4]            =   char(Gl_RecievedArray[loc_VPosition + 4]);
        }
        else {
          Gl_ic2_BurnerVersionResultNew.s_version[0]            =   char(Gl_RecievedArray[loc_VPosition    ]);
          Gl_ic2_BurnerVersionResultNew.s_version[1]            =   char(Gl_RecievedArray[loc_VPosition + 1]);
          Gl_ic2_BurnerVersionResultNew.s_version[2]            =   char(Gl_RecievedArray[loc_VPosition + 2]);
          Gl_ic2_BurnerVersionResultNew.s_version[3]            =   char(Gl_RecievedArray[loc_VPosition + 3]);
        }

          Gl_ic2_BurnerVersionResultNew.checksum1              =    Get_CRC(char(Gl_RecievedArray[24]) , char(Gl_RecievedArray[25]) , char(Gl_RecievedArray[26]) , char(Gl_RecievedArray[27]));
          Gl_ic2_BurnerVersionResultNew.checksum2              =    Get_CRC(char(Gl_RecievedArray[28]) , char(Gl_RecievedArray[29]) , char(Gl_RecievedArray[30]) , char(Gl_RecievedArray[31]));

      
      break;

      case ic2_Burner:
        Gl_ic2_BurnerResultNew.interrupt_time                  =    (getInt(Gl_RecievedArray[0], Gl_RecievedArray[1]) / (float)5);
        Gl_ic2_BurnerResultNew.interrupt_load                  =    (getInt(Gl_RecievedArray[2], Gl_RecievedArray[3]) / (float)6.25);
        Gl_ic2_BurnerResultNew.main_load                       =    (getInt(Gl_RecievedArray[4], Gl_RecievedArray[5]) / (float)8);

        if (getInt(Gl_RecievedArray[6],Gl_RecievedArray[7]) > 0){
          Gl_ic2_BurnerResultNew.net_frequency                 =    (((float)2000 / getInt(Gl_RecievedArray[6], Gl_RecievedArray[7])));  
        }
        else{
          Gl_ic2_BurnerResultNew.net_frequency                 =    0;
        }
        Gl_ic2_BurnerResultNew.voltage_reference               =    (getInt(Gl_RecievedArray[8],Gl_RecievedArray[9]) * 5 /(float)1024);

        Gl_ic2_BurnerResultNew.checksum1                       =    Get_CRC(char(Gl_RecievedArray[24]) , char(Gl_RecievedArray[25]) , char(Gl_RecievedArray[26]) , char(Gl_RecievedArray[27]));
        Gl_ic2_BurnerResultNew.checksum2                       =    Get_CRC(char(Gl_RecievedArray[28]) , char(Gl_RecievedArray[29]) , char(Gl_RecievedArray[30]) , char(Gl_RecievedArray[31]));
        
      break;

      case ic2_Faults:
        Gl_ic2_FaultsResultNew.Fault00                        =     CheckFault(Gl_RecievedArray[0]);
        Gl_ic2_FaultsResultNew.Fault01                        =     CheckFault(Gl_RecievedArray[1]);
        Gl_ic2_FaultsResultNew.Fault02                        =     CheckFault(Gl_RecievedArray[2]);
        Gl_ic2_FaultsResultNew.Fault03                        =     CheckFault(Gl_RecievedArray[3]);
        Gl_ic2_FaultsResultNew.Fault04                        =     CheckFault(Gl_RecievedArray[4]);
        Gl_ic2_FaultsResultNew.Fault05                        =     CheckFault(Gl_RecievedArray[5]);
        Gl_ic2_FaultsResultNew.Fault06                        =     CheckFault(Gl_RecievedArray[6]);
        Gl_ic2_FaultsResultNew.Fault07                        =     CheckFault(Gl_RecievedArray[7]);
        Gl_ic2_FaultsResultNew.Fault08                        =     CheckFault(Gl_RecievedArray[8]);
        Gl_ic2_FaultsResultNew.Fault09                        =     CheckFault(Gl_RecievedArray[9]);
        Gl_ic2_FaultsResultNew.Fault10                        =     CheckFault(Gl_RecievedArray[10]);
        Gl_ic2_FaultsResultNew.Fault11                        =     CheckFault(Gl_RecievedArray[11]);
        Gl_ic2_FaultsResultNew.Fault12                        =     CheckFault(Gl_RecievedArray[12]);
        Gl_ic2_FaultsResultNew.Fault13                        =     CheckFault(Gl_RecievedArray[13]);
        Gl_ic2_FaultsResultNew.Fault14                        =     CheckFault(Gl_RecievedArray[14]);
        Gl_ic2_FaultsResultNew.Fault15                        =     CheckFault(Gl_RecievedArray[15]);
        Gl_ic2_FaultsResultNew.Fault16                        =     CheckFault(Gl_RecievedArray[16]);
        Gl_ic2_FaultsResultNew.Fault17                        =     CheckFault(Gl_RecievedArray[17]);
        Gl_ic2_FaultsResultNew.Fault18                        =     CheckFault(Gl_RecievedArray[18]);
        Gl_ic2_FaultsResultNew.Fault19                        =     CheckFault(Gl_RecievedArray[19]);
        Gl_ic2_FaultsResultNew.Fault20                        =     CheckFault(Gl_RecievedArray[20]);
        Gl_ic2_FaultsResultNew.Fault21                        =     CheckFault(Gl_RecievedArray[21]);
        Gl_ic2_FaultsResultNew.Fault22                        =     CheckFault(Gl_RecievedArray[22]);
        Gl_ic2_FaultsResultNew.Fault23                        =     CheckFault(Gl_RecievedArray[23]);
        Gl_ic2_FaultsResultNew.Fault24                        =     CheckFault(Gl_RecievedArray[24]);
        Gl_ic2_FaultsResultNew.Fault25                        =     CheckFault(Gl_RecievedArray[25]);
        Gl_ic2_FaultsResultNew.Fault26                        =     CheckFault(Gl_RecievedArray[26]);
        Gl_ic2_FaultsResultNew.Fault27                        =     CheckFault(Gl_RecievedArray[27]);
        Gl_ic2_FaultsResultNew.Fault28                        =     CheckFault(Gl_RecievedArray[28]);
        Gl_ic2_FaultsResultNew.Fault29                        =     CheckFault(Gl_RecievedArray[29]);
        Gl_ic2_FaultsResultNew.Fault30                        =     CheckFault(Gl_RecievedArray[30]);
        Gl_ic2_FaultsResultNew.Fault31                        =     CheckFault(Gl_RecievedArray[31]);
      break;    
    }
  }
 }

void DisplayDataFromSerialPort (byte loc_Status){
   String loc_Topic2;

  if (Gl_ValidTelegram == true and Gl_DebugWriteTelegramResult == true ){
    switch(loc_Status){
      case Nothing:
        //Nothing to do
      break;
      case ic2_Status:
        loc_Topic2 = "ic2_Status"; // 10 characters
        
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 0].Item), String(Gl_ic2_StatusResultNew.t1),                 String(GL_ic2_Status[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 1].Item), String(Gl_ic2_StatusResultNew.t2),                 String(GL_ic2_Status[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 2].Item), String(Gl_ic2_StatusResultNew.t3),                 String(GL_ic2_Status[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 3].Item), String(Gl_ic2_StatusResultNew.t4),                 String(GL_ic2_Status[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 4].Item), String(Gl_ic2_StatusResultNew.t5),                 String(GL_ic2_Status[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 5].Item), String(Gl_ic2_StatusResultNew.t6),                 String(GL_ic2_Status[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 6].Item), String(Gl_ic2_StatusResultNew.ch_pressure),        String(GL_ic2_Status[ 6].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 7].Item), String(Gl_ic2_StatusResultNew.temp_set),           String(GL_ic2_Status[ 7].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 8].Item), String(Gl_ic2_StatusResultNew.fanspeed_set),       String(GL_ic2_Status[ 8].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 9].Item), String(Gl_ic2_StatusResultNew.fan_speed),          String(GL_ic2_Status[ 9].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[10].Item), String(Gl_ic2_StatusResultNew.fan_pwm),            String(GL_ic2_Status[10].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[11].Item), String(Gl_ic2_StatusResultNew.io_curr),            String(GL_ic2_Status[11].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[12].Item), String(Gl_ic2_StatusResultNew.display_code),       String(GL_ic2_Status[12].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[13].Item), String(Gl_ic2_StatusResultNew.gp_switch),          String(GL_ic2_Status[13].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[14].Item), String(Gl_ic2_StatusResultNew.tap_switch),         String(GL_ic2_Status[14].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[15].Item), String(Gl_ic2_StatusResultNew.roomtherm),          String(GL_ic2_Status[15].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[16].Item), String(Gl_ic2_StatusResultNew.pump),               String(GL_ic2_Status[16].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[17].Item), String(Gl_ic2_StatusResultNew.dwk),                String(GL_ic2_Status[17].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[18].Item), String(Gl_ic2_StatusResultNew.alarm_status),       String(GL_ic2_Status[18].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[19].Item), String(Gl_ic2_StatusResultNew.ch_cascade_relay),   String(GL_ic2_Status[19].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[20].Item), String(Gl_ic2_StatusResultNew.opentherm),          String(GL_ic2_Status[20].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[21].Item), String(Gl_ic2_StatusResultNew.gasvalve),           String(GL_ic2_Status[21].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[22].Item), String(Gl_ic2_StatusResultNew.spark),              String(GL_ic2_Status[22].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[23].Item), String(Gl_ic2_StatusResultNew.io_signal),          String(GL_ic2_Status[23].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[24].Item), String(Gl_ic2_StatusResultNew.ch_ot_disabled),     String(GL_ic2_Status[24].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[25].Item), String(Gl_ic2_StatusResultNew.low_water_pressure), String(GL_ic2_Status[25].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[26].Item), String(Gl_ic2_StatusResultNew.pressure_sensor),    String(GL_ic2_Status[26].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[27].Item), String(Gl_ic2_StatusResultNew.burner_block),       String(GL_ic2_Status[27].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[28].Item), String(Gl_ic2_StatusResultNew.grad_flag),          String(GL_ic2_Status[28].Units));
        
        Serial.println("Display State : ");
        loc_Topic2 = "ic2_Code"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 0].Item), String(Gl_ic2_CodeResultNew.opentherm),            String(GL_ic2_Code[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 1].Item), String(Gl_ic2_CodeResultNew.boiler_ext),           String(GL_ic2_Code[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 2].Item), String(Gl_ic2_CodeResultNew.frost),                String(GL_ic2_Code[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 3].Item), String(Gl_ic2_CodeResultNew.central_heating_rf),   String(GL_ic2_Code[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 4].Item), String(Gl_ic2_CodeResultNew.tapwater_int),         String(GL_ic2_Code[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 5].Item), String(Gl_ic2_CodeResultNew.sensortest),           String(GL_ic2_Code[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 6].Item), String(Gl_ic2_CodeResultNew.zone_heating),         String(GL_ic2_Code[ 6].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 7].Item), String(Gl_ic2_CodeResultNew.standby),              String(GL_ic2_Code[ 7].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 8].Item), String(Gl_ic2_CodeResultNew.postrun_boiler),       String(GL_ic2_Code[ 8].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 9].Item), String(Gl_ic2_CodeResultNew.service),              String(GL_ic2_Code[ 9].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[10].Item), String(Gl_ic2_CodeResultNew.tapwater),             String(GL_ic2_Code[10].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[11].Item), String(Gl_ic2_CodeResultNew.postrun_ch),           String(GL_ic2_Code[11].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[12].Item), String(Gl_ic2_CodeResultNew.boiler_int),           String(GL_ic2_Code[12].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[13].Item), String(Gl_ic2_CodeResultNew.buffer),               String(GL_ic2_Code[13].Units));
        break;
        
    case ic2_Parameters:
        loc_Topic2 = "ic2_Parameters"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 0].Item), String(Gl_ic2_ParametersResultNew.heater_on),          String(Gl_ic2_Parameters[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 1].Item), String(Gl_ic2_ParametersResultNew.comfort_mode),       String(Gl_ic2_Parameters[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 2].Item), String(Gl_ic2_ParametersResultNew.ch_set_max),         String(Gl_ic2_Parameters[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 3].Item), String(Gl_ic2_ParametersResultNew.dhw_set),            String(Gl_ic2_Parameters[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 4].Item), String(Gl_ic2_ParametersResultNew.eco_days),           String(Gl_ic2_Parameters[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 5].Item), String(Gl_ic2_ParametersResultNew.comfort_set),        String(Gl_ic2_Parameters[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 6].Item), String(Gl_ic2_ParametersResultNew.dhw_at_night),       String(Gl_ic2_Parameters[ 6].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 7].Item), String(Gl_ic2_ParametersResultNew.ch_at_night),        String(Gl_ic2_Parameters[ 7].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 8].Item), String(Gl_ic2_ParametersResultNew.param_1),            String(Gl_ic2_Parameters[ 8].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 9].Item), String(Gl_ic2_ParametersResultNew.param_2),            String(Gl_ic2_Parameters[ 9].Units));        
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[10].Item), String(Gl_ic2_ParametersResultNew.param_3),            String(Gl_ic2_Parameters[10].Units));        
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[11].Item), String(Gl_ic2_ParametersResultNew.param_4),            String(Gl_ic2_Parameters[11].Units));        
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[12].Item), String(Gl_ic2_ParametersResultNew.param_5),            String(Gl_ic2_Parameters[12].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[13].Item), String(Gl_ic2_ParametersResultNew.param_6),            String(Gl_ic2_Parameters[13].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[14].Item), String(Gl_ic2_ParametersResultNew.param_7),            String(Gl_ic2_Parameters[14].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[15].Item), String(Gl_ic2_ParametersResultNew.param_8),            String(Gl_ic2_Parameters[15].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[16].Item), String(Gl_ic2_ParametersResultNew.param_9),            String(Gl_ic2_Parameters[16].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[17].Item), String(Gl_ic2_ParametersResultNew.param_A),            String(Gl_ic2_Parameters[17].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[18].Item), String(Gl_ic2_ParametersResultNew.param_b),            String(Gl_ic2_Parameters[18].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[19].Item), String(Gl_ic2_ParametersResultNew.param_c),            String(Gl_ic2_Parameters[19].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[20].Item), String(Gl_ic2_ParametersResultNew.param_d),            String(Gl_ic2_Parameters[20].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[21].Item), String(Gl_ic2_ParametersResultNew.param_E),            String(Gl_ic2_Parameters[21].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[22].Item), String(Gl_ic2_ParametersResultNew.param_Edot),         String(Gl_ic2_Parameters[22].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[23].Item), String(Gl_ic2_ParametersResultNew.param_F),            String(Gl_ic2_Parameters[23].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[24].Item), String(Gl_ic2_ParametersResultNew.param_H),            String(Gl_ic2_Parameters[24].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[25].Item), String(Gl_ic2_ParametersResultNew.param_n),            String(Gl_ic2_Parameters[25].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[26].Item), String(Gl_ic2_ParametersResultNew.param_o),            String(Gl_ic2_Parameters[26].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[27].Item), String(Gl_ic2_ParametersResultNew.param_P),            String(Gl_ic2_Parameters[27].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[28].Item), String(Gl_ic2_ParametersResultNew.param_r),            String(Gl_ic2_Parameters[28].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[29].Item), String(Gl_ic2_ParametersResultNew.param_Fdot),         String(Gl_ic2_Parameters[29].Units));
    break;

    case ic2_OperatingHours:
        loc_Topic2 = "ic2_OperatingHours"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 0].Item), String(Gl_ic2_OperatingHoursResultNew.line_power_connected),   String(Gl_ic2_OperatingHours[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 1].Item), String(Gl_ic2_OperatingHoursResultNew.line_power_disconnect),  String(Gl_ic2_OperatingHours[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 2].Item), String(Gl_ic2_OperatingHoursResultNew.ch_function),            String(Gl_ic2_OperatingHours[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 3].Item), String(Gl_ic2_OperatingHoursResultNew.dhw_function),           String(Gl_ic2_OperatingHours[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 4].Item), String(Gl_ic2_OperatingHoursResultNew.burnerstarts),           String(Gl_ic2_OperatingHours[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 5].Item), String(Gl_ic2_OperatingHoursResultNew.ignition_failed),        String(Gl_ic2_OperatingHours[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 6].Item), String(Gl_ic2_OperatingHoursResultNew.flame_lost),             String(Gl_ic2_OperatingHours[ 6].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 7].Item), String(Gl_ic2_OperatingHoursResultNew.reset),                  String(Gl_ic2_OperatingHours[ 7].Units));
    break;

    case ic2_BurnerVersion:
        loc_Topic2 = "ic2_BurnerVersion"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 0].Item), String(Gl_ic2_BurnerVersionResultNew.h_version),            String(Gl_ic2_BurnerVersion[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 1].Item), String(Gl_ic2_BurnerVersionResultNew.s_version),            String(Gl_ic2_BurnerVersion[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 2].Item), String(Gl_ic2_BurnerVersionResultNew.checksum1),            String(Gl_ic2_BurnerVersion[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 3].Item), String(Gl_ic2_BurnerVersionResultNew.checksum2),            String(Gl_ic2_BurnerVersion[ 3].Units));
    break;

    case ic2_Burner:
        loc_Topic2 = "ic2_Burner"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 0].Item), String(Gl_ic2_BurnerResultNew.interrupt_time),       String(Gl_ic2_Burner[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 1].Item), String(Gl_ic2_BurnerResultNew.interrupt_load),       String(Gl_ic2_Burner[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 2].Item), String(Gl_ic2_BurnerResultNew.main_load),            String(Gl_ic2_Burner[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 3].Item), String(Gl_ic2_BurnerResultNew.net_frequency),        String(Gl_ic2_Burner[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 4].Item), String(Gl_ic2_BurnerResultNew.voltage_reference),    String(Gl_ic2_Burner[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 5].Item), String(Gl_ic2_BurnerResultNew.checksum1),            String(Gl_ic2_Burner[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 6].Item), String(Gl_ic2_BurnerResultNew.checksum2),            String(Gl_ic2_Burner[ 6].Units));
    break;

    case ic2_Faults:
        loc_Topic2 = "ic2_Faults"; // 10 characters
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 0].Item), String(Gl_ic2_FaultsResultNew.Fault00),       String(Gl_ic2_Faults[ 0].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 1].Item), String(Gl_ic2_FaultsResultNew.Fault01),       String(Gl_ic2_Faults[ 1].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 2].Item), String(Gl_ic2_FaultsResultNew.Fault02),       String(Gl_ic2_Faults[ 2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 3].Item), String(Gl_ic2_FaultsResultNew.Fault03),       String(Gl_ic2_Faults[ 3].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 4].Item), String(Gl_ic2_FaultsResultNew.Fault04),       String(Gl_ic2_Faults[ 4].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 5].Item), String(Gl_ic2_FaultsResultNew.Fault05),       String(Gl_ic2_Faults[ 5].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 6].Item), String(Gl_ic2_FaultsResultNew.Fault06),       String(Gl_ic2_Faults[ 6].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 7].Item), String(Gl_ic2_FaultsResultNew.Fault07),       String(Gl_ic2_Faults[ 7].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 8].Item), String(Gl_ic2_FaultsResultNew.Fault08),       String(Gl_ic2_Faults[ 8].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 9].Item), String(Gl_ic2_FaultsResultNew.Fault09),       String(Gl_ic2_Faults[ 9].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[10].Item), String(Gl_ic2_FaultsResultNew.Fault10),       String(Gl_ic2_Faults[10].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[11].Item), String(Gl_ic2_FaultsResultNew.Fault11),       String(Gl_ic2_Faults[11].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[12].Item), String(Gl_ic2_FaultsResultNew.Fault12),       String(Gl_ic2_Faults[12].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[13].Item), String(Gl_ic2_FaultsResultNew.Fault13),       String(Gl_ic2_Faults[13].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[14].Item), String(Gl_ic2_FaultsResultNew.Fault14),       String(Gl_ic2_Faults[14].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[15].Item), String(Gl_ic2_FaultsResultNew.Fault15),       String(Gl_ic2_Faults[15].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[16].Item), String(Gl_ic2_FaultsResultNew.Fault16),       String(Gl_ic2_Faults[16].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[17].Item), String(Gl_ic2_FaultsResultNew.Fault17),       String(Gl_ic2_Faults[17].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[18].Item), String(Gl_ic2_FaultsResultNew.Fault18),       String(Gl_ic2_Faults[18].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[19].Item), String(Gl_ic2_FaultsResultNew.Fault19),       String(Gl_ic2_Faults[19].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[20].Item), String(Gl_ic2_FaultsResultNew.Fault20),       String(Gl_ic2_Faults[20].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[21].Item), String(Gl_ic2_FaultsResultNew.Fault21),       String(Gl_ic2_Faults[2].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[22].Item), String(Gl_ic2_FaultsResultNew.Fault22),       String(Gl_ic2_Faults[22].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[23].Item), String(Gl_ic2_FaultsResultNew.Fault23),       String(Gl_ic2_Faults[23].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[24].Item), String(Gl_ic2_FaultsResultNew.Fault24),       String(Gl_ic2_Faults[24].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[25].Item), String(Gl_ic2_FaultsResultNew.Fault25),       String(Gl_ic2_Faults[25].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[26].Item), String(Gl_ic2_FaultsResultNew.Fault26),       String(Gl_ic2_Faults[26].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[27].Item), String(Gl_ic2_FaultsResultNew.Fault27),       String(Gl_ic2_Faults[27].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[28].Item), String(Gl_ic2_FaultsResultNew.Fault28),       String(Gl_ic2_Faults[28].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[29].Item), String(Gl_ic2_FaultsResultNew.Fault29),       String(Gl_ic2_Faults[29].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[30].Item), String(Gl_ic2_FaultsResultNew.Fault30),       String(Gl_ic2_Faults[30].Units));
        PrintTelegram (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[31].Item), String(Gl_ic2_FaultsResultNew.Fault31),       String(Gl_ic2_Faults[31].Units));
    break;
    }     
  }

}

void PublishDataFromSerialPort (byte loc_Status){
  String loc_Topic2;
  if (Gl_ValidTelegram == true ){
    switch(loc_Status){
      case Nothing:
        //Nothing to do
      break;
      case ic2_Status:
        loc_Topic2 = "ic2_Status"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 0].Item), String(Gl_ic2_StatusResultNew.t1) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 1].Item), String(Gl_ic2_StatusResultNew.t2) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 2].Item), String(Gl_ic2_StatusResultNew.t3) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 3].Item), String(Gl_ic2_StatusResultNew.t4) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 4].Item), String(Gl_ic2_StatusResultNew.t5) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 5].Item), String(Gl_ic2_StatusResultNew.t6) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 6].Item), String(Gl_ic2_StatusResultNew.ch_pressure) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 7].Item), String(Gl_ic2_StatusResultNew.temp_set) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 8].Item), String(Gl_ic2_StatusResultNew.fanspeed_set) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[ 9].Item), String(Gl_ic2_StatusResultNew.fan_speed) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[10].Item), String(Gl_ic2_StatusResultNew.fan_pwm) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[11].Item), String(Gl_ic2_StatusResultNew.io_curr) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[12].Item), String(Gl_ic2_StatusResultNew.display_code) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[13].Item), String(Gl_ic2_StatusResultNew.gp_switch) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[14].Item), String(Gl_ic2_StatusResultNew.tap_switch) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[15].Item), String(Gl_ic2_StatusResultNew.roomtherm) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[16].Item), String(Gl_ic2_StatusResultNew.pump) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[17].Item), String(Gl_ic2_StatusResultNew.dwk) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[18].Item), String(Gl_ic2_StatusResultNew.alarm_status) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[19].Item), String(Gl_ic2_StatusResultNew.ch_cascade_relay) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[20].Item), String(Gl_ic2_StatusResultNew.opentherm) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[21].Item), String(Gl_ic2_StatusResultNew.gasvalve) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[22].Item), String(Gl_ic2_StatusResultNew.spark) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[23].Item), String(Gl_ic2_StatusResultNew.io_signal) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[24].Item), String(Gl_ic2_StatusResultNew.ch_ot_disabled) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[25].Item), String(Gl_ic2_StatusResultNew.low_water_pressure) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[26].Item), String(Gl_ic2_StatusResultNew.pressure_sensor) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[27].Item), String(Gl_ic2_StatusResultNew.burner_block) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Status[28].Item), String(Gl_ic2_StatusResultNew.grad_flag) );
        
        Serial.println("Display State : ");
        loc_Topic2 = "ic2_Code"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 0].Item), String(Gl_ic2_CodeResultNew.opentherm) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 1].Item), String(Gl_ic2_CodeResultNew.boiler_ext) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 2].Item), String(Gl_ic2_CodeResultNew.frost) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 3].Item), String(Gl_ic2_CodeResultNew.central_heating_rf) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 4].Item), String(Gl_ic2_CodeResultNew.tapwater_int) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 5].Item), String(Gl_ic2_CodeResultNew.sensortest) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 6].Item), String(Gl_ic2_CodeResultNew.zone_heating) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 7].Item), String(Gl_ic2_CodeResultNew.standby) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 8].Item), String(Gl_ic2_CodeResultNew.postrun_boiler) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[ 9].Item), String(Gl_ic2_CodeResultNew.service) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[10].Item), String(Gl_ic2_CodeResultNew.tapwater) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[11].Item), String(Gl_ic2_CodeResultNew.postrun_ch) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[12].Item), String(Gl_ic2_CodeResultNew.boiler_int) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(GL_ic2_Code[13].Item), String(Gl_ic2_CodeResultNew.buffer) );
        break;
        
    case ic2_Parameters:
        loc_Topic2 = "ic2_Parameters"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 0].Item), String(Gl_ic2_ParametersResultNew.heater_on) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 1].Item), String(Gl_ic2_ParametersResultNew.comfort_mode) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 2].Item), String(Gl_ic2_ParametersResultNew.ch_set_max) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 3].Item), String(Gl_ic2_ParametersResultNew.dhw_set) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 4].Item), String(Gl_ic2_ParametersResultNew.eco_days) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 5].Item), String(Gl_ic2_ParametersResultNew.comfort_set) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 6].Item), String(Gl_ic2_ParametersResultNew.dhw_at_night) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 7].Item), String(Gl_ic2_ParametersResultNew.ch_at_night) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 8].Item), String(Gl_ic2_ParametersResultNew.param_1) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[ 9].Item), String(Gl_ic2_ParametersResultNew.param_2) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[10].Item), String(Gl_ic2_ParametersResultNew.param_3) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[11].Item), String(Gl_ic2_ParametersResultNew.param_4) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[12].Item), String(Gl_ic2_ParametersResultNew.param_5) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[13].Item), String(Gl_ic2_ParametersResultNew.param_6) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[14].Item), String(Gl_ic2_ParametersResultNew.param_7) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[15].Item), String(Gl_ic2_ParametersResultNew.param_8) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[16].Item), String(Gl_ic2_ParametersResultNew.param_9) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[17].Item), String(Gl_ic2_ParametersResultNew.param_A) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[18].Item), String(Gl_ic2_ParametersResultNew.param_b) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[19].Item), String(Gl_ic2_ParametersResultNew.param_c) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[20].Item), String(Gl_ic2_ParametersResultNew.param_d) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[21].Item), String(Gl_ic2_ParametersResultNew.param_E) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[22].Item), String(Gl_ic2_ParametersResultNew.param_Edot) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[23].Item), String(Gl_ic2_ParametersResultNew.param_F) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[24].Item), String(Gl_ic2_ParametersResultNew.param_H) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[25].Item), String(Gl_ic2_ParametersResultNew.param_n) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[26].Item), String(Gl_ic2_ParametersResultNew.param_o) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[27].Item), String(Gl_ic2_ParametersResultNew.param_P) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[28].Item), String(Gl_ic2_ParametersResultNew.param_r) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Parameters[29].Item), String(Gl_ic2_ParametersResultNew.param_Fdot) );
    break;

    case ic2_OperatingHours:
        loc_Topic2 = "ic2_OperatingHours"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 0].Item), String(Gl_ic2_OperatingHoursResultNew.line_power_connected) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 1].Item), String(Gl_ic2_OperatingHoursResultNew.line_power_disconnect) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 2].Item), String(Gl_ic2_OperatingHoursResultNew.ch_function) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 3].Item), String(Gl_ic2_OperatingHoursResultNew.dhw_function) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 4].Item), String(Gl_ic2_OperatingHoursResultNew.burnerstarts) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 5].Item), String(Gl_ic2_OperatingHoursResultNew.ignition_failed) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 6].Item), String(Gl_ic2_OperatingHoursResultNew.flame_lost) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_OperatingHours[ 7].Item), String(Gl_ic2_OperatingHoursResultNew.reset) );
    break;

    case ic2_BurnerVersion:
        loc_Topic2 = "ic2_BurnerVersion"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 0].Item), String(Gl_ic2_BurnerVersionResultNew.h_version) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 1].Item), String(Gl_ic2_BurnerVersionResultNew.s_version) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 2].Item), String(Gl_ic2_BurnerVersionResultNew.checksum1) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_BurnerVersion[ 3].Item), String(Gl_ic2_BurnerVersionResultNew.checksum2) );
    break;

    case ic2_Burner:
        loc_Topic2 = "ic2_Burner"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 0].Item), String(Gl_ic2_BurnerResultNew.interrupt_time) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 1].Item), String(Gl_ic2_BurnerResultNew.interrupt_load) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 2].Item), String(Gl_ic2_BurnerResultNew.main_load) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 3].Item), String(Gl_ic2_BurnerResultNew.net_frequency) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 4].Item), String(Gl_ic2_BurnerResultNew.voltage_reference) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 5].Item), String(Gl_ic2_BurnerResultNew.checksum1) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Burner[ 6].Item), String(Gl_ic2_BurnerResultNew.checksum2) );
    break;

    case ic2_Faults:
        loc_Topic2 = "ic2_Faults"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 0].Item), String(Gl_ic2_FaultsResultNew.Fault00) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 1].Item), String(Gl_ic2_FaultsResultNew.Fault01) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 2].Item), String(Gl_ic2_FaultsResultNew.Fault02) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 3].Item), String(Gl_ic2_FaultsResultNew.Fault03) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 4].Item), String(Gl_ic2_FaultsResultNew.Fault04) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 5].Item), String(Gl_ic2_FaultsResultNew.Fault05) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 6].Item), String(Gl_ic2_FaultsResultNew.Fault06) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 7].Item), String(Gl_ic2_FaultsResultNew.Fault07) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 8].Item), String(Gl_ic2_FaultsResultNew.Fault08) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[ 9].Item), String(Gl_ic2_FaultsResultNew.Fault09) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[10].Item), String(Gl_ic2_FaultsResultNew.Fault10) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[11].Item), String(Gl_ic2_FaultsResultNew.Fault11) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[12].Item), String(Gl_ic2_FaultsResultNew.Fault12) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[13].Item), String(Gl_ic2_FaultsResultNew.Fault13) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[14].Item), String(Gl_ic2_FaultsResultNew.Fault14) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[15].Item), String(Gl_ic2_FaultsResultNew.Fault15) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[16].Item), String(Gl_ic2_FaultsResultNew.Fault16) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[17].Item), String(Gl_ic2_FaultsResultNew.Fault17) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[18].Item), String(Gl_ic2_FaultsResultNew.Fault18) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[19].Item), String(Gl_ic2_FaultsResultNew.Fault19) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[20].Item), String(Gl_ic2_FaultsResultNew.Fault20) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[21].Item), String(Gl_ic2_FaultsResultNew.Fault21) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[22].Item), String(Gl_ic2_FaultsResultNew.Fault22) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[23].Item), String(Gl_ic2_FaultsResultNew.Fault23) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[24].Item), String(Gl_ic2_FaultsResultNew.Fault24) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[25].Item), String(Gl_ic2_FaultsResultNew.Fault25) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[26].Item), String(Gl_ic2_FaultsResultNew.Fault26) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[27].Item), String(Gl_ic2_FaultsResultNew.Fault27) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[28].Item), String(Gl_ic2_FaultsResultNew.Fault28) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[29].Item), String(Gl_ic2_FaultsResultNew.Fault29) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[30].Item), String(Gl_ic2_FaultsResultNew.Fault30) );
        SendMQTT (Gl_Topic1, loc_Topic2, String(Gl_ic2_Faults[31].Item), String(Gl_ic2_FaultsResultNew.Fault31) );
    break;   
    }     
  }
}

void PublishSerialStats (byte loc_Status){
  if (loc_Status != Nothing) {
    SendMQTT(Gl_Topic1, "TelegramErrors" ,String(Gl_ParameterList[loc_Status].TelegramTypeName), String(Gl_ParameterList[loc_Status].TelegramErrors));  
    SendMQTT(Gl_Topic1, "TelegramMaxRetrieveTime" ,String(Gl_ParameterList[loc_Status].TelegramTypeName), String(Gl_ParameterList[loc_Status].TelegramMaxRetrieveTime));  
    SendMQTT(Gl_Topic1, "InterGasGWUptime" , "Time", String(millis()));  
  }
}

double getDouble(byte msb, byte lsb) {
    // Combine msb and lsb into a signed 16-bit integer (for clarity)
    double value = (msb << 8) | lsb;  // Shift msb to the left and combine with lsb

    // If the most significant bit of msb is set, the value is negative
    if (msb > 127) {
        value = value - 65536;  // Convert to negative by adjusting the range
    }

    // Return the result scaled by 100
    return (double)value / 100.0;
}

int getInt(byte msb, byte lsb) {
      int loc_value;
      loc_value = int(lsb * 256 + msb);
    return loc_value;
}

int Getsigned(byte lsb){
    int loc_Value;
    if (lsb > 127) {
        loc_Value = -int((lsb ^ 255) + 1);
    }
    else {
        loc_Value = lsb;
    }
    return loc_Value;
}

int Get_CRC(char byte1, char byte2, char byte3, char byte4) {
  // Maak een lege string voor de CRC
  String crc= "";
  char crc1[8] = "";

  // Voor elke byte, converteer naar hex en zorg voor een lengte van 2 tekens
  crc += String(byte1, HEX);
  crc += String(byte2, HEX);
  crc += String(byte3, HEX);
  crc += String(byte4, HEX);

  // Als de lengte van de hex-string één teken is, voeg dan een voorloopnul toe
  crc.replace("0x", ""); // Verwijder de "0x" prefix die String(byte, HEX) toevoegt
  while (crc.length() < 8) {
    crc = "0" + crc; // Voeg voorloopnullen toe tot de lengte 8 tekens is
  }

  return crc.toInt();
}




byte CheckFault (byte loc_FaultOccurences){
  byte loc_Value = 0;

  if (loc_FaultOccurences == 255) {
    loc_Value = 0;    
  }
  else {
    loc_Value = loc_FaultOccurences;
  }

  return loc_Value;
}




void EmptySerialBuffer2() {
  while(InterGasSerial.available() != 0)
  {
      (void)InterGasSerial.read();
  }
  
}


void EmptySerialBuffer() {
  byte loc_ByteRecieved;
  while (InterGasSerial.available() > 0) {
    loc_ByteRecieved = InterGasSerial.read();
    Serial.println("Cleaning Serial Buffer");
    }
  }

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Conneting to WiFi...");
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT");
      client.subscribe("IntergasGW/sensor");
    } else {
      Serial.print("Fout, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void SendMQTT( String loc_Topiclevel1, String loc_Topiclevel2, String loc_Topiclevel3, String MQTTValue ) {
  String loc_MQTTTopic;
  loc_MQTTTopic = loc_Topiclevel1 + "/" + loc_Topiclevel2 + "/" + loc_Topiclevel3;
  client.publish(loc_MQTTTopic.c_str(), MQTTValue.c_str());
}


void PrintTelegram (String loc_Topiclevel1, String loc_Topiclevel2, String loc_Topiclevel3, String loc_Value, String loc_Units){
 Serial.printf ("%-12s %-12s %-20s %10s %-10s \n", loc_Topiclevel1, loc_Topiclevel2, loc_Topiclevel3.c_str(), loc_Value, loc_Units );
}
