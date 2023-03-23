#include <Arduino.h>
#include <U8g2lib.h>
#include <string>
#include <iostream>
#include <map>
#include <bitset>
#include <STM32FreeRTOS.h>
#include <cmath>
#include <ES_CAN.h>
#include <ctime>

using namespace std;

//Queues
QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

//Position
volatile bool leftpos = 0;

//Wavearr: 2D arrays containing all different type of waveforms
int wavearr[3][90] = {{0,4,8,13,17,22,26,30,35,39,43,47,52,56,60,63,67,71,75,78,82,85,88,92,95,98,100,103,106,108,110,113,115,116,118,120,121,123,124,125,126,126,127,127,127,128,127,127,127,126,126,125,124,123,121,120,118,116,115,113,110,108,106,103,100,98,95,92,88,85,82,78,75,71,67,63,60,56,52,47,43,39,35,30,26,22,17,13,8,4},
                      {128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128},
                      {-128, -124, -122, -118, -116, -112, -110, -108, -104, -102, -98, -96, -92, -90, -88, -84, -82, -78, -76, -72, -70, -68, -64, -62, -58, -56, -54, -50, -48, -44, -42, -38, -36, -34, -30, -28, -24, -22, -18, -16, -14, -10, -8, -4, -2, 0, 2, 4, 8, 10, 14, 16, 18, 22, 24, 28, 30, 34, 36, 38, 42, 44, 48, 50, 54, 56, 58, 62, 64, 68, 70, 72, 76, 78, 82, 84, 88, 90, 92, 96, 98, 102, 104, 108, 110, 112, 116, 118, 122, 124}};

//KnobValueTracking
volatile uint32_t knobCount[4] = {0,0,2,0};
volatile uint32_t prevKnob[4] = {0,0,0,0};

//Tracked Pressed Keys
volatile uint32_t keyVal = 4095;

SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t CAN_TX_Semaphore;

//Recieved Volume
uint32_t mastervol = 0;
uint32_t masteroct = 2;
uint32_t masterwave = 0;

const uint32_t stepSizes [] = {0,51076922,54112683,57330004,60740599,64274185,68178701,72231589,76528508,81077269,85899346,91006452,96418111};

//Constants
const uint32_t interval = 100; //Display update interval

int32_t recvkey = 4095;

//Pressed Key
std::map<int, std::string> intToKey = {{0, "C"}, {1, "C#"}, {2, "D"}, {3, "D#"}, {4, "E"}, {5, "F"}, {6, "F#"}, {7, "G"}, {8, "G#"}, {9, "A"}, {10, "A#"}, {11, "B"}};

//Pin definitions
//Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

//Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

//Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

//Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

//Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to generate random number
int random(int x, int y) {
    int random_number = rand() % (y - x + 1) + x; // Generate a random number between x and y inclusive
    return random_number;
}

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

const char* intToBinaryString(int value) {
    static char buffer[13];  // Allocate a buffer to hold the binary string
    std::bitset<12> bits((value & 0xFFF));  // Mask the last 12 bits and convert to binary format
    std::string binaryString = bits.to_string();  // Convert the binary format to a std::string
    binaryString.copy(buffer, sizeof(buffer));  // Copy the std::string to the buffer
    buffer[sizeof(buffer) - 1] = '\0';  // Add a null terminator to the end of the buffer
    return buffer;  // Return the binary string as a const char*
}

class readBoard{
  public:

  readBoard(){
  }

  void writePins(int ra0, int ra1, int ra2, int ren){
    digitalWrite(RA0_PIN, ra0);
    digitalWrite(RA1_PIN, ra1);
    digitalWrite(RA2_PIN, ra2);
    digitalWrite(REN_PIN, ren);
    delayMicroseconds(10);
  }

  uint32_t readKeys(){
    uint32_t retval = 0;
    //GET C-D#
    this->writePins(0,0,0,1);
    retval += digitalRead(C0_PIN)<<11;
    retval += digitalRead(C1_PIN)<<10;
    retval += digitalRead(C2_PIN)<<9;
    retval += digitalRead(C3_PIN)<<8;

    //GET E-G
    this->writePins(1,0,0,1);
    delayMicroseconds(3);
    retval += digitalRead(C0_PIN)<<7;
    retval += digitalRead(C1_PIN)<<6;
    retval += digitalRead(C2_PIN)<<5;
    retval += digitalRead(C3_PIN)<<4;

    //GET C-D#
    this->writePins(0,1,0,1);
    delayMicroseconds(3);
    retval += digitalRead(C0_PIN)<<3;
    retval += digitalRead(C1_PIN)<<2;
    retval += digitalRead(C2_PIN)<<1;
    retval += digitalRead(C3_PIN)<<0;

    return retval;
  }

