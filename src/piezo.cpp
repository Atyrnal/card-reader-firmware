/*
  Copyright (c) 2025 Antony Rinaldi
*/


#include "piezo.hpp"

void Piezo::playTone(float freq, int durationMs) {
    setTone(freq);
    delay(durationMs);
    stopTone();
}

void Piezo::begin(uint8_t pzpin, uint8_t channel) {
    pin = pzpin;
    pinMode(pin, OUTPUT);
    ledcChannel = channel;
    ledcSetup(ledcChannel, 1000, 10); // 1 kHz to start
    ledcAttachPin(pin, ledcChannel);
}

void Piezo::setTone(float freq) {
    if (freq != tone) {
        tone = freq;
        ledcWriteTone(ledcChannel, (uint32_t) freq);
        ledcWrite(ledcChannel, 512);// 50% duty (1023/2)
    }
}

void Piezo::stopTone() {
    tone = -1;
    ledcWriteTone(ledcChannel, 0);
}

Piezo::Piezo() {
    tone = -1;
}