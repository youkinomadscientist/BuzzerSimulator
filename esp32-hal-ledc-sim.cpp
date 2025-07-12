#include "esp32-hal-ledc.h"
#include <iostream>
#include <atomic>
#include <vector>
#include <cmath>

// 确保此文件仅在 PC 平台上编译
#ifndef PLATFORM_ESP32
#define PLATFORM_PC
#endif

#ifdef PLATFORM_PC

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// --- miniaudio 音频后端 ---

struct LedcChannelState {
    std::atomic<double> frequency;
    std::atomic<double> phase;
    std::atomic<uint32_t> duty; // 0-1023 for 10-bit resolution
    std::atomic<uint32_t> resolution_max_duty; // e.g., 1023 for 10-bit
    std::atomic<bool> attached;
};

// 我们模拟 ESP32 的16个LEDC通道
#define NUM_LEDC_CHANNELS 16
static LedcChannelState g_ledc_channels[NUM_LEDC_CHANNELS];
static std::vector<int> g_pin_to_channel(256, -1); // GPIO pin -> channel mapping

static ma_device g_audio_device;
static bool g_audio_initialized = false;

// 音频回调函数，由 miniaudio 调用以生成音频样本
void sim_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    (void)pInput;
    float* pOutputF32 = (float*)pOutput;
    double sampleRate = pDevice->sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        pOutputF32[i] = 0.0f;
    }

    // 混合所有活动通道的声音
    for (int ch = 0; ch < NUM_LEDC_CHANNELS; ++ch) {
        if (!g_ledc_channels[ch].attached.load() || g_ledc_channels[ch].duty.load() == 0) {
            continue;
        }

        double freq = g_ledc_channels[ch].frequency.load();
        if (freq <= 0) continue;

        double phase = g_ledc_channels[ch].phase.load();
        double phase_increment = freq / sampleRate;
        
        for (ma_uint32 frame = 0; frame < frameCount; ++frame) {
            // 生成方波
            if (phase < 0.5) {
                pOutputF32[frame] += 0.1f; // 幅度，多个通道时需要减小
            } else {
                pOutputF32[frame] -= 0.1f;
            }
            phase += phase_increment;
            if (phase >= 1.0) {
                phase -= 1.0;
            }
        }
        g_ledc_channels[ch].phase.store(phase);
    }
}

