#include "buzzer_hal.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// 此预处理器检查确保此文件仅在定义了 PLATFORM_PC 时才被编译。
#ifdef PLATFORM_PC

// 通过在所有其他标准头文件之后包含 miniaudio 的实现，
// 我们可以有时避免模糊的编译器/链接器错误。
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// 用于保存音频状态的结构体，在主线程和音频回调之间共享
struct AudioState {
    std::atomic<double> frequency;
    std::atomic<double> phase;
    std::atomic<bool> is_playing;
};

// 默认初始化全局状态对象。我们将在 buzzer_init 中设置其值。
static AudioState g_audio_state;
static ma_device g_audio_device;
static bool g_audio_initialized = false;

// 此函数由 miniaudio 在单独的线程中调用以生成音频样本。
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput; // 未使用。

    float* pOutputF32 = (float*)pOutput;
    double freq = g_audio_state.frequency.load();
    bool is_playing = g_audio_state.is_playing.load();
    
    if (freq <= 0 || !is_playing) {
        // 如果未播放或频率无效，则输出静音。
        for (ma_uint32 i = 0; i < frameCount; ++i) {
            pOutputF32[i] = 0.0f;
        }
        return;
    }

    double phase = g_audio_state.phase.load();
    double sampleRate = pDevice->sampleRate;
    double phase_increment = freq / sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        // 生成方波
        if (phase < 0.5) {
            pOutputF32[i] = 0.2f; // 幅度
        } else {
            pOutputF32[i] = -0.2f; // 幅度
        }
        
        phase += phase_increment;
        if (phase >= 1.0) {
            phase -= 1.0;
        }
    }
    g_audio_state.phase.store(phase);
}

void buzzer_init(int pin) {
    (void)pin; // 在模拟中忽略引脚。

    if (g_audio_initialized) {
        return;
    }

    // 使用 store() 显式初始化原子成员。这是最安全的方法。
    g_audio_state.frequency.store(0.0);
    g_audio_state.phase.store(0.0);
    g_audio_state.is_playing.store(false);

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1; // Mono
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = data_callback;

    if (ma_device_init(NULL, &deviceConfig, &g_audio_device) != MA_SUCCESS) {
        std::cerr << "[BUZZER_SIM] 初始化音频设备失败。" << std::endl;
        return;
    }
    
    g_audio_initialized = true;
    std::cout << "[BUZZER_SIM] miniaudio 设备初始化成功。" << std::endl;
}

void buzzer_play(int frequency, int duration_ms) {
    if (!g_audio_initialized) return;

    if (frequency > 0) {
        std::cout << "[BUZZER_SIM] 播放 " << frequency << "Hz 持续 " << duration_ms << "ms" << std::endl;
        buzzer_start(frequency);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        buzzer_stop();
    } else {
        std::cout << "[BUZZER_SIM] 暂停 " << duration_ms << "ms" << std::endl;
        buzzer_stop(); // 确保静音
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    }
}

void buzzer_start(int frequency) {
    if (!g_audio_initialized) return;

    if (frequency > 0) {
        g_audio_state.frequency.store((double)frequency);
        g_audio_state.is_playing.store(true);
        if (ma_device_get_state(&g_audio_device) != ma_device_state_started) {
            ma_device_start(&g_audio_device);
        }
    }
}

void buzzer_stop() {
    if (!g_audio_initialized) return;
    
    g_audio_state.is_playing.store(false);
    // 我们不停止设备，只输出静音。这更高效。
}

void buzzer_play_melody(const Note* melody, int length) {
    std::cout << "[BUZZER_SIM] 播放包含 " << length << " 个音符的旋律。" << std::endl;
    for (int i = 0; i < length; ++i) {
        buzzer_play(melody[i].frequency, melody[i].duration_ms);
        // 音符之间的短暂延迟现在由 buzzer_play 的精确计时处理。
    }
    std::cout << "[BUZZER_SIM] 旋律播放完毕。" << std::endl;
}

// 当程序退出时，我们应该取消初始化设备，但对于这个简单的
// 模拟，我们将让操作系统清理它。更健壮的实现会
// 使用 RAII 或显式的 buzzer_deinit() 函数。

#endif // PLATFORM_PC