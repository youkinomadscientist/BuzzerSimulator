#ifndef BUZZER_HAL_H
#define BUZZER_HAL_H

#include <stdint.h>

/**
 * @brief 以指定的频率和可选的持续时间在引脚上生成方波。
 *
 * @param pin 将要输出信号的 GPIO 引脚号。
 * @param frequency PWM 信号的频率（单位：赫兹 Hz）。
 * @param duration 声音的持续时间（单位：毫秒 ms）。如果未提供或为 0，
 *                 则必须手动调用 noTone() 来结束输出。
 */
void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0);

/**
 * @brief 停止在指定引脚上由 tone() 函数产生的波形。
 *
 * @param pin 需要停止声音的 GPIO 引脚号。
 */
void noTone(uint8_t pin);

/**
 * @brief 设置用于生成音调的 LEDC 通道。
 *
 * @param channel 要使用的 LEDC 通道 (0-15)。
 */
void setToneChannel(uint8_t channel);

#endif // BUZZER_HAL_H