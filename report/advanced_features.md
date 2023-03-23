# 4. Advanced Features
    

## 4.1 Automatic octave adjustments across multiple boards

#### Determining East and West connections on board
  
  We can detect connections from the east and west of keyboard like a physical switch. The positions of the inputs in the matrix are:
  
  | HS Input | Row  | Column |
  | -------- | ----- | -------|
  | West     | 5    | 3      |
  | East     | 6    | 3      |

Using this method, we could automatically detect if the master was at the left or right of the board. This information was then used to adjust the octave accordingly (make octave lower when it's a left slave and higher otherwise).

## 4.2 Advanced Waveforms

In the music synthesizer, we used a look-up table (LUT) to create sine waves. A LUT is a pre-calculated table of values that can be accessed at runtime to avoid the need for complex mathematical calculations.

To create a sine wave LUT, we first calculated the values of sine function at regular intervals (e.g. every degree or every radian) and stored them in an array. We could then use this LUT to generate a sine wave by reading the appropriate value from the array at each sample point in time.

To create sawtooth and square waves using a LUT, we can use a similar technique. For sawtooth waves, we can create a ramp function LUT that increases linearly from 0 to 1 and then resets to 0, and then use this to generate a sawtooth wave by reading the appropriate value from the array at each sample point in time. For square waves, we can create a step function LUT that toggles between 0 and 1, and then use this to generate a square wave by reading the appropriate value from the array at each sample point in time.

In addition, we only store half of the sine wave LUT since it is an odd function, meaning that the values in the second half of the LUT are simply the negative values of the first half. This allows us to minimize the memory usage of the LUT without sacrificing the accuracy of the waveform.

## 4.3 Polyphony 

To implement polyphony in the music synthesizer, we added different waveforms together to create a richer and more complex sound. This was achieved using a technique called additive synthesis, where multiple waveforms were combined to create a single, more complex waveform. The implementation of polyphony allowed for the playing of multiple notes or voices simultaneously, with each voice assigned a different waveform, envelope, and other parameters, resulting in complex and expressive musical textures.

