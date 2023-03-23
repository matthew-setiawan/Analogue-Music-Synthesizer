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

Note detection was achieved by scanning the key buttons 

## 1.3 Knob Integration

There are four knobs on the device. Two of these were set up to allow for the adjustment of the volume and the octave of the sound played by the device. The knob integration is achieved through the same key matrix.

## 1.4 Display User Interface

An OLED display is readily available on the device. This is set up to display information on the volume, octave and the current keys pressed on the device.

## 1.5 Sound Generation

Based on the keys pressed and the parameters set by the knobs, the desired note is played. Internally, is is represented by a sawtooth function with a sample rate of 22 kHz.
