#include "esp32_tone_api.h"
#include "esp32-hal-ledc.h"
#include <thread>
#include <chrono>
#include <stdio.h> // For printf used in log_d

// 这是一个简化的、仅用于PC模拟的 `tone` API 实现。
// 它提供了与ESP32相同的API，但内部直接调用我们模拟的 `ledc` 函数，
// 而不使用FreeRTOS的任务和队列。

// 辅助函数，用于在PC上实现延时
static void simple_delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration) {
    // 附加引脚并设置频率。我们假设分辨率总是10位。
    ledcAttach(pin, frequency, 10);
    ledcWriteTone(pin, frequency);

    if (duration > 0) {
        simple_delay(duration);
        noTone(pin); // 播放完成后调用 noTone
    }
}

void noTone(uint8_t pin) {
    ledcWriteTone(pin, 0); // 停止声音
    ledcDetach(pin);       // 释放引脚资源
}

void setToneChannel(uint8_t channel) {
    // 在这个简化的模拟中，通道是自动管理的。
    // 这个函数可以保留为空，以保持API兼容性。
    (void)channel;
    log_d("setToneChannel(%d) called, but is a no-op in this simplified simulator.", channel);
}