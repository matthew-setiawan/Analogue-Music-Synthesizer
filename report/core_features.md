# 1. Core Features

The development of core features formed an essential basis as necessary to ensure the functionality and usability of the provided code. Notably, the following core features were included in our code.

## 1.1 CAN Communication

The objective of this project was to establish reliable communication between one sender and multiple receivers using CAN. To achieve this, I developed a communication algorithm that enables the sender to send messages to all receivers simultaneously. The project was successful in enabling efficient and error-free communication between the sender and multiple receivers. The array below was used to send and recieve messages:

'''uint8_t msgOut[8] = {0};'''

## 1.1.1 Sending Notes

Each value in the 

## 1.1.2 Sending Volume, Octave and Wave Type

This is to facilitate communication and deployment across separate keyboards. CAN Messages are sent which contain information comprising of octave and volume details which are synchronized in state. 

## 1.2 Note Detection and Playing

Note detection is achieved by continuously scanning a key matrix to find out which keys are pressed. This allows for a smaller number of microcontroller pins to be able to detect all keys. 

## 1.3 Knob Integration

There are four knobs on the device. Two of these were set up to allow for the adjustment of the volume and the octave of the sound played by the device. The knob integration is achieved through the same key matrix.

## 1.4 Display User Interface

An OLED display is readily available on the device. This is set up to display information on the volume, octave and the current keys pressed on the device.

## 1.5 Sound Generation

Based on the keys pressed and the parameters set by the knobs, the desired note is played. Internally, is is represented by a sawtooth function with a sample rate of 22 kHz.
