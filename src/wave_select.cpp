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


QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;

uint32_t masterstate = 1;

//sin array
// u_int32_t sinarr[90] = {0, 4, 8, 13, 17, 22, 26, 30, 35, 39, 43, 47, 52, 56, 60, 63, 67, 71, 75, 78, 82, 85, 88, 92, 95, 98, 100, 103, 106, 108, 110, 113, 115, 116, 118, 120, 121, 123, 124, 125, 126, 126, 127, 127, 127, 128, 127, 127, 127, 126, 126, 125, 124, 123, 121, 120, 118, 116, 115, 113, 110, 108, 106, 103, 100, 98, 95, 92, 88, 85, 82, 78, 75, 71, 67, 63, 60, 56, 52, 47, 43, 39, 35, 30, 26, 22, 17, 13, 8, 4};

// TABLE_SIZE for the wavetable 
#define TABLE_SIZE 90 // the size of the wave table array

// wavetable is an array that holds lookup table
u_int32_t waveTable[TABLE_SIZE] = {0, 4, 8, 13, 17, 22, 26, 30, 35, 39, 43, 47, 52, 56, 60, 63, 67, 71, 75, 78, 82, 85, 88, 92, 95, 98, 100, 103, 106, 108, 110, 113, 115, 116, 118, 120, 121, 123, 124, 125, 126, 126, 127, 127, 127, 128, 127, 127, 127, 126, 126, 125, 124, 123, 121, 120, 118, 116, 115, 113, 110, 108, 106, 103, 100, 98, 95, 92, 88, 85, 82, 78, 75, 71, 67, 63, 60, 56, 52, 47, 43, 39, 35, 30, 26, 22, 17, 13, 8, 4};

//Step Size
volatile uint32_t currentStepSize;
volatile uint32_t knobCount3;
volatile uint32_t prevKnob3;
volatile uint32_t knobCount2 = 2;
volatile uint32_t prevKnob2;

volatile uint32_t knobCount1 = 0;
volatile uint32_t prevKnob1;
volatile uint32_t knobCount0;
volatile uint32_t prevKnob0;

volatile int32_t jst_knobCount0 = 0;
volatile int32_t jst_knobCount1 = 0;
volatile int32_t js_state = 0;
volatile uint32_t prevKnobj1;
volatile uint32_t prevKnobj0;

//Key String/Array
volatile uint32_t keyVal = 4095;

//Constants
const long pibitshift16 = 205887; // define the value of pi
const long incrbitshift16 = 3;

SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t CAN_TX_Semaphore;

//Recieved Volume
uint32_t mastervol = 0;
uint32_t masteroct = 2;

//Key/Val Mapping
std::map<std::string, std::uint32_t> keyvalmap = {{"111111111111",0},
                                                {"011111111111",1},
                                                {"101111111111",2},
                                                {"110111111111",3},
                                                {"111011111111",4},
                                                {"111101111111",5},
                                                {"111110111111",6},
                                                {"111111011111",7},
                                                {"111111101111",8},
                                                {"111111110111",9},
                                                {"111111111011",10},
                                                {"111111111101",11},
                                                {"111111111110",12}};

const uint32_t stepSizes [] = {0,51076922,54112683,57330004,60740599,64274185,68178701,72231589,76528508,81077269,85899346,91006452,96418111};
const uint32_t rawstepSizes [] = {0,262,277,294,311,329,349,369,392,415,440,466,494};

//Constants
const uint32_t interval = 100; //Display update interval

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

uint32_t readCols(){
  uint32_t retval = 0;

  //GET C-D#
  digitalWrite(RA0_PIN, LOW);
  digitalWrite(RA1_PIN, LOW);
  digitalWrite(RA2_PIN, LOW);
  digitalWrite(REN_PIN, HIGH);

  delayMicroseconds(3);

  retval += 2048*digitalRead(C0_PIN);
  retval += 1024*digitalRead(C1_PIN);
  retval += 512*digitalRead(C2_PIN);
  retval += 256*digitalRead(C3_PIN);

  //GET E-G
  digitalWrite(RA0_PIN, HIGH);
  digitalWrite(RA1_PIN, LOW);
  digitalWrite(RA2_PIN, LOW);
  digitalWrite(REN_PIN, HIGH);

  delayMicroseconds(3);

  retval += 128*digitalRead(C0_PIN);
  retval += 64*digitalRead(C1_PIN);
  retval += 32*digitalRead(C2_PIN);
  retval += 16*digitalRead(C3_PIN);

  //GET C-D#
  digitalWrite(RA0_PIN, LOW);
  digitalWrite(RA1_PIN, HIGH);
  digitalWrite(RA2_PIN, LOW);
  digitalWrite(REN_PIN, HIGH);

  delayMicroseconds(3);

  retval += 8*digitalRead(C0_PIN);
  retval += 4*digitalRead(C1_PIN);
  retval += 2*digitalRead(C2_PIN);
  retval += 1*digitalRead(C3_PIN);

  return retval;
}

