#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <StateMachine.h>  
//
#include <EnvironmentSettings.h>
//
//Define Software Serial ports
#define Intergas_TXport                D6    // output
#define Intergas_RXport                D7    // input
//
#define          ledPin               2
//
// Init Wifi Client and MQTT
WiFiClient   espClient;
PubSubClient client(espClient);
//
// Init staemachine
StateMachine IntergasStateMachine = StateMachine();
//
struct States {
  const char*           State;
};


struct States IGSMStates[25] = {
// 1234567890123456789012345678901 -> 31 + 1 char = 32
{ "IGSM_SetBurnerRequest"       		},
{ "IGSM_SendRequestToSerialPort1"		},
{ "IGSM_GetDataFromSerialPort1"	  	},
{ "IGSM_ParseDataFromSerialPort1"		},
{ "IGSM_SetBurnerSettings"	      	},
{ "IGSM_EmptySerialBuffer1"		      },
{ "IGSM_EmptySerialBuffer2"     		},
{ "IGSM_DetermineRequest"	        	},
{ "IGSM_SendRequestToSerialPort2"		},
{ "IGSM_GetDataFromSerialPort2"		  },
{ "IGSM_ParseDataFromSerialPort2"		},
{ "IGSM_PublishDataFromSerialPort1"	},
{ "IGSM_DisplayDataFromSerialPort1"	},
{ "IGSM_PublishDataFromSerialPort2"	},
{ "IGSM_DisplayDataFromSerialPort2"	},
{ "IGSM_PublishSerialStats"	      	}
};


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


struct TelegramObjects {
  const char*           Item;
  String                Formatted;
  const char*           Units;
  const char*           ExtraInfo;
};

struct TelegramObjects GL_ic2_Status[31] = {
  { "t1"                 , "" , "(  °C )", nullptr },
  { "t2"                 , "" , "(  °C )", nullptr },
  { "t3"                 , "" , "(  °C )", nullptr },
  { "t4"                 , "" , "(  °C )", nullptr },
  { "t5"                 , "" , "(  °C )", nullptr },
  { "t6"                 , "" , "(  °C )", nullptr },
  { "ch_pressure"        , "" , "( Bar )", nullptr },
  { "temp_set"           , "" , "(  °C )", nullptr },
  { "fanspeed_set"       , "" , "( RPM )", nullptr },
  { "fan_speed"          , "" , "( RPM )", nullptr },
  { "fan_pwm"            , "" , "( --- )", nullptr },
  { "io_curr"            , "" , "( --- )", nullptr },
  { "display_code"       , "" , "( --- )", nullptr },
  { "gp_switch"          , "" , "( --- )", nullptr },
  { "tap_switch"         , "" , "( --- )", nullptr },
  { "roomtherm"          , "" , "( --- )", nullptr },
  { "pump"               , "" , "( --- )", nullptr },
  { "dwk"                , "" , "( --- )", nullptr },
  { "alarm_status"       , "" , "( --- )", nullptr },
  { "ch_cascade_relay"   , "" , "( --- )", nullptr },
  { "opentherm"          , "" , "( --- )", nullptr },
  { "gasvalve"           , "" , "( --- )", nullptr },
  { "spark"              , "" , "( --- )", nullptr },
  { "io_signal"          , "" , "( --- )", nullptr },
  { "ch_ot_disabled"     , "" , "( --- )", nullptr },
  { "low_water_pressure" , "" , "( --- )", nullptr },
  { "pressure_sensor"    , "" , "( --- )", nullptr },
  { "burner_block"       , "" , "( --- )", nullptr },
  { "grad_flag"          , "" , "( --- )", nullptr }
};

struct TelegramObjects GL_ic2_Code[15] = {
  { "opentherm"               , "" , "( --- )" , nullptr },
  { "boiler_ext"              , "" , "( --- )" , nullptr },
  { "frost"                   , "" , "( --- )" , nullptr },
  { "central_heating_rf"      , "" , "( --- )" , nullptr },
  { "tapwater_int"            , "" , "( --- )" , nullptr },
  { "sensortest"              , "" , "( --- )" , nullptr },
  { "zone_heating"            , "" , "( --- )" , nullptr },
  { "standby"                 , "" , "( --- )" , nullptr },
  { "postrun_boiler"          , "" , "( --- )" , nullptr },
  { "service"                 , "" , "( --- )" , nullptr },
  { "tapwater"                , "" , "( --- )" , nullptr },
  { "postrun_ch"              , "" , "( --- )" , nullptr },
  { "boiler_int"              , "" , "( --- )" , nullptr },
  { "buffer"                  , "" , "( --- )" , nullptr },
};

struct TelegramObjects Gl_ic2_BurnerVersion[4] = {
  { "h_version"               , "" , "( --- )" , nullptr },
  { "s_version"               , "" , "( --- )" , nullptr },
  { "checksum1"               , "" , "( --- )" , nullptr },
  { "checksum2"               , "" , "( --- )" , nullptr },
};

struct TelegramObjects Gl_ic2_Burner[7] = {
  { "interrupt_time"          , "" , "( --- )" , nullptr },
  { "interrupt_load"          , "" , "(   % )" , nullptr },
  { "main_load"               , "" , "(   % )" , nullptr },
  { "net_frequency"           , "" , "(  Hz )" , nullptr },
  { "voltage_reference"       , "" , "(   V )" , nullptr },
  { "checksum1"               , "" , "( --- )" , nullptr },
  { "checksum2"               , "" , "( --- )" , nullptr },
};

struct TelegramObjects Gl_ic2_Faults[32] = {
  { "Fault00"                 , "" , "( --- )" , nullptr },
  { "Fault01"                 , "" , "( --- )" , nullptr },
  { "Fault02"                 , "" , "( --- )" , nullptr },
  { "Fault03"                 , "" , "( --- )" , nullptr },
  { "Fault04"                 , "" , "( --- )" , nullptr },
  { "Fault05"                 , "" , "( --- )" , nullptr },
  { "Fault06"                 , "" , "( --- )" , nullptr },
  { "Fault07"                 , "" , "( --- )" , nullptr },
  { "Fault08"                 , "" , "( --- )" , nullptr },
  { "Fault09"                 , "" , "( --- )" , nullptr },
  { "Fault10"                 , "" , "( --- )" , nullptr },
  { "Fault11"                 , "" , "( --- )" , nullptr },
  { "Fault12"                 , "" , "( --- )" , nullptr },
  { "Fault13"                 , "" , "( --- )" , nullptr },
  { "Fault14"                 , "" , "( --- )" , nullptr },
  { "Fault15"                 , "" , "( --- )" , nullptr },
  { "Fault16"                 , "" , "( --- )" , nullptr },
  { "Fault17"                 , "" , "( --- )" , nullptr },
  { "Fault18"                 , "" , "( --- )" , nullptr },
  { "Fault19"                 , "" , "( --- )" , nullptr },
  { "Fault20"                 , "" , "( --- )" , nullptr },
  { "Fault21"                 , "" , "( --- )" , nullptr },
  { "Fault22"                 , "" , "( --- )" , nullptr },
  { "Fault23"                 , "" , "( --- )" , nullptr },
  { "Fault24"                 , "" , "( --- )" , nullptr },
  { "Fault25"                 , "" , "( --- )" , nullptr },
  { "Fault26"                 , "" , "( --- )" , nullptr },
  { "Fault27"                 , "" , "( --- )" , nullptr },
  { "Fault28"                 , "" , "( --- )" , nullptr },
  { "Fault29"                 , "" , "( --- )" , nullptr },
  { "Fault30"                 , "" , "( --- )" , nullptr },
  { "Fault31"                 , "" , "( --- )" , nullptr },
};

struct TelegramObjects Gl_ic2_Parameters[30] = {
  { "heater_on"               , "" , "( --- )" , nullptr },
  { "comfort_mode"            , "" , "( --- )" , nullptr },
  { "ch_set_max"              , "" , "( --- )" , nullptr },
  { "dhw_set"                 , "" , "( --- )" , nullptr },
  { "eco_days"                , "" , "( --- )" , nullptr },
  { "comfort_set"             , "" , "( --- )" , nullptr },
  { "dhw_at_night"            , "" , "( --- )" , nullptr },
  { "ch_at_night"             , "" , "( --- )" , nullptr },
  { "param_1"                 , "" , "( --- )" , nullptr },
  { "param_2"                 , "" , "( --- )" , nullptr },
  { "param_3"                 , "" , "( --- )" , nullptr },
  { "param_4"                 , "" , "( --- )" , nullptr },
  { "param_5"                 , "" , "( --- )" , nullptr },
  { "param_6"                 , "" , "( --- )" , nullptr },
  { "param_7"                 , "" , "( --- )" , nullptr },
  { "param_8"                 , "" , "( --- )" , nullptr },
  { "param_9"                 , "" , "( --- )" , nullptr },
  { "param_A"                 , "" , "( --- )" , nullptr },
  { "param_b"                 , "" , "( --- )" , nullptr },
  { "param_C"                 , "" , "( --- )" , nullptr },
  { "param_d"                 , "" , "( --- )" , nullptr },
  { "param_E"                 , "" , "( --- )" , nullptr },
  { "param_E."                , "" , "( --- )" , nullptr },
  { "param_F"                 , "" , "( --- )" , nullptr },
  { "param_H"                 , "" , "( --- )" , nullptr },
  { "param_n"                 , "" , "( --- )" , nullptr },
  { "param_o"                 , "" , "( --- )" , nullptr },
  { "param_P"                 , "" , "( --- )" , nullptr },
  { "param_r"                 , "" , "( --- )" , nullptr },
  { "param_F."                , "" , "( --- )" , nullptr },
};

