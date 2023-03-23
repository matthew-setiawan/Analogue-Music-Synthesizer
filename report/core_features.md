# 1. Core Features

The development of core features formed an essential basis as necessary to ensure the functionality and usability of the provided code. Notably, the following core features were included in our code.

## 1.1 CAN Communication

The objective of this project was to establish reliable communication between one sender and multiple receivers using CAN. To achieve this, I developed a communication algorithm that enables the sender to send messages to all receivers simultaneously. The project was successful in enabling efficient and error-free communication between the sender and multiple receivers. The array below was used to send and recieve messages:

```uint8_t msgOut[8] = {0};```

## 1.1.1 Sending Notes

Each value in msgOut was limited to 8 bits, since there was 12 notes we needed to use two indexs to store note values. Since notes being played (keyVal) was stored as a 12-bit integer of maximum size 4095 we used the following 2 indexs to define the notes played:
```
msgOut[4] = keyVal/100;
msgOut[5] = keyVal%100;
```

## 1.1.2 Sending Volume, Octave and Wave Type

Sending volume, octave and wave types were more trivial as the values are much lower. We simply specified the following:
```
msgOut[1] = knobCount[2];//sending octave
msgOut[2] = knobCount[3];//sending volume
msgOut[3] = knobCount[0];//sending wavetype
```

## 1.2 Note Detection and Playing

### 1.2.1 Reading Keys

Prior to note detection we could read weather each key was being pressed by adjusting RA2, RA1 and RA0 and reading the pins accordingly. These values were used to create a 12-bit integer (keyVal) which stores data on all keys being pressed like the following:

```
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
```
### 1.2.2 Reading keyVal and playing sound

We used a simple loop to iterate through bits of keyVal and find out which notes are being played. For each key being pressed we play the specified wave with the corresponding frequency specified in our stepSizes array:

```
const uint32_t stepSizes [] = {0,51076922,54112683,57330004,60740599,64274185,68178701,72231589,76528508,81077269,85899346,91006452,96418111};
```

## 1.3 Knob Integration

There are four knobs on the device. Knob 1 is used to select the waveform, knob 2 to determine master/slave, knob 3 to adjust the octave, and knob 4 to adjust the volume. The knobs were obtained similarly to reading the keys in 1.2.1. However, we had to keep track of changes in the knob states to determine whether the knob was moving left, right, or idle.

## 1.4 Display User Interface

An OLED display is readily available on the device. This display identifies the keys being played, waveform type, master/slave configuration, volume and octave of each board.

## 1.5 Sound Generation

Based on the voltage obtained with process 1.2.2, we simply used the following code which sets the voltage with Vres (sum of waveforms of keys being played): 

```
analogWrite(OUTR_PIN, ((Vres+128))>>vol);
```