void readKnobs(){
  uint32_t knob3 = 0;
  uint32_t knob2 = 0;
  digitalWrite(RA0_PIN, HIGH);
  digitalWrite(RA1_PIN, HIGH);
  digitalWrite(RA2_PIN, LOW);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(3);
  //GET KNOB2 
  knob2 += 2*digitalRead(C2_PIN);
  knob2 += 1*digitalRead(C3_PIN);
  //GET KNOB3
  knob3 += 2*digitalRead(C0_PIN);
  knob3 += 1*digitalRead(C1_PIN);

  if(((knob3==1 && prevKnob3==0)||(knob3==2 && prevKnob3==3))&&knobCount3<5){
    knobCount3 += 1;
  }
  else if(((knob3==0 && prevKnob3==1)||(knob3==3 && prevKnob3==2))&&knobCount3>0){
    knobCount3 += -1;
  }
  prevKnob3 = knob3;

  if(((knob2==1 && prevKnob2==0)||(knob2==2 && prevKnob2==3))&&knobCount2<5){
    knobCount2 += 1;
  }
  else if(((knob2==0 && prevKnob2==1)||(knob2==3 && prevKnob2==2))&&knobCount2>0){
    knobCount2 += -1;
  }
  prevKnob2 = knob2;

  if(knobCount1 != 2){
    knobCount3 = mastervol;
    knobCount2 = masteroct;
  }
}

long testvar = 0;

void rjoystickknob(){
  uint32_t jst_knob1 = 0;
  uint32_t jst_knob0 = 0;
  uint32_t test_add = 0;
  delayMicroseconds(1);
  digitalWrite(RA0_PIN, HIGH);
  digitalWrite(RA1_PIN, LOW);
  digitalWrite(RA2_PIN, HIGH);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(3);
  //GET KNOB0  jst_knob0 += 2*digitalRead(C3_PIN);
  jst_knob0 += 1*digitalRead(JOYX_PIN);
  delayMicroseconds(1);
  // jst_knob0 -= 1*digitalRead(JOYY_PIN);  //GET KNOB1  jst_knob1 += 2*digitalRead(C1_PIN);
  jst_knob1 += 1*digitalRead(JOYY_PIN);
  if(((jst_knob0==1 && prevKnobj0==0)||(jst_knob0==2 && prevKnobj0==3))&&jst_knobCount0<6){
    jst_knobCount0 += 1;
    js_state += 1;
  }
  prevKnobj0 = jst_knob0;
  if(((jst_knob1==1 && prevKnobj1==0)||(jst_knob1==2 && prevKnobj1==3))&&prevKnobj1<6){
    jst_knobCount1 -= 1;
    js_state -= 1;
  }
  prevKnobj1 = jst_knob1;
}

void readKnobs01(){
  uint32_t knob1 = 0;
  uint32_t knob0 = 0;
  digitalWrite(RA0_PIN, LOW);
  digitalWrite(RA1_PIN, LOW);
  digitalWrite(RA2_PIN, HIGH);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(3);
  //GET KNOB0
  knob0 += 2*digitalRead(C2_PIN);
  knob0 += 1*digitalRead(C3_PIN);
  //GET KNOB1
  knob1 += 2*digitalRead(C0_PIN);
  knob1 += 1*digitalRead(C1_PIN);

  if(((knob0==1 && prevKnob0==0)||(knob0==2 && prevKnob0==3))&&knobCount0<3){
    knobCount0 += 1;
  }
  else if(((knob0==0 && prevKnob0==1)||(knob0==3 && prevKnob0==2))&&knobCount0>-3){
    knobCount0 += -1;
  }
  prevKnob0 = knob0;

  if(((knob1==1 && prevKnob1==0)||(knob1==2 && prevKnob1==3))&&knobCount1<3){
    knobCount1 += 1;
  }
  else if(((knob1==0 && prevKnob1==1)||(knob1==3 && prevKnob1==2))&&knobCount1>0){
    knobCount1 += -1;
  }
  prevKnob1 = knob1;
}


// Uses knobCount0 to select waveform from a wavetable
void generateWaveTable(){
  // 0 = SAWTOOTH
  // 1 = SINE
  // 2 = TRIANGLE
  // 3 = SQUARE
  // 4 = SPECIAL MATHEMATICAL WAVEFORM
  switch(knobCount0){
    case 0: // SAWTOOTH
        testvar = 0;
        for(size_t i = 0; i < TABLE_SIZE; i++){
        waveTable[i] = i;
        }
        break;
    case 1: // SINE
        testvar = 1;
        for(size_t i = 0; i < TABLE_SIZE; i++){
            waveTable[i] = sin(2*PI*i/TABLE_SIZE)*127 + 128;
        }
        break;
    case 2: // TRIANGLE
        testvar = 2;
        for(size_t i = 0; i < TABLE_SIZE; i++){
            if(i < TABLE_SIZE/2){ // left half of triangle
            waveTable[i] = i*2;
            }
            else{                 // right half of triangle
            waveTable[i] = (TABLE_SIZE - i)*2;
            }
        }
        break;
    case 3: // SQUARE
        testvar = 3;
        for(size_t i = 0; i < TABLE_SIZE; i++){
            if(i < TABLE_SIZE/2){
            waveTable[i] = 255;
            }
            else{
            waveTable[i] = 0;
            }
        }
        break;
    case 4: // SPECIAL MATHEMATICAL WAVEFORM - sum of sines (can change later to a fourier series sum of sines/cosines)
        testvar = 4;
        for(size_t i = 0; i < TABLE_SIZE; i++){
            waveTable[i] = (sin(2*PI*i/TABLE_SIZE)*127 + 128) + (sin(4*PI*i/TABLE_SIZE)*127 + 128);
        }
        break;
    case 5: // SPECIAL MATHEMATICAL WAVEFORM - sum of sines (can change later to a fourier series sum of sines/cosines)
        testvar = 5;
        for(size_t i = 0; i < TABLE_SIZE; i++){
            waveTable[i] = (sin(2*PI*i/TABLE_SIZE)*127 + 128) + (sin(4*PI*i/TABLE_SIZE)*127 + 128);
        }
        break;
  }
}


