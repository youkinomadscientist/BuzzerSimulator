#ifndef BUZZER_HAL_H
#define BUZZER_HAL_H

/**
 * @brief 定义一个音符的结构体，包含频率和持续时间。
 *
 * 频率为 0 代表休止符（静音）。
 */
struct Note {
    int frequency;      // 频率 (Hz)
    int duration_ms;    // 持续时间 (ms)
};

/**
 * @brief 初始化蜂鸣器硬件或模拟环境。
 *
 * 在 ESP32 上，此函数为指定的 GPIO 引脚配置 LEDC PWM 定时器和通道。
 * 在 PC 上，此函数打印一条初始化消息。
 *
 * @param pin 连接到蜂鸣器的 GPIO 引脚号（在 PC 上被忽略）。
 */
void buzzer_init(int pin);

/**
 * @brief 以指定的频率和持续时间播放声音（阻塞式调用）。
 *
 * 此函数将阻塞程序的执行，直到声音播放完毕。
 *
 * @param frequency 声音的频率（单位：赫兹 Hz）。使用 0 表示静音。
 * @param duration_ms 声音的持续时间（单位：毫秒 ms）。
 */
void buzzer_play(int frequency, int duration_ms);

/**
 * @brief 以指定的频率开始播放声音（非阻塞式调用）。
 *
 * 声音将持续播放，直到调用 buzzer_stop()。
 *
 * @param frequency 声音的频率（单位：赫兹 Hz）。必须大于 0。
 */
void buzzer_start(int frequency);

/**
 * @brief 停止当前由 buzzer_start() 播放的声音。
 */
void buzzer_stop();

/**
 * @brief 以阻塞方式播放一段音符序列（旋律）。
 *
 * @param melody 指向音符（Note）结构体数组的指针。
 * @param length 旋律数组中音符的数量。
 */
void buzzer_play_melody(const Note* melody, int length);

#endif // BUZZER_HAL_H