struct TelegramObjects Gl_ic2_OperatingHours[8] = {
  { "line_power_connected"    , "" , "(   h )" , nullptr },
  { "line_power_disconnect"   , "" , "(   # )" , nullptr },
  { "ch_function"             , "" , "(   h )" , nullptr },
  { "dhw_function"            , "" , "(   h )" , nullptr },
  { "burnerstarts"            , "" , "(   # )" , nullptr },
  { "ignition_failed"         , "" , "(   # )" , nullptr },
  { "flame_lost"              , "" , "(   # )" , nullptr },
  { "reset"                   , "" , "(   # )" , nullptr },
};



struct ParameterRecord {
  const byte            Telegram;
  const char*           SerialCommand;
  const bool            ic2;
  const bool            ic3;
  const bool            DebugSerialRxTx;
  const bool            WriteResultsToSerial;
  const bool            WriteResultsToMQTT;
  const byte            TelegramLength;
  const unsigned long   TelegramReadSchedule;
  const byte            TelegramMaxDuration;
  const char*           TelegramTypeName;
  unsigned long         TelegramLastTimeRun;
  bool                  TelegramValid;
  long                  TelegramErrors;
  int                   TelegramMaxRetrieveTime;
  int                   TelegramRetrieveTime;
}; 

const byte Gl_NoOfTelegrams = 11;   // N + 1 = 10 + 1 = 11