  void readAllKnobs(int ra0, int ra1, int ra2, int ren, int k1, int k2, int bound1, int bound2){
    uint32_t knob1 = 0;
    uint32_t knob0 = 0;
    this->writePins(ra0,ra1,ra2,ren);
    delayMicroseconds(3);
    //GET KNOB0
    knob0 += digitalRead(C2_PIN)<<1;
    knob0 += digitalRead(C3_PIN)<<0;
    //GET KNOB1
    knob1 += digitalRead(C0_PIN)<<1;
    knob1 += digitalRead(C1_PIN)<<0;

    if(((knob0==1 && prevKnob[k1]==0)||(knob0==2 && prevKnob[k1]==3))&&knobCount[k1]<bound1){
      knobCount[k1] += 1;
    }
    else if(((knob0==0 && prevKnob[k1]==1)||(knob0==3 && prevKnob[k1]==2))&&knobCount[k1]>0){
      knobCount[k1] += -1;
    }
    prevKnob[k1] = knob0;

    if(((knob1==1 && prevKnob[k2]==0)||(knob1==2 && prevKnob[k2]==3))&&knobCount[k2]<bound2){
      knobCount[k2] += 1;
    }
    else if(((knob1==0 && prevKnob[k2]==1)||(knob1==3 && prevKnob[k2]==2))&&knobCount[k2]>0){
      knobCount[k2] += -1;
    }
    prevKnob[k2] = knob1;
  }

  void readKnobs01(){
    this->readAllKnobs(0,0,1,1,0,1,2,1);
    if(knobCount[1] != 1){
      knobCount[3] = mastervol;
      knobCount[2] = masteroct;
    }
  }
  void readKnobs(){
    this->readAllKnobs(1,1,0,1,2,3,8,8);
    if(knobCount[1] != 1){
      knobCount[0] = masterwave;
    }
  }

};

readBoard userinput;

int rightleftdetect(){
  userinput.writePins(1,0,1,1);
  delayMicroseconds(3);
  int west = digitalRead(C3_PIN);
  userinput.writePins(0,1,1,1);
  delayMicroseconds(3);
  int east = digitalRead(C3_PIN);
  if(west==1&&east==1){
    return 0;
  }
  else if(west==1&&east==0){
    leftpos = 1;
    return 1;//left
  }
  else if(west==0&&east==1){
    leftpos = 0;
    return 2;//right
  }
  else{
    return 3;
  }
}

void sampleISR(){
  static uint32_t time = 0;
  uint32_t Vout;
  uint32_t zeroCount = 0;
  uint32_t Vfinal = 0;//main keyboard V
  int tempkeyVal = keyVal;
  for(int i=11;i>=0;i--){
    if(tempkeyVal%2==0){
      //if(zeroCount<4){pressedkey = pressedkey+intToKey[i];}
      u_int32_t index = ((((stepSizes[i+1]<<2)>>knobCount[2])*time)>>22)%360;
      if(index>=180){
        Vfinal += -wavearr[knobCount[0]][(index-180)>>1];
      }
      else{
        Vfinal += wavearr[knobCount[0]][(index)>>1];
      }
      zeroCount += 1;
    }
    tempkeyVal = tempkeyVal/2;
  }

  uint32_t Vfinal_master = 0;//master(sender) keyboard sounds
  tempkeyVal = recvkey;
  for(int i=11;i>=0;i--){
    if(tempkeyVal%2==0){
      //if(zeroCount<4){pressedkey = pressedkey+intToKey[i];}
      u_int32_t index = ((((stepSizes[i+1]<<2)>>(knobCount[2]+(-2*leftpos+1)))*time)>>22)%360;
      if(index>=180){
        Vfinal_master += -wavearr[knobCount[0]][(index-180)>>1];
      }
      else{
        Vfinal_master += wavearr[knobCount[0]][(index)>>1];
      }
      zeroCount += 1;
    }
    tempkeyVal = tempkeyVal/2;
  }

  time += 1;

  if(zeroCount < 3){

  }
  else if(zeroCount < 9){
    zeroCount = 3;
  }
  else if(zeroCount < 12){
    zeroCount = 4;
  }
  uint32_t Vres = (Vfinal+Vfinal_master)>>zeroCount;
  analogWrite(OUTR_PIN, ((Vres+128))>>knobCount[3]);
}

void scanKeysTask(void * pvParameters) {
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    __atomic_store_n(&keyVal, userinput.readKeys(), __ATOMIC_RELAXED);
    userinput.readKnobs();
    userinput.readKnobs01();
    xSemaphoreGive(keyArrayMutex);
  }
}

