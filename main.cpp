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

// --- 定义旋律和音效 ---

// 开机提示音：两个音调由低到高的短音
const Note startup_melody[] = {
    {440, 150}, // A4
    {880, 200}  // A5
};

// 错误提示音：三声短促的高音
const Note error_melody[] = {
    {1200, 100},
    {0, 50},    // 暂停
    {1200, 100},
    {0, 50},    // 暂停
    {1200, 100}
};

// 简单旋律：“小星星”
const Note twinkle_melody[] = {
    {262, 250}, {262, 250}, // C4, C4
    {392, 250}, {392, 250}, // G4, G4
    {440, 250}, {440, 250}, // A4, A4
    {392, 500},             // G4
    {349, 250}, {349, 250}, // F4, F4
    {330, 250}, {330, 250}, // E4, E4
    {294, 250}, {294, 250}, // D4, D4
    {262, 500}              // C4
};

// --- 应用逻辑 ---

void play_startup_sound() {
    buzzer_play_melody(startup_melody, sizeof(startup_melody) / sizeof(Note));
}

void play_error_alert() {
    buzzer_play_melody(error_melody, sizeof(error_melody) / sizeof(Note));
}

void play_twinkle_star() {
    buzzer_play_melody(twinkle_melody, sizeof(twinkle_melody) / sizeof(Note));
}


#include <cstdlib> // For system()

// 应用程序的主入口点。
// 在 ESP32 上，这个函数会从 app_main() 中调用。
// 在 PC 上，这就是主函数。
int main() {
    // 解决 Windows 命令行输出中文乱码的问题
    // 将控制台代码页切换为 UTF-8 (65001)，并将输出重定向到 nul 以避免显示 "Active code page: 65001"
    system("chcp 65001 > nul");

    // 初始化蜂鸣器
    buzzer_init(BUZZER_PIN);

    // 1. 播放开机提示音
    play_startup_sound();
    Sleep(1000);

    // 播放一段更长的旋律
    play_twinkle_star();

    // 播放错误提示音
    play_error_alert();
    Sleep(500);
    return 0;
}

// 对于 ESP32，我们需要定义 app_main 入口函数
#ifdef PLATFORM_ESP32
extern "C" void app_main(void)
{
    main();
}
#endif