struct  ParameterRecord Gl_ParameterList[Gl_NoOfTelegrams] =
{
  {  0, ""       ,  true,  true, false, true,   true,  0,         0,     0, "Nothing"            , 0, 0, 0, 0, 0}, // Not used
  {  1, "S?\r"   ,  true, false, false, false,  true, 32,      1000,   155, "ic2_Status"         , 0, 0, 0, 0, 0}, // Max 150 ms, Prefered setting 160
  {  2, "S2\r"   , false, false, false, false, false, 32, 999999999,   155, "StatusExtra"        , 0, 0, 0, 0, 0}, // To be done
  {  3, "REV"    ,  true, false, false, false,  true, 32,   3600000,   155, "ic2_BurnerVersion"  , 0, 0, 0, 0, 0}, // Max 135 ms, Prefered setting 145
  {  4, "CRC"    ,  true, false, false, false,  true, 32,     60000,   155, "ic2_Burner"         , 0, 0, 0, 0, 0}, // Max 100 ms, Prefered setting 110
  {  5, "HN\r"   ,  true, false, false, false,  true, 32,     60000,   155, "ic2_OperatingHours" , 0, 0, 0, 0, 0}, // Max 130 ms, Prefered setting 140
  {  6, "EN\r"   ,  true, false, false, false,  true, 32,     10000,   155, "ic2_Faults"         , 0, 0, 0, 0, 0}, // Max 150 ms, Prefered setting 160
  {  7, "V?\r"   ,  true, false, false, false,  true, 32,     60000,   155, "ic2_Parameters"     , 0, 0, 0, 0, 0}, // Max 135 ms, Prefered setting 145
  {  8, "LX?"    , false, false, false, false, false, 32, 999999999,   155, "parametersExtra"    , 0, 0, 0, 0, 0}, // To be done
  {  9, "B?\r"   , false, false, false, false, false, 32, 999999999,   155, "infoblok"           , 0, 0, 0, 0, 0}, // To be done
  { 10, "B2\r"   , false, false, false, false, false, 32, 999999999,   155, "infoblokExtra"      , 0, 0, 0, 0, 0}  // To be done
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
  int  checksum1; 
  int  checksum2;
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
bool          Gl_DebugState                           = true;

byte          Gl_Request                              = 0;

unsigned char Gl_RecievedArray[64];    

bool          Gl_ic2                                  = 0;
bool          Gl_ic3                                  = 0;

//Define Serial port
SoftwareSerial InterGasSerial(Intergas_RXport, Intergas_TXport); 

void MQTTconnect() {
  while (!client.connected()) {
    Serial.print        ("MQTT - Connecting MQTT   : ");
    //
    if (client.connect("ESP8266Client")) {
      Serial.println    ("Connected to MQTT");
      delay(500);
    }
    else {
      Serial.print      ("Error, rc=");
      Serial.println    (client.state());
      delay(500);
    }
  }
}


void SendMQTT(const char* loc_MQTTTopiclevel1, const char* loc_MQTTTopiclevel2, const char* loc_MQTTTopiclevel3, const char* loc_MQTTValue ) {
  char loc_MQTTTopic[200];
  //
  strcpy(loc_MQTTTopic, loc_MQTTTopiclevel1);
  strcat(loc_MQTTTopic, "/");
  strcat(loc_MQTTTopic, loc_MQTTTopiclevel2);
  strcat(loc_MQTTTopic, "/");
  strcat(loc_MQTTTopic, loc_MQTTTopiclevel3);
  //
  if (!client.connected()) {
    MQTTconnect();
  }
  //
  client.publish(loc_MQTTTopic, loc_MQTTValue);
}


void PrintTelegram (const char* loc_Topiclevel1, const char* loc_Topiclevel2, const char* loc_Topiclevel3, const char* loc_Value, const char* loc_Units){
  Serial.printf ("%-12s %-12s %-20s %10s %-10s \n", loc_Topiclevel1, loc_Topiclevel2, loc_Topiclevel3, loc_Value, loc_Units );
}


unsigned long Duration(unsigned long loc_time1, unsigned long loc_time2 ){ 
  //
  // Needed beacause roll-over of millis() (max 4294967295 ms)
  //
  // Example
  //
  //  loc_time1   loc_time1   Duration      
  // 4294917296  4294917296          0
  // 4294917297  4294917296          1
  // 4294917298  4294917296          2
  //     
  // 4294967295  4294917296      49999
  //          0  4294917296      50000
  //          1  4294917296      50001
  //
  //       9999  4294917296      59999
  //      10000  4294917296      60000
  //
  unsigned long loc_duration = 0;
  loc_duration = loc_time1 - loc_time2;
  return loc_duration;

}


double getDouble(byte msb, byte lsb) {
  // Combine msb and lsb into a signed 16-bit integer (for clarity)
  double value = (msb << 8) | lsb;  // Shift msb to the left and combine with lsb
  //
  // If the most significant bit of msb is set, the value is negative
  if (msb > 127) {
      value = value - 65536;  // Convert to negative by adjusting the range
  }
  //
  // Return the result scaled by 100
  return (double)value / 100.0;
}


int getInt(byte msb, byte lsb) {
    int loc_value;
    loc_value = int(msb * 256 + lsb);
  return loc_value;
}


int getSigned(byte lsb){
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
  //
  byte loc_Value = 0;
  //
  if (loc_FaultOccurences == 255) {
    loc_Value = 0;    
  }
  else {
    loc_Value = loc_FaultOccurences;
  }
  return loc_Value;
}


void WriteState() {
  if ( Gl_DebugState == true ) {
    Serial.printf     (  "State: %33s \n", IGSMStates[IntergasStateMachine.currentState].State );
  }
}


void SetBurnerRequest() {
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
  Gl_Request = ic2_BurnerVersion;
}


void SetBurnerSettings() {
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
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


void EmptySerialBuffer() {
  //
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
  while (InterGasSerial.available() > 0) {
    InterGasSerial.read();
  }
}


void DetermineRequest () {
  //  
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
                Gl_Request  = 0;
  unsigned long CurrentTime = millis();
  //
  struct loc_ParameterRecord {
    bool                  ic2;
    bool                  ic3;
    unsigned long         OverTime;
  }; 
  //
  loc_ParameterRecord loc_ParameterList[Gl_NoOfTelegrams];
  //
  for (int counter = 1; counter < Gl_NoOfTelegrams; counter++) {
    loc_ParameterList[counter].ic2                 = Gl_ParameterList[counter].ic2;
    loc_ParameterList[counter].ic3                 = Gl_ParameterList[counter].ic3;
    //
    if (Duration(CurrentTime, Gl_ParameterList[counter].TelegramLastTimeRun) > Gl_ParameterList[counter].TelegramReadSchedule ) {
      loc_ParameterList[counter].OverTime     = (Duration(CurrentTime, Gl_ParameterList[counter].TelegramLastTimeRun) - Gl_ParameterList[counter].TelegramReadSchedule);
    }
    else {
      loc_ParameterList[counter].OverTime     = 0;
    }
  };
//
int             maxIndex = 0;
unsigned long   maxVal   = 0;

for (int counter = 1; counter < Gl_NoOfTelegrams; counter++) {
  if ( (loc_ParameterList[counter].OverTime > maxVal) and (loc_ParameterList[counter].ic2 == Gl_ic2) and (loc_ParameterList[counter].ic3 == Gl_ic3) ) {
    maxVal   = loc_ParameterList[counter].OverTime;
    maxIndex = counter;
  }
}
//
Gl_Request = maxIndex;
}


void SendRequestToSerialPort () {
  //
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
  // OnboardLed On
  digitalWrite(ledPin,HIGH);
  //
  Gl_ParameterList[Gl_Request].TelegramLastTimeRun = millis();
  InterGasSerial.write(Gl_ParameterList[Gl_Request].SerialCommand);
  // 
  if (Gl_ParameterList[Gl_Request].DebugSerialRxTx == true){
    Serial.printf ("TX : Data: %-3s \n", Gl_ParameterList[Gl_Request].SerialCommand );
  }
  //
  // OnboardLed Off
  digitalWrite(ledPin,HIGH);
}

void GetDataFromSerialPort () {
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }

  //
    unsigned char loc_ByteRecieved;
    byte          loc_ArrayIndex = 0;
    //
    unsigned long loc_TimeStartRecievingTelegram;
    //
    unsigned long loc_TelegramMaxDuration;
    //
    byte          loc_BytesInSerialBuffer;
    byte          loc_DebugArray[Gl_ParameterList[Gl_Request].TelegramLength][3];
    //    
    if ( Gl_ParameterList[Gl_Request].DebugSerialRxTx == false ) {
        loc_TelegramMaxDuration = Gl_ParameterList[Gl_Request].TelegramMaxDuration;  
    }
    else {
        loc_TelegramMaxDuration = Gl_ParameterList[Gl_Request].TelegramMaxDuration + Gl_SerialRecieveTimeoutExtraDubugTime;
    }
    //
    loc_TimeStartRecievingTelegram = millis();
    //  
    while ( (Duration(millis(), loc_TimeStartRecievingTelegram) <= loc_TelegramMaxDuration) and (loc_ArrayIndex < Gl_ParameterList[Gl_Request].TelegramLength) ){
        loc_BytesInSerialBuffer = InterGasSerial.available();
        if ( loc_BytesInSerialBuffer > 0 ) {
        loc_ByteRecieved = InterGasSerial.read();
            if ( Gl_ParameterList[Gl_Request].DebugSerialRxTx == true ){
            loc_DebugArray[loc_ArrayIndex][0] = loc_BytesInSerialBuffer;
            loc_DebugArray[loc_ArrayIndex][1] = loc_ArrayIndex;
            loc_DebugArray[loc_ArrayIndex][2] = loc_ByteRecieved;
            }
        Gl_RecievedArray[loc_ArrayIndex] = loc_ByteRecieved;
        loc_ArrayIndex++;
        }
    } 
    //

    Gl_ParameterList[Gl_Request].TelegramRetrieveTime         =  Duration (millis(), loc_TimeStartRecievingTelegram);

    if (Gl_ParameterList[Gl_Request].TelegramMaxRetrieveTime < Gl_ParameterList[Gl_Request].TelegramRetrieveTime) {
        Gl_ParameterList[Gl_Request].TelegramMaxRetrieveTime = Gl_ParameterList[Gl_Request].TelegramRetrieveTime;  
    }

    //

    if ( loc_ArrayIndex == Gl_ParameterList[Gl_Request].TelegramLength ) {
      Gl_ParameterList[Gl_Request].TelegramValid = true;
    }
    else {
      Gl_ParameterList[Gl_Request].TelegramValid  = false;
      Gl_ParameterList[Gl_Request].TelegramErrors = Gl_ParameterList[Gl_Request].TelegramErrors + 1;
    }
    //
    //
    if (Gl_ParameterList[Gl_Request].DebugSerialRxTx == true){
      for (byte counter = 0; counter < loc_ArrayIndex; counter++ ){
        Serial.printf ("RX : Data: %-3s Index: %-3s BytesInBuffer: %-3s \n", String(loc_DebugArray[counter][2]).c_str() , String(loc_DebugArray[counter][1]).c_str(), String(loc_DebugArray[counter][0]).c_str() );
      }
    }
    //
    //    
    if ( Gl_ParameterList[Gl_Request].DebugSerialRxTx == true ){
      if ( loc_ArrayIndex == Gl_ParameterList[Gl_Request].TelegramLength ){
        Serial.printf ("Type: %-20s Telegram: %4s   RecievingTime: %4s ms\n", String(Gl_ParameterList[Gl_Request].TelegramTypeName).c_str() , "Good", String(Gl_ParameterList[Gl_Request].TelegramRetrieveTime).c_str() );
      }
      else {
        Serial.printf ("Type: %-20s Telegram: %4s   RecievingTime: %4s ms\n", String(Gl_ParameterList[Gl_Request].TelegramTypeName).c_str() , "Bad" , String(Gl_ParameterList[Gl_Request].TelegramRetrieveTime).c_str() );
      }
    }
}

void ParseDataFromSerialPort (){
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  byte loc_VPosition     = 0;
  //
  switch (Gl_Request) {
    case ic2_Status:
      Gl_ic2_StatusResultNew.t1                               =   getDouble(Gl_RecievedArray[ 1], Gl_RecievedArray[ 0]);
      Gl_ic2_StatusResultNew.t2                               =   getDouble(Gl_RecievedArray[ 3], Gl_RecievedArray[ 2]);
      Gl_ic2_StatusResultNew.t3                               =   getDouble(Gl_RecievedArray[ 5], Gl_RecievedArray[ 4]);
      Gl_ic2_StatusResultNew.t4                               =   getDouble(Gl_RecievedArray[ 7], Gl_RecievedArray[ 6]);
      Gl_ic2_StatusResultNew.t5                               =   getDouble(Gl_RecievedArray[ 9], Gl_RecievedArray[ 8]);
      Gl_ic2_StatusResultNew.t6                               =   getDouble(Gl_RecievedArray[11], Gl_RecievedArray[10]);
      Gl_ic2_StatusResultNew.ch_pressure                      =   getDouble(Gl_RecievedArray[13], Gl_RecievedArray[12]);
      Gl_ic2_StatusResultNew.temp_set                         =   getDouble(Gl_RecievedArray[15], Gl_RecievedArray[14]);
      Gl_ic2_StatusResultNew.fanspeed_set                     =   getDouble(Gl_RecievedArray[17], Gl_RecievedArray[16]) * 100;
      Gl_ic2_StatusResultNew.fan_speed                        =   getDouble(Gl_RecievedArray[19], Gl_RecievedArray[18]) * 100;
      Gl_ic2_StatusResultNew.fan_pwm                          =   getDouble(Gl_RecievedArray[21], Gl_RecievedArray[20]) *  10;
      Gl_ic2_StatusResultNew.io_curr                          =   getDouble(Gl_RecievedArray[21], Gl_RecievedArray[22])      ;
      Gl_ic2_StatusResultNew.display_code                     =             Gl_RecievedArray[24];
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
      Gl_ic2_ParametersResultNew.heater_on                    =   getSigned(Gl_RecievedArray[ 0]);
      Gl_ic2_ParametersResultNew.comfort_mode                 =   getSigned(Gl_RecievedArray[ 1]);
      Gl_ic2_ParametersResultNew.ch_set_max                   =   getSigned(Gl_RecievedArray[ 2]);
      Gl_ic2_ParametersResultNew.dhw_set                      =   getSigned(Gl_RecievedArray[ 3]);
      Gl_ic2_ParametersResultNew.eco_days                     =   getSigned(Gl_RecievedArray[ 4]);
      Gl_ic2_ParametersResultNew.comfort_set                  =   getSigned(Gl_RecievedArray[ 5]);
      Gl_ic2_ParametersResultNew.dhw_at_night                 =   getSigned(Gl_RecievedArray[ 6]);
      Gl_ic2_ParametersResultNew.ch_at_night                  =   getSigned(Gl_RecievedArray[ 7]);
      Gl_ic2_ParametersResultNew.param_1                      =   getSigned(Gl_RecievedArray[ 8]);
      Gl_ic2_ParametersResultNew.param_2                      =   getSigned(Gl_RecievedArray[ 9]);
      Gl_ic2_ParametersResultNew.param_3                      =   getSigned(Gl_RecievedArray[10]);
      Gl_ic2_ParametersResultNew.param_4                      =   getSigned(Gl_RecievedArray[11]);
      Gl_ic2_ParametersResultNew.param_5                      =   getSigned(Gl_RecievedArray[12]);
      Gl_ic2_ParametersResultNew.param_6                      =   getSigned(Gl_RecievedArray[13]);
      Gl_ic2_ParametersResultNew.param_7                      =   getSigned(Gl_RecievedArray[14]);
      Gl_ic2_ParametersResultNew.param_8                      =   getSigned(Gl_RecievedArray[15]);
      Gl_ic2_ParametersResultNew.param_9                      =   getSigned(Gl_RecievedArray[16]);
      Gl_ic2_ParametersResultNew.param_A                      =   getSigned(Gl_RecievedArray[17]);
      Gl_ic2_ParametersResultNew.param_b                      =   getSigned(Gl_RecievedArray[18]);
      Gl_ic2_ParametersResultNew.param_C                      =   getSigned(Gl_RecievedArray[19]);
      Gl_ic2_ParametersResultNew.param_c                      =   getSigned(Gl_RecievedArray[20]);
      Gl_ic2_ParametersResultNew.param_d                      =   getSigned(Gl_RecievedArray[21]);
      Gl_ic2_ParametersResultNew.param_E                      =   getSigned(Gl_RecievedArray[22]);
      Gl_ic2_ParametersResultNew.param_Edot                   =   getSigned(Gl_RecievedArray[23]);
      Gl_ic2_ParametersResultNew.param_F                      =   getSigned(Gl_RecievedArray[24]);
      Gl_ic2_ParametersResultNew.param_H                      =   getSigned(Gl_RecievedArray[25]);
      Gl_ic2_ParametersResultNew.param_n                      =   getSigned(Gl_RecievedArray[26]);
      Gl_ic2_ParametersResultNew.param_o                      =   getSigned(Gl_RecievedArray[27]);
      Gl_ic2_ParametersResultNew.param_P                      =   getSigned(Gl_RecievedArray[28]);
      Gl_ic2_ParametersResultNew.param_r                      =   getSigned(Gl_RecievedArray[29]);
      Gl_ic2_ParametersResultNew.param_Fdot                   =   getSigned(Gl_RecievedArray[30]);
    break;    

    case ic2_OperatingHours:
      Gl_ic2_OperatingHoursResultNew.line_power_connected     =   getInt(Gl_RecievedArray[ 1], Gl_RecievedArray[ 0]);
      Gl_ic2_OperatingHoursResultNew.line_power_disconnect    =   getInt(Gl_RecievedArray[ 3], Gl_RecievedArray[ 2]);
      Gl_ic2_OperatingHoursResultNew.ch_function              =   getInt(Gl_RecievedArray[ 5], Gl_RecievedArray[ 4]);
      Gl_ic2_OperatingHoursResultNew.dhw_function             =   getInt(Gl_RecievedArray[ 7], Gl_RecievedArray[ 6]);
      Gl_ic2_OperatingHoursResultNew.burnerstarts             =   getInt(Gl_RecievedArray[ 9], Gl_RecievedArray[ 8]);
      Gl_ic2_OperatingHoursResultNew.ignition_failed          =   getInt(Gl_RecievedArray[11], Gl_RecievedArray[10]);
      Gl_ic2_OperatingHoursResultNew.flame_lost               =   getInt(Gl_RecievedArray[13], Gl_RecievedArray[12]);
      Gl_ic2_OperatingHoursResultNew.reset                    =   getInt(Gl_RecievedArray[15], Gl_RecievedArray[14]);
    break;

    case ic2_BurnerVersion:
      //
      for (byte Position = 0; Position < 32; Position++) {
        if ((char(Gl_RecievedArray[Position])) == 'V') {
          loc_VPosition = Position;
        }
      }    
      //
      // Retrieve Hardware version
      for (byte Position = 0; Position < loc_VPosition; Position++) {
        Gl_ic2_BurnerVersionResultNew.h_version[Position]     = char(Gl_RecievedArray[Position]);
      }
      //
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
      //
      Gl_ic2_BurnerVersionResultNew.checksum1              =    Get_CRC(char(Gl_RecievedArray[24]) , char(Gl_RecievedArray[25]) , char(Gl_RecievedArray[26]) , char(Gl_RecievedArray[27]));
      Gl_ic2_BurnerVersionResultNew.checksum2              =    Get_CRC(char(Gl_RecievedArray[28]) , char(Gl_RecievedArray[29]) , char(Gl_RecievedArray[30]) , char(Gl_RecievedArray[31]));
    break;

  case ic2_Burner:
    Gl_ic2_BurnerResultNew.interrupt_time                 =    getInt(Gl_RecievedArray[1], Gl_RecievedArray[0]) / (float)5   ;
    Gl_ic2_BurnerResultNew.interrupt_load                 =    getInt(Gl_RecievedArray[3], Gl_RecievedArray[2]) / (float)6.25;
    Gl_ic2_BurnerResultNew.main_load                      =    getInt(Gl_RecievedArray[5], Gl_RecievedArray[4]) / (float)8   ;
    //
    if (getInt(Gl_RecievedArray[6],Gl_RecievedArray[7]) > 0){
      Gl_ic2_BurnerResultNew.net_frequency              =    (float)2000 / getInt(Gl_RecievedArray[7], Gl_RecievedArray[6]) ;  
    }
    else{
      Gl_ic2_BurnerResultNew.net_frequency              =    0;
    }
    //
    Gl_ic2_BurnerResultNew.voltage_reference              =    (getInt(Gl_RecievedArray[9],Gl_RecievedArray[8]) * 5) / (float)1024;
    Gl_ic2_BurnerResultNew.checksum1                      =    Get_CRC(char(Gl_RecievedArray[24]) , char(Gl_RecievedArray[25]) , char(Gl_RecievedArray[26]) , char(Gl_RecievedArray[27]));
    Gl_ic2_BurnerResultNew.checksum2                      =    Get_CRC(char(Gl_RecievedArray[28]) , char(Gl_RecievedArray[29]) , char(Gl_RecievedArray[30]) , char(Gl_RecievedArray[31]));
  break;

  case ic2_Faults:
    Gl_ic2_FaultsResultNew.Fault00                        =     CheckFault(Gl_RecievedArray[ 0]);
    Gl_ic2_FaultsResultNew.Fault01                        =     CheckFault(Gl_RecievedArray[ 1]);
    Gl_ic2_FaultsResultNew.Fault02                        =     CheckFault(Gl_RecievedArray[ 2]);
    Gl_ic2_FaultsResultNew.Fault03                        =     CheckFault(Gl_RecievedArray[ 3]);
    Gl_ic2_FaultsResultNew.Fault04                        =     CheckFault(Gl_RecievedArray[ 4]);
    Gl_ic2_FaultsResultNew.Fault05                        =     CheckFault(Gl_RecievedArray[ 5]);
    Gl_ic2_FaultsResultNew.Fault06                        =     CheckFault(Gl_RecievedArray[ 6]);
    Gl_ic2_FaultsResultNew.Fault07                        =     CheckFault(Gl_RecievedArray[ 7]);
    Gl_ic2_FaultsResultNew.Fault08                        =     CheckFault(Gl_RecievedArray[ 8]);
    Gl_ic2_FaultsResultNew.Fault09                        =     CheckFault(Gl_RecievedArray[ 9]);
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

void FormatData (){
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
  switch (Gl_Request) {
    case ic2_Status:
      //
      GL_ic2_Status[ 0]        .Formatted = String(Gl_ic2_StatusResultNew.t1                            ).c_str();
      GL_ic2_Status[ 1]        .Formatted = String(Gl_ic2_StatusResultNew.t2                            ).c_str();
      GL_ic2_Status[ 2]        .Formatted = String(Gl_ic2_StatusResultNew.t3                            ).c_str();
      GL_ic2_Status[ 3]        .Formatted = String(Gl_ic2_StatusResultNew.t4                            ).c_str();
      GL_ic2_Status[ 4]        .Formatted = String(Gl_ic2_StatusResultNew.t5                            ).c_str();
      GL_ic2_Status[ 5]        .Formatted = String(Gl_ic2_StatusResultNew.t6                            ).c_str();
      GL_ic2_Status[ 6]        .Formatted = String(Gl_ic2_StatusResultNew.ch_pressure                   ).c_str();
      GL_ic2_Status[ 7]        .Formatted = String(Gl_ic2_StatusResultNew.temp_set                      ).c_str();
      GL_ic2_Status[ 8]        .Formatted = String(Gl_ic2_StatusResultNew.fanspeed_set                  ).c_str();
      GL_ic2_Status[ 9]        .Formatted = String(Gl_ic2_StatusResultNew.fan_speed                     ).c_str();
      GL_ic2_Status[10]        .Formatted = String(Gl_ic2_StatusResultNew.fan_pwm                       ).c_str();
      GL_ic2_Status[11]        .Formatted = String(Gl_ic2_StatusResultNew.io_curr                       ).c_str();
      GL_ic2_Status[12]        .Formatted = String(Gl_ic2_StatusResultNew.display_code                  ).c_str();
      GL_ic2_Status[13]        .Formatted = String(Gl_ic2_StatusResultNew.gp_switch                     ).c_str();
      GL_ic2_Status[14]        .Formatted = String(Gl_ic2_StatusResultNew.tap_switch                    ).c_str();
      GL_ic2_Status[15]        .Formatted = String(Gl_ic2_StatusResultNew.roomtherm                     ).c_str();
      GL_ic2_Status[16]        .Formatted = String(Gl_ic2_StatusResultNew.pump                          ).c_str();
      GL_ic2_Status[17]        .Formatted = String(Gl_ic2_StatusResultNew.dwk                           ).c_str();
      GL_ic2_Status[18]        .Formatted = String(Gl_ic2_StatusResultNew.alarm_status                  ).c_str();
      GL_ic2_Status[19]        .Formatted = String(Gl_ic2_StatusResultNew.ch_cascade_relay              ).c_str();
      GL_ic2_Status[20]        .Formatted = String(Gl_ic2_StatusResultNew.opentherm                     ).c_str();
      GL_ic2_Status[21]        .Formatted = String(Gl_ic2_StatusResultNew.gasvalve                      ).c_str();
      GL_ic2_Status[22]        .Formatted = String(Gl_ic2_StatusResultNew.spark                         ).c_str();
      GL_ic2_Status[23]        .Formatted = String(Gl_ic2_StatusResultNew.io_signal                     ).c_str();
      GL_ic2_Status[24]        .Formatted = String(Gl_ic2_StatusResultNew.ch_ot_disabled                ).c_str();
      GL_ic2_Status[25]        .Formatted = String(Gl_ic2_StatusResultNew.low_water_pressure            ).c_str();
      GL_ic2_Status[26]        .Formatted = String(Gl_ic2_StatusResultNew.pressure_sensor               ).c_str();
      GL_ic2_Status[27]        .Formatted = String(Gl_ic2_StatusResultNew.burner_block                  ).c_str();
      GL_ic2_Status[28]        .Formatted = String(Gl_ic2_StatusResultNew.grad_flag                     ).c_str();

    break;

  case ic2_Parameters:
  break;    

  case ic2_OperatingHours:
  break;

  case ic2_BurnerVersion:
  break;

  case ic2_Burner:
  break;

  case ic2_Faults:
  break;    
  }
}




void DisplayDataFromSerialPort (){
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
//
String loc_Topic2;
  switch(Gl_Request){
    case ic2_Status:
        loc_Topic2 = "ic2_Status"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 0]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t1                            ).c_str(), String(GL_ic2_Status[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 1]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t2                            ).c_str(), String(GL_ic2_Status[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 2]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t3                            ).c_str(), String(GL_ic2_Status[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 3]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t4                            ).c_str(), String(GL_ic2_Status[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 4]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t5                            ).c_str(), String(GL_ic2_Status[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 5]        .Item).c_str(), String(Gl_ic2_StatusResultNew.t6                            ).c_str(), String(GL_ic2_Status[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 6]        .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_pressure                   ).c_str(), String(GL_ic2_Status[ 6].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 7]        .Item).c_str(), String(Gl_ic2_StatusResultNew.temp_set                      ).c_str(), String(GL_ic2_Status[ 7].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 8]        .Item).c_str(), String(Gl_ic2_StatusResultNew.fanspeed_set                  ).c_str(), String(GL_ic2_Status[ 8].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 9]        .Item).c_str(), String(Gl_ic2_StatusResultNew.fan_speed                     ).c_str(), String(GL_ic2_Status[ 9].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[10]        .Item).c_str(), String(Gl_ic2_StatusResultNew.fan_pwm                       ).c_str(), String(GL_ic2_Status[10].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[11]        .Item).c_str(), String(Gl_ic2_StatusResultNew.io_curr                       ).c_str(), String(GL_ic2_Status[11].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[12]        .Item).c_str(), String(Gl_ic2_StatusResultNew.display_code                  ).c_str(), String(GL_ic2_Status[12].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[13]        .Item).c_str(), String(Gl_ic2_StatusResultNew.gp_switch                     ).c_str(), String(GL_ic2_Status[13].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[14]        .Item).c_str(), String(Gl_ic2_StatusResultNew.tap_switch                    ).c_str(), String(GL_ic2_Status[14].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[15]        .Item).c_str(), String(Gl_ic2_StatusResultNew.roomtherm                     ).c_str(), String(GL_ic2_Status[15].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[16]        .Item).c_str(), String(Gl_ic2_StatusResultNew.pump                          ).c_str(), String(GL_ic2_Status[16].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[17]        .Item).c_str(), String(Gl_ic2_StatusResultNew.dwk                           ).c_str(), String(GL_ic2_Status[17].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[18]        .Item).c_str(), String(Gl_ic2_StatusResultNew.alarm_status                  ).c_str(), String(GL_ic2_Status[18].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[19]        .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_cascade_relay              ).c_str(), String(GL_ic2_Status[19].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[20]        .Item).c_str(), String(Gl_ic2_StatusResultNew.opentherm                     ).c_str(), String(GL_ic2_Status[20].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[21]        .Item).c_str(), String(Gl_ic2_StatusResultNew.gasvalve                      ).c_str(), String(GL_ic2_Status[21].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[22]        .Item).c_str(), String(Gl_ic2_StatusResultNew.spark                         ).c_str(), String(GL_ic2_Status[22].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[23]        .Item).c_str(), String(Gl_ic2_StatusResultNew.io_signal                     ).c_str(), String(GL_ic2_Status[23].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[24]        .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_ot_disabled                ).c_str(), String(GL_ic2_Status[24].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[25]        .Item).c_str(), String(Gl_ic2_StatusResultNew.low_water_pressure            ).c_str(), String(GL_ic2_Status[25].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[26]        .Item).c_str(), String(Gl_ic2_StatusResultNew.pressure_sensor               ).c_str(), String(GL_ic2_Status[26].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[27]        .Item).c_str(), String(Gl_ic2_StatusResultNew.burner_block                  ).c_str(), String(GL_ic2_Status[27].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[28]        .Item).c_str(), String(Gl_ic2_StatusResultNew.grad_flag                     ).c_str(), String(GL_ic2_Status[28].Units).c_str());        
      
      Serial.println("Display State : ");
      loc_Topic2 = "ic2_Code"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 0]          .Item).c_str(), String(Gl_ic2_CodeResultNew.opentherm                       ).c_str(), String(GL_ic2_Code[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 1]          .Item).c_str(), String(Gl_ic2_CodeResultNew.boiler_ext                      ).c_str(), String(GL_ic2_Code[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 2]          .Item).c_str(), String(Gl_ic2_CodeResultNew.frost                           ).c_str(), String(GL_ic2_Code[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 3]          .Item).c_str(), String(Gl_ic2_CodeResultNew.central_heating_rf              ).c_str(), String(GL_ic2_Code[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 4]          .Item).c_str(), String(Gl_ic2_CodeResultNew.tapwater_int                    ).c_str(), String(GL_ic2_Code[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 5]          .Item).c_str(), String(Gl_ic2_CodeResultNew.sensortest                      ).c_str(), String(GL_ic2_Code[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 6]          .Item).c_str(), String(Gl_ic2_CodeResultNew.zone_heating                    ).c_str(), String(GL_ic2_Code[ 6].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 7]          .Item).c_str(), String(Gl_ic2_CodeResultNew.standby                         ).c_str(), String(GL_ic2_Code[ 7].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 8]          .Item).c_str(), String(Gl_ic2_CodeResultNew.postrun_boiler                  ).c_str(), String(GL_ic2_Code[ 8].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 9]          .Item).c_str(), String(Gl_ic2_CodeResultNew.service                         ).c_str(), String(GL_ic2_Code[ 9].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[10]          .Item).c_str(), String(Gl_ic2_CodeResultNew.tapwater                        ).c_str(), String(GL_ic2_Code[10].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[11]          .Item).c_str(), String(Gl_ic2_CodeResultNew.postrun_ch                      ).c_str(), String(GL_ic2_Code[11].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[12]          .Item).c_str(), String(Gl_ic2_CodeResultNew.boiler_int                      ).c_str(), String(GL_ic2_Code[12].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[13]          .Item).c_str(), String(Gl_ic2_CodeResultNew.buffer                          ).c_str(), String(GL_ic2_Code[13].Units).c_str());
    break;
        
    case ic2_Parameters:
      loc_Topic2 = "ic2_Parameters"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 0]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.heater_on                 ).c_str(), String(Gl_ic2_Parameters[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 1]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.comfort_mode              ).c_str(), String(Gl_ic2_Parameters[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 2]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.ch_set_max                ).c_str(), String(Gl_ic2_Parameters[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 3]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.dhw_set                   ).c_str(), String(Gl_ic2_Parameters[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 4]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.eco_days                  ).c_str(), String(Gl_ic2_Parameters[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 5]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.comfort_set               ).c_str(), String(Gl_ic2_Parameters[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 6]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.dhw_at_night              ).c_str(), String(Gl_ic2_Parameters[ 6].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 7]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.ch_at_night               ).c_str(), String(Gl_ic2_Parameters[ 7].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 8]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_1                   ).c_str(), String(Gl_ic2_Parameters[ 8].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[ 9]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_2                   ).c_str(), String(Gl_ic2_Parameters[ 9].Units).c_str());  
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[10]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_3                   ).c_str(), String(Gl_ic2_Parameters[10].Units).c_str());  
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[11]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_4                   ).c_str(), String(Gl_ic2_Parameters[11].Units).c_str());  
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[12]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_5                   ).c_str(), String(Gl_ic2_Parameters[12].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[13]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_6                   ).c_str(), String(Gl_ic2_Parameters[13].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[14]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_7                   ).c_str(), String(Gl_ic2_Parameters[14].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[15]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_8                   ).c_str(), String(Gl_ic2_Parameters[15].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[16]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_9                   ).c_str(), String(Gl_ic2_Parameters[16].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[17]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_A                   ).c_str(), String(Gl_ic2_Parameters[17].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[18]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_b                   ).c_str(), String(Gl_ic2_Parameters[18].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[19]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_c                   ).c_str(), String(Gl_ic2_Parameters[19].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[20]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_d                   ).c_str(), String(Gl_ic2_Parameters[20].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[21]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_E                   ).c_str(), String(Gl_ic2_Parameters[21].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[22]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_Edot                ).c_str(), String(Gl_ic2_Parameters[22].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[23]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_F                   ).c_str(), String(Gl_ic2_Parameters[23].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[24]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_H                   ).c_str(), String(Gl_ic2_Parameters[24].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[25]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_n                   ).c_str(), String(Gl_ic2_Parameters[25].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[26]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_o                   ).c_str(), String(Gl_ic2_Parameters[26].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[27]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_P                   ).c_str(), String(Gl_ic2_Parameters[27].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[28]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_r                   ).c_str(), String(Gl_ic2_Parameters[28].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters[29]    .Item).c_str(), String(Gl_ic2_ParametersResultNew.param_Fdot                ).c_str(), String(Gl_ic2_Parameters[29].Units).c_str());
    break;

    case ic2_OperatingHours:
      loc_Topic2 = "ic2_OperatingHours"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 0].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.line_power_connected  ).c_str(), String(Gl_ic2_OperatingHours[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 1].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.line_power_disconnect ).c_str(), String(Gl_ic2_OperatingHours[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 2].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.ch_function           ).c_str(), String(Gl_ic2_OperatingHours[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 3].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.dhw_function          ).c_str(), String(Gl_ic2_OperatingHours[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 4].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.burnerstarts          ).c_str(), String(Gl_ic2_OperatingHours[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 5].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.ignition_failed       ).c_str(), String(Gl_ic2_OperatingHours[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 6].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.flame_lost            ).c_str(), String(Gl_ic2_OperatingHours[ 6].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 7].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.reset                 ).c_str(), String(Gl_ic2_OperatingHours[ 7].Units).c_str());
    break;

    case ic2_BurnerVersion:
      loc_Topic2 = "ic2_BurnerVersion"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion[ 0] .Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.h_version              ).c_str(), String(Gl_ic2_BurnerVersion[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion[ 1] .Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.s_version              ).c_str(), String(Gl_ic2_BurnerVersion[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion[ 2] .Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.checksum1              ).c_str(), String(Gl_ic2_BurnerVersion[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion[ 3] .Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.checksum2              ).c_str(), String(Gl_ic2_BurnerVersion[ 3].Units).c_str());
    break;

    case ic2_Burner:
    loc_Topic2 = "ic2_Burner"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 0]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.interrupt_time                ).c_str(), String(Gl_ic2_Burner[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 1]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.interrupt_load                ).c_str(), String(Gl_ic2_Burner[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 2]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.main_load                     ).c_str(), String(Gl_ic2_Burner[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 3]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.net_frequency                 ).c_str(), String(Gl_ic2_Burner[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 4]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.voltage_reference             ).c_str(), String(Gl_ic2_Burner[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 5]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.checksum1                     ).c_str(), String(Gl_ic2_Burner[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner[ 6]        .Item).c_str(), String(Gl_ic2_BurnerResultNew.checksum2                     ).c_str(), String(Gl_ic2_Burner[ 6].Units).c_str());
    break;

    case ic2_Faults:
      loc_Topic2 = "ic2_Faults"; // 10 characters
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 0]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault00                       ).c_str(), String(Gl_ic2_Faults[ 0].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 1]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault01                       ).c_str(), String(Gl_ic2_Faults[ 1].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 2]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault02                       ).c_str(), String(Gl_ic2_Faults[ 2].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 3]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault03                       ).c_str(), String(Gl_ic2_Faults[ 3].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 4]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault04                       ).c_str(), String(Gl_ic2_Faults[ 4].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 5]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault05                       ).c_str(), String(Gl_ic2_Faults[ 5].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 6]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault06                       ).c_str(), String(Gl_ic2_Faults[ 6].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 7]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault07                       ).c_str(), String(Gl_ic2_Faults[ 7].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 8]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault08                       ).c_str(), String(Gl_ic2_Faults[ 8].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[ 9]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault09                       ).c_str(), String(Gl_ic2_Faults[ 9].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[10]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault10                       ).c_str(), String(Gl_ic2_Faults[10].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[11]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault11                       ).c_str(), String(Gl_ic2_Faults[11].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[12]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault12                       ).c_str(), String(Gl_ic2_Faults[12].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[13]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault13                       ).c_str(), String(Gl_ic2_Faults[13].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[14]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault14                       ).c_str(), String(Gl_ic2_Faults[14].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[15]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault15                       ).c_str(), String(Gl_ic2_Faults[15].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[16]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault16                       ).c_str(), String(Gl_ic2_Faults[16].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[17]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault17                       ).c_str(), String(Gl_ic2_Faults[17].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[18]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault18                       ).c_str(), String(Gl_ic2_Faults[18].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[19]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault19                       ).c_str(), String(Gl_ic2_Faults[19].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[20]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault20                       ).c_str(), String(Gl_ic2_Faults[20].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[21]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault21                       ).c_str(), String(Gl_ic2_Faults[21].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[22]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault22                       ).c_str(), String(Gl_ic2_Faults[22].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[23]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault23                       ).c_str(), String(Gl_ic2_Faults[23].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[24]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault24                       ).c_str(), String(Gl_ic2_Faults[24].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[25]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault25                       ).c_str(), String(Gl_ic2_Faults[25].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[26]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault26                       ).c_str(), String(Gl_ic2_Faults[26].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[27]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault27                       ).c_str(), String(Gl_ic2_Faults[27].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[28]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault28                       ).c_str(), String(Gl_ic2_Faults[28].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[29]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault29                       ).c_str(), String(Gl_ic2_Faults[29].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[30]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault30                       ).c_str(), String(Gl_ic2_Faults[30].Units).c_str());
      PrintTelegram (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults[31]        .Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault31                       ).c_str(), String(Gl_ic2_Faults[31].Units).c_str());
    break;
    }     
}

