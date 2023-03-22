# Analogue-Music-Synthesizer - Embedded Systems Coursework 2


  # Report
  
  This report focuses on the development of an embedded software for a music synthesizer circuit.
  
  The report highlights the solutions leading to a robust real-time programming implementation. More specifically, it details the allocation of functions to tasks, the time characterisation of these tasks, how data is shared and synchronised between these and any deadlocks that may occur.

## Table of Contents

### [1. Core Features](report/core_features.md)

#### 1.1 CAN Communication

#### 1.2 Note Detection

#### 1.3 Display User Interface

#### 1.4 Knob Integration


  ### [2 Real-Time Programming](report/real_time_programming.md)
  
### 2. Real-Time Programming Techniques
   

#### 2.1 Design and Architecture

#### 2.2 Performance, Reliability and Robustness

##### 2.2.1 Thread Safety
 
##### 2.2.2 Interrupts 

##### 2.2.3 Minimize Memory Usage
  
### [3 Real-Time System Analysis](report/timing_analysis.md)
  

# 3. Real-Time System Analysis Report

To evaluate the reliability of our system, it was necessary to conduct data analysis and statistical tests to observe and analyse the real-time performance of the system. To this end, our group conducted a series of measurements to determine the time required to complete specific tasks and performed critical instant analysis to verify whether each task could be completed before the respective deadlines. To study the overall behaviour of the system, we collected statistics from the FreeRTOS() operating system to show the time the real-world performance of the operating system.

### 3.1 Methodology

To determine the timing requirements for tasks and interrupts, we computed the average execution time for each task across all 32 intervals.To determine the timing requirements for tasks and interrupts, we computed the average execution time for each task across all 32 intervals.This was then validated with the FreeRTOS stats buffer timer which appeared to show similar estimated time values when timed for. 
Figure 1 and 2 demonstrate the timing analysis of tasks in the system.


<p align="center">
<img src="/Images/figure-1.png" width="400" alt="Figure 1: Timing Analysis for Tasks in The System">
  
  <p align="center">
    <em>
  Figure 1: Timing Analysis for Tasks in The System
    </em>
 </p>
</p>



<p align="center">
<img src="/Images/figure-2.png" width="400" alt="Figure 2: Timing Analysis for Each ISR">
  
  <p align="center">
    <em>
Figure 2: Timing Analysis for Each ISR
    </em>
 </p>
</p>


The aggregated timing statistics indicate consistent pattern wherein the displayUpdate() tasks exhibit significantly longer execution times compared to the other tasks. This can be attributed to the task's requirement to draw the relevant output on the display screen, which significantly increases the time required for this task. Additionally, the scanKeysTask() task exhibits relatively longer execution times, which is justifiable due to the need to scan through the keys, knobs, and joysticks, making this an overall time-consuming task.

## 3.2 Critical Instant Analysis

To determine the critical instant analysis of each task we need to determine both the deadlines and interrupt times of tasks within the system. We can determine the overall execution time of each task by taking the sum of our measured time and the relevant interrupt for the task (in figure 1).

For instance, the scanKeysTask() only has a single interrupt and hence the time taken for this task can be determined as follows:


$$ T_{scanKeysTask()} =\ T_{scankeysmeasured()} +\ T_{sampleISR} =\ 70.7\ +\ 11.3\ =\ 82.0\ \mu s $$

### 3.2.1 Determining Task Deadlines

The deadlines for the displayUpdateTask() and scanKeysTask() are definable based on the port max frequency from STM32 and primarily use the vTaskDelayUntil() parameter. In our implementation, we extended the scheduling interval for displayUpdateTask() to 150ms for as it's acceptable to wait for a period of 0.15s for the task to occur and it's a task of lower priority.

### 3.2.2 Calculating Deadlines CAN Transmission



### 3.2.3 Timing Analysis

### 3.2.4 Scan Key Task Timing

## 3.3 CPU Memory and Utilization

### 3.3.1 Stack Size Utilization

### 3.3.2 CPU Utilization

## 3.4 Real World Timing Statistics
  

  ### [4 Advanced Features](report/advanced_features.md)
  
#### [4.1 Handshaking and auto detection for multiple boards](https://github.com/matthew-setiawan/Analogue-Music-Synthesizer/blob/main/report/advanced_features.md#handshaking-and-auto-detection)

#### 4.2 Automatic octave adjustments across multiple boards

#### 4.3 Advanced Waveforms

#### 4.4 Polyphony 

#### 4.5 Keyboard Autodetect 


  #### Project Guidance

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)
  
  TO BE REMOVED:
  
   ### Data and resource sharing
  
  ### Dependencies
