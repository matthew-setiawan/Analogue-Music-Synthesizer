#include <Arduino.h>
#include <U8g2lib.h>
#include <string>
#include <iostream>
#include <map>
#include <bitset>
#include <STM32FreeRTOS.h>
#include <cmath>
#include <ES_CAN.h>
using namespace std;

QueueHandle_t msgInQ;

//Step Size
volatile uint32_t currentStepSize;
volatile uint32_t knobCount3;
volatile uint32_t prevKnob3;
volatile uint32_t knobCount2;
volatile uint32_t prevKnob2;

//Key String/Array
volatile uint32_t keyVal;

//Constants
const long pibitshift16 = 205887; // define the value of pi
const long incrbitshift16 = 3;
SemaphoreHandle_t keyArrayMutex;

//Time
int time = 0;

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
}

long testvar = 0;

void sampleISR(){
  static uint32_t phaseAcc = 0;
  phaseAcc = phaseAcc + (currentStepSize>>knobCount2);
  testvar = phaseAcc;
  uint32_t Vout = (phaseAcc>>24) - 128;
  analogWrite(OUTR_PIN, (Vout + 128)>>knobCount3);
  time = time + incrbitshift16;
}

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void scanKeysTask(void * pvParameters) {
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    keyVal = readCols();
    readKnobs();
    xSemaphoreGive(keyArrayMutex);
    if(readCols()==4095){
        currentStepSize = stepSizes[0];
    }
    else{
      int indexs[12];
      int playedcount = 0;
      for(int i = 0; i < 12; i++){
        if(intToBinaryString(readCols())[i]=='0'){
          currentStepSize = stepSizes[i+1];
          delayMicroseconds(20);
        }
      }
    }
    uint8_t TX_Message[8] = {0};
    TX_Message[0] = readCols();
    CAN_TX(0x123, TX_Message);
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
    u8g2.drawStr(2,10,intToBinaryString(keyVal));
    u8g2.drawStr(2,20,("Volume: "+to_string(5-knobCount3)).c_str());
    u8g2.drawStr(2,30,("Octave: "+to_string(knobCount2)).c_str());
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

    uint8_t RX_Message[8] = {0};
    uint32_t ID = 0x123;
    while (CAN_CheckRXLevel()){
      CAN_RX(ID, RX_Message);
    }
    cout << "Test:" << RX_Message[0] << endl;
  }
}

void setup() {
  // put your setup code here, to run once:

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
  CAN_Init(true);
  setCANFilter(0x123,0x7FF);
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_Start();
  msgInQ = xQueueCreate(36,8);

  //Interrupt for sampleISR()
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(44000, HERTZ_FORMAT);
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

  //Knobs
  keyArrayMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();
}

void loop() {
}