void displayUpdateTask(void * pvParameters){
  const TickType_t xFrequency = 150/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //u8g2.drawStr(2,10,"Helllo World!");  // write something to the internal memory
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    //Master/Slave Screen Message
    string state;
    if(knobCount[1] == 1){
      state = "sender(master)";
    }
    else{
      state = "reciever";
    }

    //Wave Type Screen Message
    string wave;
    if(knobCount[0] == 0){
      wave = "sine";
    }
    else if(knobCount[0] == 1){
      wave = "square";
    }
    else{
      wave = "saw";
    }
    string pressedkey = "";
    int tempkeyVal1 = keyVal;
    int tempkeyVal2 = recvkey;
    for(int i=11;i>=0;i--){
      if(tempkeyVal1%2==0||tempkeyVal2%2==0){
        pressedkey = intToKey[i]+pressedkey;
      }
      tempkeyVal1 = tempkeyVal1/2;
      tempkeyVal2 = tempkeyVal2/2;
    }

    u8g2.drawStr(2,10,(state+" | "+wave).c_str());
    u8g2.drawStr(2,20,("Pressed Key: "+pressedkey).c_str());
    u8g2.drawStr(2,30,("Volume: "+to_string(8-knobCount[3])+"|"+"Octave: "+to_string(knobCount[2])).c_str());
    xSemaphoreGive(keyArrayMutex);
    u8g2.setCursor(2,20);
    u8g2.sendBuffer();
    rightleftdetect();

  }
}
void decodeTask(void *pvParameters)
{
  int8_t misses = 0; // Checks how many times middle CAN has been missed
  while (true)
  {
    uint8_t RX_Message_local[8];
    uint32_t ID_Local = 0;
    xQueueReceive(msgInQ, RX_Message_local, portMAX_DELAY);
    //testvar = RX_Message_local[1];
    if(knobCount[1]==0&&leftpos==1){//handling left slave
      mastervol = RX_Message_local[2];
      masteroct = RX_Message_local[1] + 1;
      masterwave = RX_Message_local[3];
      __atomic_store_n(&recvkey, RX_Message_local[4]*100+RX_Message_local[5], __ATOMIC_RELAXED);
    }
    else if(knobCount[1]==0&&leftpos==0){//handling right slave
      mastervol = RX_Message_local[2];
      if(RX_Message_local[1]==0){
        masteroct = 0;
      }
      else{
        masteroct = RX_Message_local[1] - 1;
      }
      masterwave = RX_Message_local[3];
      __atomic_store_n(&recvkey, RX_Message_local[4]*100+RX_Message_local[5], __ATOMIC_RELAXED);
    }
    else{
      __atomic_store_n(&recvkey, 4096, __ATOMIC_RELAXED);
    }
  }
}
void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CAN_TX_Task (void * pvParameters) {
	uint8_t msgOut[8] = {0};
	while (1) {
    if(knobCount[1] == 1){
      msgOut[1] = knobCount[2];//sending octave
      msgOut[2] = knobCount[3];//sending volume
      msgOut[3] = knobCount[0];//sending wavetype
      msgOut[4] = keyVal/100;
      msgOut[5] = keyVal%100;
      CAN_TX(0x123, msgOut);
    }
    
	}
}

void CAN_TX_ISR (void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void setup() {
  // put your setup code here, to run once:
  msgInQ = xQueueCreate(36,8);
  msgOutQ = xQueueCreate(36, 8);
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);

  //Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  //Initialise CAN Communication
  CAN_Init(false);
  setCANFilter(0x123,0x7ff);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_Start();

  //Interrupt for sampleISR()
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
  scanKeysTask,		/* Function that implements the task */
  "scanKeys",		/* Text name for the task */
  64,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  2,			/* Task priority */
  &scanKeysHandle);

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
  displayUpdateTask,     /* Function that implements the task */
  "displayUpdate",       /* Text name for the task */
  256,                   /* Stack size in words, not bytes */
  NULL,                  /* Parameter passed into the task */
  1,                     /* Task priority */
  &displayUpdateHandle);

  TaskHandle_t CAN_TX_Handle = NULL;
  xTaskCreate(CAN_TX_Task, /* Function that implements the task */
  "CAN_TX",    /* Text name for the task */
  32,          /* Stack size in words, not bytes */
  NULL,        /* Parameter passed into the task */
  1,           /* Task priority */
  &CAN_TX_Handle);

  TaskHandle_t decodeHandle = NULL;
  xTaskCreate(
  decodeTask,   /* Function that implements the task */
  "decodeTask", /* Text name for the task */
  32,           /* Stack size in words, not bytes */
  NULL,         /* Parameter passed into the task */
  1,            /* Task priority */
  &decodeHandle);

  //Knobs
  keyArrayMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();
}

void loop() {
}
