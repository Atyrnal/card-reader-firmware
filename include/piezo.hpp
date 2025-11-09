/*
  Copyright (c) 2025 Antony Rinaldi
*/

#include <Arduino.h>

class Piezo {
    public:
        void playTone(float freq, int durationMs);
        void setTone(float freq);
        void stopTone();
        Piezo();
        void begin(uint8_t pin, uint8_t channel);
    private:
        uint8_t ledcChannel;
        uint8_t pin;
        float tone;
};