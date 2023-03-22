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

$$ \begin{tabular}{|l|l|l|l|l|}
\hline
\textbf{Variable Name} & \textbf{Variable Type} & \textbf{Read By} & \textbf{Write By} & \textbf{Access Method} \ \hline
msgInQ & QueueHandle_t & decodeTask & CAN_RX_ISR & Queue \ \hline
msgOutQ & QueueHandle_t & & & Queue \ \hline
leftpos & bool & decodeTask & displayUpdateTask & Regular access \ \hline
wavearr & 2D array of uint32_t & sampleISR & & Regular access \ \hline
knobCount & 1D array of uint32_t & CAN_TX_Task & scanKeysTask, decodeTask & Mutex, Semaphore \ \hline
prevKnob & 1D array of uint32_t & scanKeysTask & scanKeysTask & Mutex, Semaphore \ \hline
keyVal & uint32_t & sampleISR & scanKeysTask & Atomic Store \ \hline
mastervol & uint32_t & scanKeysTask & decodeTask & Regular access \ \hline
masteroct & uint32_t & scanKeysTask & decodeTask & Regular access \ \hline
masterwave & uint32_t & scanKeysTask & decodeTask & Regular access \ \hline
stepSizes & 1D array of uint32_t & sampleISR & & Regular access \ \hline
\end{tabular} $$

  <p align="center">
    <em>
  Table 1: Summarising system-level thread safety implementation 
    </em>
 </p>