void sampleISR(){
  //SAWTOOTH
  /*
  static uint32_t phaseAcc = 0;
  phaseAcc = phaseAcc + (currentStepSize>>knobCount2);
  uint32_t Vout = (phaseAcc>>24) - 128;
  testvar = Vout;
  analogWrite(OUTR_PIN, (Vout + 128)>>knobCount3);
  */
  generateWaveTable();
  static uint32_t clocktick = 0;
  uint32_t Vout;
  uint32_t zeroCount = 0;
  uint32_t Vfinal = 0;
  int tempkeyVal = keyVal;
  for(int i=11;i>=0;i--){
    if(tempkeyVal%2==0){
      u_int32_t index = ((((stepSizes[i+1]<<2)>>knobCount2)*clocktick)>>22)%360;
      if(index>=180){
        Vfinal += -waveTable[(index-180)>>1];
      }
      else{
        Vfinal += waveTable[(index)>>1];
      }
      zeroCount += 1;
    }
    tempkeyVal = tempkeyVal/2;
  }
//   testvar = zeroCount;
  if(zeroCount < 3){

  }
  else if(zeroCount < 9){
    zeroCount = 3;
  }
  else if(zeroCount < 12){
    zeroCount = 4;
  }
  uint32_t Vres = Vfinal>>zeroCount-1;
  analogWrite(OUTR_PIN, ((Vres+128)>>1)>>knobCount3);
  clocktick +=1;
}

void scanKeysTask(void * pvParameters) {
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyVal = readCols();
    readKnobs();
    readKnobs01();
    xSemaphoreGive(keyArrayMutex);
    if(readCols()==4095){
        currentStepSize = stepSizes[0];
    }
    else{
      int indexs[12];
      int playedcount = 0;
      for(int i = 0; i < 12; i++){
        if(intToBinaryString(readCols())[i]=='0'){
          indexs[playedcount] = stepSizes[i+1];
          playedcount += 1;
        }
      }
      currentStepSize = indexs[random(0,playedcount - 1)];
    }
    uint8_t TX_Message[8] = {0};
    //TX_Message[1] = 10;
    //CAN_TX(0x123, TX_Message);
  }
}

void displayUpdateTask(void * pvParameters){
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //u8g2.drawStr(2,10,"Helllo World!");  // write something to the internal memory
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    string state;
    if(knobCount1 == 2){
      state = "master";
    }
    else if(knobCount1 == 1){
      state = "slave left";
    }
    else{
      state = "slave right";
    }
    u8g2.drawStr(2,10,state.c_str());
    u8g2.drawStr(2,20,("Volume: "+to_string(5-knobCount3)).c_str());
    u8g2.drawStr(2,30,("Octave: "+to_string(knobCount2)).c_str());
    u8g2.drawStr(5,10,("WAVEFORM: "+to_string(knobCount0)).c_str());
    xSemaphoreGive(keyArrayMutex);
    //currentStepSize = keystepmap[readCols()];
    u8g2.setCursor(2,20);
    //u8g2.print(count++);
    u8g2.sendBuffer();          // transfer internal memory to the display
    //Toggle LED
    cout << intToBinaryString(readCols()) << endl;
    digitalToggle(LED_BUILTIN);
    cout << testvar << endl;
    cout << knobCount3 << endl;
    cout << knobCount2 << endl;
    cout << knobCount1 << endl;
    cout << knobCount0 << endl;
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
    if(knobCount1==1){//handling left slave
      mastervol = RX_Message_local[2];
      masteroct = RX_Message_local[1] + 1;
    }
    else if(knobCount1==0){//handling right slave
      mastervol = RX_Message_local[2];
      if(RX_Message_local[1]==0){
        masteroct = 0;
      }
      else{
        masteroct = RX_Message_local[1] - 1;
      }
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
	  //xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		//xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
    if(knobCount1 == 1){
      msgOut[1] = knobCount2;//sending octave
      msgOut[2] = knobCount3;//sending volume
      msgOut[3] = 1;
      if(masterstate==1){
        CAN_TX(0x123, msgOut);
      }
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

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");

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
  1,			/* Task priority */
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