# Analogue-Music-Synthesizer - Embedded Systems Coursework 2

  # Running The Code
  Navigate to https://github.com/matthew-setiawan/Analogue-Music-Synthesizer/tree/main/src to find main.cpp used to upload software to the board. Timing analysis code can be run and demonstrated by navigating to the test_timing branch and running the relevant code there. To observe how we conducted analysis with the FreeRTOS() stats buffer please navigate to the timing_report value. 
  
  # Using the Board
  
  ## Configuring a Board
  - Boards are set to slaves/recievers by default which are unable to edit their volume, wave or octave
  - Twist 2nd knob to the left to make this a master/sender board then you can edit volume, wave and octaves using knobs 1, 3 and 4 respectively.

  ## Connecting an additional Board
  - Warning: before connecting boards with each other make sure that they are all set as slaves or one set as master. If two or more boards are set as master accidently use yellow reset button to reset boards.
  - Upon connection as slaves, set the middle board to master. You will notice the octaves and volumes of left and/or right boards changing respective to the master keyboard, you will also notice the keys being played on the master will also be sent and played by the slaves.
  
  ## Video
  
  <iframe width="715" height="402" src="https://www.youtube.com/embed/MiLZXhW4XqU" title="Joe Rogan on How Weed Affects Disciplined People | ft Dr. Andrew Huberman" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>
 

  # Report
  
  This report focuses on the development of an embedded software for a music synthesizer circuit.
  
  The report highlights the solutions leading to a robust real-time programming implementation. More specifically, it details the allocation of functions to tasks, the time characterisation of these tasks, how data is shared and synchronised between these and any deadlocks that may occur. Please see and click the relevant sections in the below table of contents to learn more about our task and implementation.

## Table of Contents

### [1. Core Features](report/core_features.md)

#### 1.1 CAN Communication

##### 1.1.1 Sending Notes

##### 1.1.2 Sending Volume, Octave and Wave Type

#### 1.2 Note Detection

#### 1.2.1 Reading Keys

#### 1.2.2 Reading keyVal and playing sound

#### 1.3 Display User Interface

#### 1.4 Knob Detection

#### 1.5 Sound Generation



  ### [2 Real-Time Programming](report/real_time_programming.md)

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
  
#### 4.1 Automatic octave adjustments across multiple boards

#### 4.2 Advanced Waveforms

#### 4.3 Polyphony 