void PublishDataFromSerialPort (){
    if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  String loc_Topic2;
    switch(Gl_Request){
      case ic2_Status:
        loc_Topic2 = "ic2_Status"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 0]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t1                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 1]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t2                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 2]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t3                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 3]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t4                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 4]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t5                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 5]    .Item).c_str(), String(Gl_ic2_StatusResultNew.t6                                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 6]    .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_pressure                        ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 7]    .Item).c_str(), String(Gl_ic2_StatusResultNew.temp_set                           ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 8]    .Item).c_str(), String(Gl_ic2_StatusResultNew.fanspeed_set                       ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[ 9]    .Item).c_str(), String(Gl_ic2_StatusResultNew.fan_speed                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[10]    .Item).c_str(), String(Gl_ic2_StatusResultNew.fan_pwm                            ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[11]    .Item).c_str(), String(Gl_ic2_StatusResultNew.io_curr                            ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[12]    .Item).c_str(), String(Gl_ic2_StatusResultNew.display_code                       ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[13]    .Item).c_str(), String(Gl_ic2_StatusResultNew.gp_switch                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[14]    .Item).c_str(), String(Gl_ic2_StatusResultNew.tap_switch                         ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[15]    .Item).c_str(), String(Gl_ic2_StatusResultNew.roomtherm                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[16]    .Item).c_str(), String(Gl_ic2_StatusResultNew.pump                               ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[17]    .Item).c_str(), String(Gl_ic2_StatusResultNew.dwk                                ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[18]    .Item).c_str(), String(Gl_ic2_StatusResultNew.alarm_status                       ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[19]    .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_cascade_relay                   ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[20]    .Item).c_str(), String(Gl_ic2_StatusResultNew.opentherm                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[21]    .Item).c_str(), String(Gl_ic2_StatusResultNew.gasvalve                           ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[22]    .Item).c_str(), String(Gl_ic2_StatusResultNew.spark                              ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[23]    .Item).c_str(), String(Gl_ic2_StatusResultNew.io_signal                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[24]    .Item).c_str(), String(Gl_ic2_StatusResultNew.ch_ot_disabled                     ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[25]    .Item).c_str(), String(Gl_ic2_StatusResultNew.low_water_pressure                 ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[26]    .Item).c_str(), String(Gl_ic2_StatusResultNew.pressure_sensor                    ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[27]    .Item).c_str(), String(Gl_ic2_StatusResultNew.burner_block                       ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Status[28]    .Item).c_str(), String(Gl_ic2_StatusResultNew.grad_flag                          ).c_str() );
        
        loc_Topic2 = "ic2_Code"; // 10 characters
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 0]      .Item).c_str() , String(Gl_ic2_CodeResultNew.opentherm                           ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 1]      .Item).c_str() , String(Gl_ic2_CodeResultNew.boiler_ext                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 2]      .Item).c_str() , String(Gl_ic2_CodeResultNew.frost                               ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 3]      .Item).c_str() , String(Gl_ic2_CodeResultNew.central_heating_rf                  ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 4]      .Item).c_str() , String(Gl_ic2_CodeResultNew.tapwater_int                        ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 5]      .Item).c_str() , String(Gl_ic2_CodeResultNew.sensortest                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 6]      .Item).c_str() , String(Gl_ic2_CodeResultNew.zone_heating                        ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 7]      .Item).c_str() , String(Gl_ic2_CodeResultNew.standby                             ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 8]      .Item).c_str() , String(Gl_ic2_CodeResultNew.postrun_boiler                      ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[ 9]      .Item).c_str() , String(Gl_ic2_CodeResultNew.service                             ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[10]      .Item).c_str() , String(Gl_ic2_CodeResultNew.tapwater                            ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[11]      .Item).c_str() , String(Gl_ic2_CodeResultNew.postrun_ch                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[12]      .Item).c_str() , String(Gl_ic2_CodeResultNew.boiler_int                          ).c_str() );
        SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(GL_ic2_Code[13]      .Item).c_str() , String(Gl_ic2_CodeResultNew.buffer                              ).c_str() );
        break;
        
    case ic2_Parameters:
      loc_Topic2 = "ic2_Parameters"; // 10 characters
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 0].Item).c_str() , String(Gl_ic2_ParametersResultNew.heater_on                 ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 1].Item).c_str() , String(Gl_ic2_ParametersResultNew.comfort_mode              ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 2].Item).c_str() , String(Gl_ic2_ParametersResultNew.ch_set_max                ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 3].Item).c_str() , String(Gl_ic2_ParametersResultNew.dhw_set                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 4].Item).c_str() , String(Gl_ic2_ParametersResultNew.eco_days                  ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 5].Item).c_str() , String(Gl_ic2_ParametersResultNew.comfort_set               ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 6].Item).c_str() , String(Gl_ic2_ParametersResultNew.dhw_at_night              ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 7].Item).c_str() , String(Gl_ic2_ParametersResultNew.ch_at_night               ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 8].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_1                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [ 9].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_2                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [10].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_3                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [11].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_4                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [12].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_5                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [13].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_6                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [14].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_7                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [15].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_8                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [16].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_9                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [17].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_A                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [18].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_b                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [19].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_c                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [20].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_d                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [21].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_E                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [22].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_Edot                ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [23].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_F                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [24].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_H                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [25].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_n                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [26].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_o                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [27].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_P                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [28].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_r                   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Parameters    [29].Item).c_str() , String(Gl_ic2_ParametersResultNew.param_Fdot                ).c_str() );
    break;

    case ic2_OperatingHours:
      loc_Topic2 = "ic2_OperatingHours"; // 10 characters
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 0].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.line_power_connected   ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 1].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.line_power_disconnect  ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 2].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.ch_function            ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 3].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.dhw_function           ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 4].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.burnerstarts           ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 5].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.ignition_failed        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 6].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.flame_lost             ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_OperatingHours[ 7].Item).c_str(), String(Gl_ic2_OperatingHoursResultNew.reset                  ).c_str() );
    break;

    case ic2_BurnerVersion:
      loc_Topic2 = "ic2_BurnerVersion"; // 10 characters
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion [ 0].Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.h_version               ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion [ 1].Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.s_version               ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion [ 2].Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.checksum1               ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_BurnerVersion [ 3].Item).c_str(), String(Gl_ic2_BurnerVersionResultNew.checksum2               ).c_str() );
    break;

    case ic2_Burner:
      loc_Topic2 = "ic2_Burner"; // 10 characters
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 0].Item).c_str(), String(Gl_ic2_BurnerResultNew.interrupt_time                 ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 1].Item).c_str(), String(Gl_ic2_BurnerResultNew.interrupt_load                 ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 2].Item).c_str(), String(Gl_ic2_BurnerResultNew.main_load                      ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 3].Item).c_str(), String(Gl_ic2_BurnerResultNew.net_frequency                  ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 4].Item).c_str(), String(Gl_ic2_BurnerResultNew.voltage_reference              ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 5].Item).c_str(), String(Gl_ic2_BurnerResultNew.checksum1                      ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Burner        [ 6].Item).c_str(), String(Gl_ic2_BurnerResultNew.checksum2                      ).c_str() );
    break;

    case ic2_Faults:
      loc_Topic2 = "ic2_Faults"; // 10 characters
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 0].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault00                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 1].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault01                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 2].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault02                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 3].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault03                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 4].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault04                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 5].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault05                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 6].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault06                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 7].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault07                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 8].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault08                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [ 9].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault09                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [10].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault10                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [11].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault11                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [12].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault12                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [13].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault13                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [14].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault14                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [15].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault15                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [16].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault16                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [17].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault17                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [18].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault18                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [19].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault19                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [20].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault20                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [21].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault21                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [22].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault22                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [23].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault23                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [24].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault24                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [25].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault25                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [26].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault26                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [27].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault27                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [28].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault28                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [29].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault29                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [30].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault30                        ).c_str() );
      SendMQTT (Gl_Topic1, loc_Topic2.c_str(), String(Gl_ic2_Faults        [31].Item).c_str(), String(Gl_ic2_FaultsResultNew.Fault31                        ).c_str() );
    break;   
  }     
}

