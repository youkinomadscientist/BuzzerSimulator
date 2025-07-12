#ifndef _ESP32_HAL_LEDC_H_
#define _ESP32_HAL_LEDC_H_

#define SOC_LEDC_SUPPORTED 1

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
// 模拟日志宏
#define log_d(format, ...) printf("[SIM_D] " format "\n", ##__VA_ARGS__)
#define log_e(format, ...) printf("[SIM_E] " format "\n", ##__VA_ARGS__)
#define log_v(format, ...) printf("[SIM_V] " format "\n", ##__VA_ARGS__)

// --- 模拟 ledc_types.h ---
typedef enum {
    LEDC_AUTO_CLK = 0,
} ledc_clk_cfg_t;

typedef enum {
  NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, NOTE_Fs, NOTE_G, NOTE_Gs, NOTE_A, NOTE_Bb, NOTE_B, NOTE_MAX
} note_t;


// --- 模拟 esp32-hal-ledc.h 函数声明 ---
#ifdef __cplusplus
extern "C" {
#endif

bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution);
bool ledcAttachChannel(uint8_t pin, uint32_t freq, uint8_t resolution, uint8_t channel);
bool ledcWrite(uint8_t pin, uint32_t duty);
bool ledcWriteChannel(uint8_t channel, uint32_t duty);
uint32_t ledcWriteTone(uint8_t pin, uint32_t freq);
uint32_t ledcWriteNote(uint8_t pin, note_t note, uint8_t octave);
uint32_t ledcRead(uint8_t pin);
uint32_t ledcReadFreq(uint8_t pin);
bool ledcDetach(uint8_t pin);
uint32_t ledcChangeFrequency(uint8_t pin, uint32_t freq, uint8_t resolution);

// 提供空实现或默认返回值
inline bool ledcSetClockSource(ledc_clk_cfg_t source) { (void)source; return true; }
inline ledc_clk_cfg_t ledcGetClockSource(void) { return LEDC_AUTO_CLK; }
inline bool ledcOutputInvert(uint8_t pin, bool out_invert) { (void)pin; (void)out_invert; return true; }
inline bool ledcFade(uint8_t pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms) { (void)pin; (void)start_duty; (void)target_duty; (void)max_fade_time_ms; return false; }
inline bool ledcFadeWithInterrupt(uint8_t pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms, void (*userFunc)(void)) { (void)pin; (void)start_duty; (void)target_duty; (void)max_fade_time_ms; (void)userFunc; return false; }
inline bool ledcFadeWithInterruptArg(uint8_t pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms, void (*userFunc)(void *), void *arg) { (void)pin; (void)start_duty; (void)target_duty; (void)max_fade_time_ms; (void)userFunc; (void)arg; return false; }

#ifdef __cplusplus
}
#endif

// 模拟 Arduino.h，因为 buzzer_esp32.cpp 包含了它
#ifndef ARDUINO_H
#define ARDUINO_H
// 在此模拟版本中不需要任何内容
#endif


#endif /* _ESP32_HAL_LEDC_H_ */