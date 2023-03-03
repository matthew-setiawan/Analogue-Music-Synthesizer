#include <cmath>
#include <stdexcept>

// Define the delay time and feedback gain for the reverb effect
const int delayTime = 22000; // 1 second delay at 22 kHz sample rate
const float feedbackGain = 0.5;



class ReverbEffect
{
public:
    ReverbEffect()
    {
        // Allocate memory for the delay buffer
        delayBuffer = new float[delayTime];
        for (int i = 0; i < delayTime; ++i)
        {
            delayBuffer[i] = 0;
        }
        delayIndex = 0;
    }

    ~ReverbEffect()
    {
        delete[] delayBuffer;
    }

    void processBlock(float* buffer, int numSamples)
    {
        if (buffer == nullptr)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }
        if (numSamples <= 0)
        {
            throw std::invalid_argument("Number of samples must be positive");
        }

        // Apply the reverb effect to the audio buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = buffer[sample];

            // Add the input sample to the delay buffer
            float delayedInput = delayBuffer[delayIndex];
            delayBuffer[delayIndex] = input + feedbackGain * delayedInput;

            // Calculate the output sample as a mix of the current and delayed input
            float output = input + delayedInput;

            buffer[sample] = output;

            // Increment the delay buffer index and wrap around if necessary
            ++delayIndex;
            if (delayIndex >= delayTime)
            {
                delayIndex = 0;
            }
        }
    }

private:
    float* delayBuffer;
    int delayIndex;
};

// Example usage
void processAudioBlock(float* buffer, int numSamples)
{
    if (buffer == nullptr)
    {
        throw std::invalid_argument("Buffer cannot be null");
    }
    if (numSamples <= 0)
    {
        throw std::invalid_argument("Number of samples must be positive");
    }

    ReverbEffect reverb;
    reverb.processBlock(buffer, numSamples);
}
