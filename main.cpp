#include <iostream>
#include <string>
#include <limits>
#include <thread>
#include <chrono>
#include <cstdlib> // For system()
#ifdef _WIN32
#include <conio.h> // For _getch()
#endif
#include "esp32_tone_api.h"
#include "esp32-hal-ledc.h"

// 定义蜂鸣器连接的 GPIO 引脚。
#define BUZZER_PIN 25

// 平台无关的延时函数
void delay_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// 跨平台清屏函数
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    // ANSI escape code for clearing screen on Linux/macOS
    std::cout << "\033[2J\033[1;1H";
#endif
}

// 跨平台“按任意键继续”函数
void press_any_key_to_continue() {
    std::cout << "\n按任意键继续...";
#ifdef _WIN32
    _getch();
#else
    system("stty raw");
    getchar();
    system("stty cooked");
#endif
    std::cout << "\n";
}


// --- 测试函数定义 ---

void test_tone_blocking() {
    std::cout << "\n--- 测试 1: tone() API - 阻塞式播放 ---\n";
    std::cout << "【预期表现】: 您将听到一声中等音调 (440Hz)，持续半秒。程序会在此期间暂停。\n";
    tone(BUZZER_PIN, 440, 500);
    std::cout << "【检验】: 您是否听到了持续半秒的音调？\n";
}

void test_tone_melody() {
    std::cout << "\n--- 测试 2: tone() API - 播放旋律 ---\n";
    std::cout << "【预期表现】: 您将依次听到 'Do-Re-Mi' 三个音符，每个持续0.2秒。\n";
    int melody[] = {262, 294, 330};
    for (int freq : melody) {
        std::cout << "  - 正在播放 " << freq << " Hz\n";
        tone(BUZZER_PIN, freq, 200);
        delay_ms(50); // 音符间的短暂间隔
    }
    std::cout << "【检验】: 您是否听到了 'Do-Re-Mi' 旋律？\n";
}

void test_ledc_attach_write_detach() {
    std::cout << "\n--- 测试 3: ledc API - 附加、写入、分离 ---\n";
    std::cout << "【预期表现】: 您将听到一声高音 (1000Hz)，持续0.3秒。\n";
    std::cout << "  - 步骤 1: ledcAttach()\n";
    ledcAttach(BUZZER_PIN, 1000, 10);
    std::cout << "  - 步骤 2: ledcWriteTone()\n";
    ledcWriteTone(BUZZER_PIN, 1000);
    delay_ms(300);
    std::cout << "  - 步骤 3: ledcDetach()\n";
    ledcDetach(BUZZER_PIN);
    std::cout << "【检验】: 您是否听到了持续0.3秒的高音？\n";
}

void test_ledc_change_freq() {
    std::cout << "\n--- 测试 4: ledc API - 改变频率 ---\n";
    std::cout << "【预期表现】: 您将听到一个由四个音符组成的快速上升音阶。\n";
    ledcAttach(BUZZER_PIN, 500, 10);
    int scale[] = {523, 587, 659, 698};
    for (int freq : scale) {
        std::cout << "  - 改变频率至 " << freq << " Hz\n";
        ledcChangeFrequency(BUZZER_PIN, freq, 10);
        ledcWrite(BUZZER_PIN, 512); // 50% 占空比
        delay_ms(250);
    }
    ledcWrite(BUZZER_PIN, 0); // 停止声音
    ledcDetach(BUZZER_PIN);
    std::cout << "【检验】: 您是否听到了上升的音阶？\n";
}

void test_ledc_write_note() {
    std::cout << "\n--- 测试 5: ledc API - 写音符 ---\n";
    std::cout << "【预期表现】: 您将听到 'La' 和 'Si' 两个音符。\n";
    ledcAttach(BUZZER_PIN, 2000, 10);
    std::cout << "  - 播放音符 A, 八度 4\n";
    ledcWriteNote(BUZZER_PIN, NOTE_A, 4);
    delay_ms(300);
    std::cout << "  - 播放音符 B, 八度 4\n";
    ledcWriteNote(BUZZER_PIN, NOTE_B, 4);
    delay_ms(300);
    ledcDetach(BUZZER_PIN);
    std::cout << "【检验】: 您是否听到了 'La' 和 'Si' 两个音符？\n";
}


void display_menu() {
    std::cout << "========================================\n";
    std::cout << "  ESP32 蜂鸣器模拟器 - 交互式测试台\n";
    std::cout << "========================================\n";
    std::cout << "  高层 tone() API 测试:\n";
    std::cout << "    1. 测试 tone() 的阻塞式播放\n";
    std::cout << "    2. 测试 tone() 播放旋律\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  底层 ledc API 测试:\n";
    std::cout << "    3. 测试 ledcAttach / Write / Detach\n";
    std::cout << "    4. 测试 ledcChangeFrequency\n";
    std::cout << "    5. 测试 ledcWriteNote\n";
    std::cout << "----------------------------------------\n";
    std::cout << "    0. 退出程序\n";
    std::cout << "========================================\n";
    std::cout << "请输入您的选择: ";
}

// 应用程序的主入口点。
int main() {
    // 解决 Windows 命令行输出中文乱码的问题
    system("chcp 65001 > nul");

    int choice = -1;
    while (choice != 0) {
        clear_screen();
        display_menu();
        std::cin >> choice;

        // 处理无效输入（例如输入了非数字字符）
        if (std::cin.fail()) {
            std::cout << "\n错误: 无效输入，请输入一个菜单上的数字。\n";
            std::cin.clear(); // 清除错误标志
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略缓冲区中的剩余内容
            choice = -1; // 重置 choice 以继续循环
            press_any_key_to_continue();
            continue;
        }

        switch (choice) {
            case 1: test_tone_blocking(); break;
            case 2: test_tone_melody(); break;
            case 3: test_ledc_attach_write_detach(); break;
            case 4: test_ledc_change_freq(); break;
            case 5: test_ledc_write_note(); break;
            case 0: break; // 退出循环
            default:
                std::cout << "\n错误: 无效选项，请重新选择。\n";
                break;
        }

        if (choice != 0) {
            press_any_key_to_continue();
        }
    }

    std::cout << "\n程序已退出。\n";
    return 0;
}