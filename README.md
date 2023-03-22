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

#### 3.1 Methodology

#### 3.2 Critical Instant Analysis

##### 3.2.1 Determining Task Deadlines

##### 3.2.2 Calculating Deadlines CAN Transmission

##### 3.2.3 Timing Analysis

##### 3.2.4 Scan Key Task Timing

#### 3.3 CPU Memory and Utilization

##### 3.3.1 Stack Size Utilization

##### 3.3.2 CPU Utilization

#### 3.4 Real World Timing Statistics
  

### [4 Advanced Features](report/advanced_features.md)
  
#### 4.1 Handshaking and auto detection for multiple boards

#### 4.2 Automatic octave adjustments across multiple boards

#### 4.3 Advanced Waveforms

#### 4.4 Polyphony 

#### 4.5 Keyboard Autodetect 


  ### Appendix

  The following projects were used as starting points for this Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)
  
  TO BE REMOVED:
  
   ### Data and resource sharing
  
  ### Dependencies
