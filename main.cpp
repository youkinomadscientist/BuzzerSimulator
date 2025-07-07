#include "buzzer_hal.h"

// 用于延时函数的平台特定头文件
#if defined(_WIN32)
#include <Windows.h> // 包含 Sleep()
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h> // 包含 sleep()
#endif

// 在 ESP32 上，延时由 FreeRTOS 的 vTaskDelay 处理
#ifdef PLATFORM_ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

// 定义 ESP32 上蜂鸣器连接的 GPIO 引脚。
// 这个定义在 PC 模拟器上会被忽略。
#define BUZZER_PIN 25

// --- 应用逻辑 ---

// 平台无关的延时函数
void delay_ms(int ms) {
#if defined(_WIN32)
    Sleep(ms);
#elif defined(__linux__) || defined(__APPLE__)
    usleep(ms * 1000);
#elif defined(PLATFORM_ESP32)
    vTaskDelay(ms / portTICK_PERIOD_MS);
#endif
}

void play_startup_sound() {
    tone(BUZZER_PIN, 440, 150);
    delay_ms(150);
    tone(BUZZER_PIN, 880, 200);
    delay_ms(200);
}

void play_error_alert() {
    tone(BUZZER_PIN, 1200, 100);
    delay_ms(150); // 包含音符本身时长和暂停时长
    tone(BUZZER_PIN, 1200, 100);
    delay_ms(150);
    tone(BUZZER_PIN, 1200, 100);
    delay_ms(100);
}

void play_twinkle_star() {
    int melody[] = {262, 262, 392, 392, 440, 440, 392, 349, 349, 330, 330, 294, 294, 262};
    int durations[] = {250, 250, 250, 250, 250, 250, 500, 250, 250, 250, 250, 250, 250, 500};
    int noteCount = sizeof(melody) / sizeof(int);

    for (int i = 0; i < noteCount; i++) {
        tone(BUZZER_PIN, melody[i], durations[i]);
        // 在音符之间添加一个小延迟，使它们听起来更分明
        delay_ms(durations[i] + 50);
    }
}


#include <cstdlib> // For system()

// 应用程序的主入口点。
// 在 ESP32 上，这个函数会从 app_main() 中调用。
// 在 PC 上，这就是主函数。
int main() {
    // 解决 Windows 命令行输出中文乱码的问题
    // 将控制台代码页切换为 UTF-8 (65001)，并将输出重定向到 nul 以避免显示 "Active code page: 65001"
    system("chcp 65001 > nul");

    // 对于新的 `tone` 库，不需要显式的 init 函数。
    // `tone()` 函数会在第一次调用时自动处理初始化。

    // 1. 播放开机提示音
    play_startup_sound();
    delay_ms(1000);

    // 播放一段更长的旋律
    play_twinkle_star();

    // 播放错误提示音
    play_error_alert();
    delay_ms(500);
    return 0;
}

// 对于 ESP32，我们需要定义 app_main 入口函数
#ifdef PLATFORM_ESP32
extern "C" void app_main(void)
{
    main();
}
#endif