void PublishSerialStats (){
  //
  if ( IntergasStateMachine.executeOnce ) {
    WriteState();
  }
  //
  SendMQTT(Gl_Topic1, "TelegramErrors"          , String(Gl_ParameterList[Gl_Request].TelegramTypeName).c_str(), String(Gl_ParameterList[Gl_Request].TelegramErrors           ).c_str() );  
  SendMQTT(Gl_Topic1, "TelegramMaxRetrieveTime" , String(Gl_ParameterList[Gl_Request].TelegramTypeName).c_str(), String(Gl_ParameterList[Gl_Request].TelegramMaxRetrieveTime  ).c_str() );  
  SendMQTT(Gl_Topic1, "TelegramRetrieveTime"    , String(Gl_ParameterList[Gl_Request].TelegramTypeName).c_str(), String(Gl_ParameterList[Gl_Request].TelegramRetrieveTime     ).c_str() );  
  SendMQTT(Gl_Topic1, "InterGasGWUptime"        , "Time"                                                       , String(millis()                                              ).c_str() );  
}


void setup_wifi() {
  //
  Serial.println(                           );
  //
  // Write some intresting information for the enduers
  Serial.print    ("WiFi - "                );
  Serial.print    ("MAC   address     : "   );
  Serial.println  (WiFi.macAddress()        );
  //
  Serial.print    ("WiFi - "                );
  Serial.print    ("Setting hostname  : "   );
  Serial.println  (String(hostname).c_str() );
  //  
  // Set some intresting information (hostname, SSID and password for the WIFI network
  WiFi.hostname   (String(hostname).c_str()   );
  WiFi.mode       (WIFI_STA                   );
  WiFi.begin      (ssid, password             );
  //
  Serial.print  ("WiFi - "                    );
  Serial.print  ("Conneting to WiFi : "       );
  //
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
    Serial.println(" Connected"               );
    //
  // Write some intresting information for the enduers
  Serial.print("WiFi - "                    );
  Serial.print("IP address        : "     );
  Serial.println(WiFi.localIP()             );
  //
  WiFi.setAutoReconnect (true);
  WiFi.persistent       (true);
}
//
// ###########################################################################
// ## Define StateMachine transistions                                      ##
// ###########################################################################
//
bool Always(){
    return true;
}
//
bool NewTelegramRequest(){
  if ( Gl_Request != Nothing ){
    return true;
  }
  return false;
}
//
bool NoNewTelegramRequest(){
  if ( Gl_Request == Nothing ){
    return true;
  }
  return false;
}
//
bool ValidTelegram(){
  if ( Gl_ParameterList[Gl_Request].TelegramValid == true ){
    return true;
  }
  return false;
}
//
bool NoValidTelegram(){
  if ( Gl_ParameterList[Gl_Request].TelegramValid == false ){
    return true;
  }
  return false;
}
//
bool WriteResultsToSerialAndMQTT (){
  if ( (Gl_ParameterList[Gl_Request].WriteResultsToSerial == true) and (Gl_ParameterList[Gl_Request].WriteResultsToMQTT == true) ){
    return true;
  }
  return false;
}
//
bool NotWriteResultsToSerialandMQTT (){
  if ( (Gl_ParameterList[Gl_Request].WriteResultsToSerial == false) and (Gl_ParameterList[Gl_Request].WriteResultsToMQTT == false) ){
    return true;
  }
  return false;
}
//
bool WriteResultsToSerialAndNotToMQTT (){
  if ( (Gl_ParameterList[Gl_Request].WriteResultsToSerial == true) and (Gl_ParameterList[Gl_Request].WriteResultsToMQTT == false) ){
    return true;
  }
  return false;
}

