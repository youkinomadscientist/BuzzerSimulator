#include "buzzer_hal.h"

// 此预处理器确保该文件仅在定义了 PLATFORM_ESP32 时才被编译
#ifdef PLATFORM_ESP32

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 定义 LEDC（LED 控制器）常量
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // 10位分辨率 (1024级)
#define LEDC_DUTY_50_PERCENT    (512)             // 50% 的占空比，用于产生方波

static int g_buzzer_pin = -1; // 全局变量，存储蜂鸣器引脚

void buzzer_init(int pin) {
    g_buzzer_pin = pin;

    // 准备并应用 LEDC PWM 定时器配置
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = 5000,  // 设置一个默认的初始频率
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 准备并应用 LEDC PWM 通道配置
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = g_buzzer_pin,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // 初始占空比为0，即不发声
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void buzzer_play(int frequency, int duration_ms) {
    if (frequency > 0) {
        buzzer_start(frequency);
        vTaskDelay(duration_ms / portTICK_PERIOD_MS); // 使用FreeRTOS的延时
        buzzer_stop();
    } else {
        // 频率为 0 表示暂停
        vTaskDelay(duration_ms / portTICK_PERIOD_MS);
    }
}

void buzzer_start(int frequency) {
    if (frequency > 0) {
        // 设置 PWM 频率
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
        // 设置 50% 的占空比以产生声音
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_50_PERCENT);
        // 更新占空比以应用设置
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }
}

void buzzer_stop() {
    // 将占空比设置为 0 以停止声音
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void buzzer_play_melody(const Note* melody, int length) {
    for (int i = 0; i < length; ++i) {
        buzzer_play(melody[i].frequency, melody[i].duration_ms);
        // 在音符之间添加一个小延迟，使它们听起来更分明
        if (i < length - 1) {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}

#endif // PLATFORM_ESP32