// 确保 miniaudio 已初始化
static void ensure_audio_initialized() {
    if (g_audio_initialized) return;

    for(int i=0; i<NUM_LEDC_CHANNELS; ++i) {
        g_ledc_channels[i].frequency.store(0.0);
        g_ledc_channels[i].phase.store(0.0);
        g_ledc_channels[i].duty.store(0);
        g_ledc_channels[i].resolution_max_duty.store(1023); // 默认10位
        g_ledc_channels[i].attached.store(false);
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1; // Mono
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = sim_data_callback;

    if (ma_device_init(NULL, &deviceConfig, &g_audio_device) != MA_SUCCESS) {
        std::cerr << "[SIM_LEDC] Failed to initialize audio device." << std::endl;
        return;
    }
    
    if (ma_device_start(&g_audio_device) != MA_SUCCESS) {
        std::cerr << "[SIM_LEDC] Failed to start audio device." << std::endl;
        ma_device_uninit(&g_audio_device);
        return;
    }

    g_audio_initialized = true;
    std::cout << "[SIM_LEDC] miniaudio device initialized and started." << std::endl;
}

// --- 模拟 LEDC 函数实现 ---

extern "C" {

bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution) {
    // 自动寻找一个空闲通道
    for (int ch = 0; ch < NUM_LEDC_CHANNELS; ++ch) {
        if (!g_ledc_channels[ch].attached.load()) {
            return ledcAttachChannel(pin, freq, resolution, ch);
        }
    }
    log_e("ledcAttach: No free channels available.");
    return false;
}

bool ledcAttachChannel(uint8_t pin, uint32_t freq, uint8_t resolution, uint8_t channel) {
    ensure_audio_initialized();
    if (channel >= NUM_LEDC_CHANNELS) {
        log_e("ledcAttachChannel: Invalid channel %d", channel);
        return false;
    }
    g_pin_to_channel[pin] = channel;
    g_ledc_channels[channel].resolution_max_duty.store((1 << resolution) - 1);
    g_ledc_channels[channel].frequency.store((double)freq);
    g_ledc_channels[channel].attached.store(true);
    log_d("Attached pin %d to channel %d with freq %u Hz, %d-bit resolution", pin, channel, freq, resolution);
    return true;
}

bool ledcWrite(uint8_t pin, uint32_t duty) {
    int channel = g_pin_to_channel[pin];
    if (channel == -1) {
        log_e("ledcWrite: Pin %d not attached to any channel.", pin);
        return false;
    }
    return ledcWriteChannel(channel, duty);
}

bool ledcWriteChannel(uint8_t channel, uint32_t duty) {
    if (channel >= NUM_LEDC_CHANNELS || !g_ledc_channels[channel].attached.load()) {
        log_e("ledcWriteChannel: Invalid or unattached channel %d", channel);
        return false;
    }
    g_ledc_channels[channel].duty.store(duty);
    // log_d("Wrote duty %u to channel %d", duty, channel);
    return true;
}

uint32_t ledcWriteTone(uint8_t pin, uint32_t freq) {
    int channel = g_pin_to_channel[pin];
    if (channel == -1) {
        log_e("ledcWriteTone: Pin %d not attached to any channel.", pin);
        return 0;
    }
    g_ledc_channels[channel].frequency.store((double)freq);
    // 50% duty cycle for a tone
    uint32_t duty = (freq > 0) ? (g_ledc_channels[channel].resolution_max_duty.load() / 2) : 0;
    ledcWrite(pin, duty);
    // log_d("Wrote tone %u Hz to pin %d (channel %d)", freq, pin, channel);
    return freq;
}

uint32_t ledcWriteNote(uint8_t pin, note_t note, uint8_t octave) {
    const uint16_t noteFrequencyBase[] = {
        // C,   C#,  D,   D#,  E,   F,   F#,  G,   G#,  A,   A#,  B
        4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902
    };
    if (note >= NOTE_MAX || octave > 8) {
        return 0;
    }
    uint32_t freq = noteFrequencyBase[note] / (1 << (8 - octave));
    return ledcWriteTone(pin, freq);
}

uint32_t ledcRead(uint8_t pin) {
    int channel = g_pin_to_channel[pin];
    if (channel == -1) return 0;
    return g_ledc_channels[channel].duty.load();
}

uint32_t ledcReadFreq(uint8_t pin) {
    int channel = g_pin_to_channel[pin];
    if (channel == -1) return 0;
    return (uint32_t)g_ledc_channels[channel].frequency.load();
}

bool ledcDetach(uint8_t pin) {
    int channel = g_pin_to_channel[pin];
    if (channel != -1) {
        g_ledc_channels[channel].attached.store(false);
        g_ledc_channels[channel].duty.store(0);
        g_pin_to_channel[pin] = -1;
        log_d("Detached pin %d from channel %d", pin, channel);
    }
    return true;
}

uint32_t ledcChangeFrequency(uint8_t pin, uint32_t freq, uint8_t resolution) {
    int channel = g_pin_to_channel[pin];
    if (channel == -1) {
        log_e("ledcChangeFrequency: Pin %d not attached.", pin);
        return 0;
    }
    g_ledc_channels[channel].frequency.store((double)freq);
    g_ledc_channels[channel].resolution_max_duty.store((1 << resolution) - 1);
    log_d("Changed pin %d (channel %d) to freq %u Hz, %d-bit resolution", pin, channel, freq, resolution);
    return freq;
}

} // extern "C"

#endif // PLATFORM_PC