bool WriteResultsToMQTTAndNotToSerial (){
  if ( (Gl_ParameterList[Gl_Request].WriteResultsToSerial == false) and (Gl_ParameterList[Gl_Request].WriteResultsToMQTT == true) ){
    return true;
  }
  return false;
}
// ###########################################################################
// ## Define teh states of the StateMachine                                 ##
// ###########################################################################
//
State* IGSM_SetBurnerRequest            = IntergasStateMachine.addState(&SetBurnerRequest          ); // State 00
State* IGSM_SendRequestToSerialPort1    = IntergasStateMachine.addState(&SendRequestToSerialPort   ); // State 01
State* IGSM_GetDataFromSerialPort1      = IntergasStateMachine.addState(&GetDataFromSerialPort     ); // State 02
State* IGSM_ParseDataFromSerialPort1    = IntergasStateMachine.addState(&ParseDataFromSerialPort   ); // State 03
State* IGSM_SetBurnerSettings           = IntergasStateMachine.addState(&SetBurnerSettings         ); // State 04
State* IGSM_EmptySerialBuffer1          = IntergasStateMachine.addState(&EmptySerialBuffer         ); // State 05
State* IGSM_EmptySerialBuffer2          = IntergasStateMachine.addState(&EmptySerialBuffer         ); // State 06
State* IGSM_DetermineRequest            = IntergasStateMachine.addState(&DetermineRequest          ); // State 07
State* IGSM_SendRequestToSerialPort2    = IntergasStateMachine.addState(&SendRequestToSerialPort   ); // State 08
State* IGSM_GetDataFromSerialPort2      = IntergasStateMachine.addState(&GetDataFromSerialPort     ); // State 09
State* IGSM_ParseDataFromSerialPort2    = IntergasStateMachine.addState(&ParseDataFromSerialPort   ); // State 10
State* IGSM_PublishDataFromSerialPort1  = IntergasStateMachine.addState(&PublishDataFromSerialPort ); // State 11
State* IGSM_DisplayDataFromSerialPort1  = IntergasStateMachine.addState(&DisplayDataFromSerialPort ); // State 12
State* IGSM_PublishDataFromSerialPort2  = IntergasStateMachine.addState(&PublishDataFromSerialPort ); // State 13
State* IGSM_DisplayDataFromSerialPort2  = IntergasStateMachine.addState(&DisplayDataFromSerialPort ); // State 14
State* IGSM_PublishSerialStats          = IntergasStateMachine.addState(&PublishSerialStats        ); // State 15


