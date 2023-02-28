#include <Arduino.h>
#include <U8g2lib.h>
#include <string>
#include <iostream>
#include <map>
#include <bitset>
#include <STM32FreeRTOS.h>
using namespace std;
//Step Size
volatile uint32_t currentStepSize;
SemaphoreHandle_t keyValMutex;//accesses the mutex object. 


//Key Array making it a global variable 
volatile uint32_t keyVal;

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


std::map<std::string, std::string> keyvalmap2 = {{"111111111111","_"},
                                                {"011111111111","C"},
                                                {"101111111111","C#"},
                                                {"110111111111","D"},
                                                {"111011111111","D#"},
                                                {"111101111111","E"},
                                                {"111110111111","F"},
                                                {"111111011111","F#"},
                                                {"111111101111","G"},
                                                {"111111110111","G#"},
                                                {"111111111011","A"},
                                                {"111111111101","A#"},
                                                {"111111111110","B"}};


const uint32_t stepSizes [] = {0,51076922,54112683,57330004,60740599,64274185,68178701,72231589,76528508,81077269,85899346,91006452,96418111};
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

//convert integers to a binary string format for display
const char* intToBinaryString(int value) {
    static char buffer[13];  // Allocate a buffer to hold the binary string
    std::bitset<12> bits((value & 0xFFF));  // Mask the last 12 bits and convert to binary format
    std::string binaryString = bits.to_string();  // Convert the binary format to a std::string
    binaryString.copy(buffer, sizeof(buffer));  // Copy the std::string to the buffer
    buffer[sizeof(buffer) - 1] = '\0';  // Add a null terminator to the end of the buffer
    return buffer;  // Return the binary string as a const char*
}

//reads columns and returns in byte format. 
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
  delayMicroseconds(10);

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
  delayMicroseconds(10);

  //GET G#-B
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
int testvar = 0;

void sampleISR(){
  static uint32_t phaseAcc = 0;
  phaseAcc = phaseAcc + currentStepSize;
  uint32_t Vout = (phaseAcc>>24) - 128;
  testvar = phaseAcc;
  analogWrite(OUTR_PIN, Vout + 128);
}

void scanKeysTask(void * pvParameters) {
  //gets the time elapsed. 
  // static uint32_t next = millis();
  // static uint32_t count = 0;
  // cout<<"scanning keys"<<endl;

  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  // cout<<"SCANNING"<<endl;
  Serial.print("here");
  
  //run infinite loop
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    xSemaphoreTake(keyValMutex,portMAX_DELAY);

    keyVal = readCols();
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
    //atomic storing
    __atomic_store_n(&currentStepSize, currentStepSize, __ATOMIC_RELAXED);

    xSemaphoreGive(keyValMutex);



  }


}
void displayUpdateTask(void * pvParameters) {

  //run infinite loop
  const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1){

    vTaskDelayUntil( &xLastWakeTime, xFrequency );

    //get the semaphore value. 
    // xSemaphoreTake(keyValMutex,portMAX_DELAY);


    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    xSemaphoreTake(keyValMutex,portMAX_DELAY);//provide take to access a variable similar to threading lock.

    u8g2.drawStr(2,20,intToBinaryString(keyVal));
    u8g2.drawStr(2,10,to_string(keyvalmap[intToBinaryString(keyVal)]).c_str());
    //currentStepSize = keystepmap[readCols()];
    u8g2.setCursor(2,20);
    u8g2.sendBuffer();          // transfer internal memory to the display
    
    //Toggle LED
    Serial.print(to_string(keyvalmap[intToBinaryString(keyVal)]).c_str());
    digitalToggle(LED_BUILTIN);
    xSemaphoreGive(keyValMutex);//breaking the threading lock



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
  Serial.print("Hello World!");


  //Interrupt for sampleISR()
  TIM_TypeDef *Instance = TIM1;

  //start a hardware timer to do polling. 
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
  &scanKeysHandle );  /* Pointer to store the task handle */

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
  displayUpdateTask,     /* Function that implements the task */
  "displayUpdate",       /* Text name for the task */
  256,                   /* Stack size in words, not bytes */
  NULL,                  /* Parameter passed into the task */
  1,                     /* Task priority */
  &displayUpdateHandle);

  keyValMutex = xSemaphoreCreateMutex();

  vTaskStartScheduler();



}

void loop() {
  // put your main code here, to run repeatedly:

    //reads the key value

}
