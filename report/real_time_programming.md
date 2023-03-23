# 2. Real-Time Programming Techniques

A series of real-time programming techniques was considered to ensure the system is both performant, robust, and reliable. Specifically, we conduct an analysis of thread safety, memory usage, and code- scalability in this report while also assessing the system design and architecture.     

## 2.1 Design and Architecture

Our overall design architecture consists of a series of tasks running in parallel by a task scheduler and ISR interrupts which are called and initiated in the setup function. Figure 1 demonstrates a diagram of dependencies and links between the different subroutines and variables.  


<p align="center">
<img src="/Images/Dependencies Diagram Emb CW2.png" width="400" alt="Figure 1: Links between the different subroutines and variables">
  
  <p align="center">
    <em>
  Figure 1: Timing Analysis for Tasks in The System
    </em>
 </p>
</p>


## 2.2 Performance, Reliability and Robustness

### 2.2.1 Thread Safety

Thread safety was an imperative consideration for our code development primarily targeted at preventing race conditions and data corruption within the system. The following methods were used to ensure system-wide thread-safety.  

* Mutex semaphores provide thread safety by acting as a lock that can be acquired and released by threads. Effectively, this ensures the other threads which utilize the shared resource such as knobcount will be blocked prior to accessing the variables and prevent race conditions occurring. 
* Atomic store has the functionality of ensuring that the store operation happens in a single step and guaranteeing that concurrent read or write operations would occur before or after the atomic store but not during.   
* Queues provide atomic operations in terms of their enqueue and dequeue operations, ensuring thread safety due to this as they perform the operations in a single uninterruptible step. 

Table 1 below shows how thread safety was implemented on a system level.  

| Variable Name | Variable Type        | Read By               | Write By                                | Access Method       |
| ------------- | --------------------| --------------------------| ----------------------------------|---------------------|
| msgInQ        | QueueHandle_t        | decodeTask           | CAN_RX_ISR                              | Queue               |
| msgOutQ       | QueueHandle_t        |                      |                                         | Queue               |
| leftpos       | bool                 | decodeTask           | displayUpdateTask                       | Regular access      |
| wavearr       | 2D array of uint32_t | sampleISR            |                                         | Regular access      |
| knobCount     | 1D array of uint32_t | CAN_TX_Task, scanKeysTask, decodeTask, sampleISR, displayUpdateTask          | scanKeysTask               | Mutex, Semaphore    |
| prevKnob      | 1D array of uint32_t | scanKeysTask         | scanKeysTask                            | Mutex, Semaphore    |
| keyVal        | uint32_t             | sampleISR            | scanKeysTask                            | Atomic Store        |
| mastervol     | uint32_t             | scanKeysTask         | decodeTask                              | Regular access      |
| masteroct     | uint32_t             | scanKeysTask         | decodeTask                              | Regular access      |
| masterwave    | uint32_t             | scanKeysTask         | decodeTask                              | Regular access      |
| stepSizes     | 1D array of uint32_t | sampleISR            |                                         | Regular access      |
| recvkey     | uint32_t | sampleISR            | decodeTask                                        | Atomic Store     |


  <p align="center">
    <em>
  Table 1: Summarising system-level thread safety implementation 
    </em>
 </p>
 
In addition to drawing dependency diagrams (which shows the potential for interrupts etc), we utilized the platformIO inspection tools to model the dependencies and potential for code defects. From the defect analysis it appeared that there were no defects visible and present in our code and hence it is demonstrated that      

### 2.2.2 Interrupts 

A series of interrupts were needed for the coursework to ensure that the tasks could be run synchronously in parallel and in a time performant manner. The interrupts included doing a series of sampleISR interrupts as needed to examine the overall state of the system and monitor changes that occur. Specifically, we have a total of 4 possible interrupt routines including: 
- SampleISR interrupt for playing sounds, this allows voltages to be changed accordingly to enable sounds corresponding to the sawtooth and sine wave output waves to be relevantly generated. ScanKeyTask() will use the SampleISR interrupt as shown in dependency diagram. 
- CAN_REGISTER_RX ISR, this has the effect of receiving messages and writing these message values into a queue in the ISR which receives data from other tasks. Meanwhile, the decodeTask() will use the ISR to process the relevant messages in queue. 
- CAN_TX_ISR() : This is necessary for tracking number of output slots usable by CAN_TX and is done through the use of semaphores.  
 
### 2.2.3 Memory Usage Optimization

To reduce CPU memory utilization, our group considered the storage and access of variables within the system. Firstly, all our variables should be initialized with suitable data types that shouldn't take more storage than needed.  

For our scankeyTask() we used an integer variable to store the scanned key values instead of an array. Implementing an array for this task would require a memory allocation equivalent to a total of 12 integer value as necessary to store all keys. However, using a 1 hot encoding process would be spatially inefficient in our system and as such we opted to store the scankey() state in an integer instead.  

Additionally, our sine wave table is optimised in terms of spatial requirements. By considering the shape of a sinusoidal wave, we can determine how the absolute values are periodic at a period of Pi. As a result, when considering these waves, we only store 90 sine values in the array to represent the wave tending from 0-pi instead of storing all wave values spanning 0-2pi in period.   