void setup() {
  // ###########################################################################
  // ## Set serial ports                                                      ##
  // ###########################################################################
  //
  // Set pimodes for the led
  pinMode(ledPin, OUTPUT);
  //
  // Set pimodes for the serial port
  pinMode(Intergas_RXport,  INPUT);     //  RX D7 input
  pinMode(Intergas_TXport, OUTPUT);     //  TX D6 output
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
    //Do nothing, just wait
    }
  // ###########################################################################
  // ## Setup wifi                                                            ##
  // ###########################################################################
  //
  setup_wifi();
  //
  // ###########################################################################
  // ## Setup MQTT                                                            ##
  // ###########################################################################
  //
  client.setServer(mqtt_server, mqtt_port);
  MQTTconnect();
  //
  // ###########################################################################
  // ## Setup StateMachine Transitions                                        ##
  // ###########################################################################
  //
  IGSM_SetBurnerRequest           ->  addTransition(&Always                           , IGSM_EmptySerialBuffer1		        );  
  //
  IGSM_EmptySerialBuffer1         ->  addTransition(&Always                           , IGSM_SendRequestToSerialPort1	    );   
  //
  IGSM_SendRequestToSerialPort1   ->  addTransition(&Always                           , IGSM_GetDataFromSerialPort1	      ); 
  //
  IGSM_GetDataFromSerialPort1     ->  addTransition(&NoValidTelegram                  , IGSM_EmptySerialBuffer1           );  
  IGSM_GetDataFromSerialPort1     ->  addTransition(&ValidTelegram                    , IGSM_ParseDataFromSerialPort1    	);  
  //
  IGSM_ParseDataFromSerialPort1   ->  addTransition(&Always                           , IGSM_SetBurnerSettings            ); 
  //
  IGSM_SetBurnerSettings          ->  addTransition(&Always                           , IGSM_DetermineRequest             );  
  //
  //
  IGSM_DetermineRequest           ->  addTransition(&NewTelegramRequest               , IGSM_EmptySerialBuffer2		        );   
  //
  IGSM_EmptySerialBuffer2         ->  addTransition(&Always                           , IGSM_SendRequestToSerialPort2	    );   
  //
  IGSM_SendRequestToSerialPort2   ->  addTransition(&Always                           , IGSM_GetDataFromSerialPort2	      ); 
  //
  IGSM_GetDataFromSerialPort2     ->  addTransition(&ValidTelegram                    , IGSM_ParseDataFromSerialPort2   	);  
  IGSM_GetDataFromSerialPort2     ->  addTransition(&NoValidTelegram                  , IGSM_PublishSerialStats    	      );  
  //
  IGSM_ParseDataFromSerialPort2   ->  addTransition(&WriteResultsToSerialAndMQTT      , IGSM_DisplayDataFromSerialPort1  	); 
  IGSM_ParseDataFromSerialPort2   ->  addTransition(&WriteResultsToMQTTAndNotToSerial , IGSM_PublishDataFromSerialPort2 	); 
  IGSM_ParseDataFromSerialPort2   ->  addTransition(&WriteResultsToSerialAndNotToMQTT , IGSM_DisplayDataFromSerialPort2   ); 
  IGSM_ParseDataFromSerialPort2   ->  addTransition(&NotWriteResultsToSerialandMQTT   , IGSM_PublishSerialStats           ); 
  //
  IGSM_DisplayDataFromSerialPort1 ->  addTransition(&Always                           , IGSM_PublishDataFromSerialPort1  	); 
  IGSM_PublishDataFromSerialPort1 ->  addTransition(&Always        	                  , IGSM_PublishSerialStats 	        );  
  //
  IGSM_DisplayDataFromSerialPort2 ->  addTransition(&Always                           , IGSM_PublishSerialStats  	        ); 
  //
  IGSM_PublishDataFromSerialPort2 ->  addTransition(&Always        	                  , IGSM_PublishSerialStats           );  
  //
  IGSM_PublishSerialStats         ->  addTransition(&Always                           , IGSM_DetermineRequest		          ); 
  //
  // Set first initial state
  IntergasStateMachine.transitionTo(IGSM_SetBurnerRequest);

}

void loop() {
  // Start the StateMachine
  IntergasStateMachine.run();
}

