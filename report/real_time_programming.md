# 2. Real-Time Programming Techniques

A series of real-time programming techniques was considered to ensure the system is both performant, robust, and reliable. Specifically, we conduct an analysis of thread safety, memory usage, and code- scalability in this report while also assessing the system design and architecture.     

## 2.1 Design and Architecture

Our overall design architecture consists of a series of tasks running in parallel by a task scheduler and ISR interrupts which are called and initiated in the setup function. Here is a diagram to indicate the dependencies in the system.  

<i>DEPENDENCIES DIAGRAM HERE </i>

## 2.2 Performance, Reliability and Robustness

### 2.2.1 Thread Safety

Thread safety was an imperative consideration for our code development primarily targeted at preventing race conditions and data corruption within the system. The following methods were used to ensure system-wide thread-safety.  

* Mutex semaphores provide thread safety by acting as a lock that can be acquired and released by threads. Effectively, this ensures the other threads which utilize the shared resource such as knobcount will be blocked prior to accessing the variables and prevent race conditions occurring. 
* Atomic store has the functionality of ensuring that the store operation happens in a single step and guaranteeing that concurrent read or write operations would occur before or after the atomic store but not during.   
* Queues provide atomic operations in terms of their enqueue and dequeue operations, ensuring thread safety due to this as they perform the operations in a single uninterruptible step. 

Table 1 below shows how thread safety was implemented on a system level.  

| Variable Name | Variable Type        | Read By               | Write By                                | Access Method       |
| ------------- | --------------------| ---------------------| ---------------------------------------|---------------------|
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


  <p align="center">
    <em>
  Table 1: Summarising system-level thread safety implementation 
    </em>
 </p>
 
 
### 2.2.2 Interrupts 

### 2.2.3 Minimize Memory Usage

To reduce CPU utilization, our group considered the storage and access of variables within the system.   

For our scankeyTask() we used an integer variable to store the scanned key values instead of an array. Implementing an array for this task would require a memory allocation equivalent to a total of 12 integer value as necessary to store all keys. However, using a 1 hot encoding process would be spatially inefficient in our system and as such we opted to store the scankey() state in an integer instead.  

Additionally, our sine wave table is optimised in terms of spatial requirements. By considering the shape of a sinusoidal wave, we can determine how the absolute values are periodic at a period of Pi. As a result, when considering these waves, we only store 90 sine values in the array to represent the wave tending from 0-pi instead of storing all wave values spanning 0-2pi in period.   
