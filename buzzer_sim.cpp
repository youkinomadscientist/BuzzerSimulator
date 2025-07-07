#include "buzzer_hal.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

// This preprocessor check ensures this file is only compiled when PLATFORM_PC is defined.
#ifdef PLATFORM_PC

// By including miniaudio's implementation *after* all other standard headers,
// we can sometimes avoid obscure compiler/linker errors.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Structure to hold our audio state, shared between main thread and audio callback
struct AudioState {
    std::atomic<double> frequency;
    std::atomic<double> phase;
    std::atomic<bool> is_playing;
};

// Default-initialize the global state object. We will set its values in buzzer_init.
static AudioState g_audio_state;
static ma_device g_audio_device;
static bool g_audio_initialized = false;

// This function is called by miniaudio in a separate thread to generate audio samples.
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput; // Unused.

    float* pOutputF32 = (float*)pOutput;
    double freq = g_audio_state.frequency.load();
    bool is_playing = g_audio_state.is_playing.load();
    
    if (freq <= 0 || !is_playing) {
        // If not playing or frequency is invalid, output silence.
        for (ma_uint32 i = 0; i < frameCount; ++i) {
            pOutputF32[i] = 0.0f;
        }
        return;
    }

    double phase = g_audio_state.phase.load();
    double sampleRate = pDevice->sampleRate;
    double phase_increment = freq / sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        // Generate a square wave
        if (phase < 0.5) {
            pOutputF32[i] = 0.2f; // Amplitude
        } else {
            pOutputF32[i] = -0.2f; // Amplitude
        }
        
        phase += phase_increment;
        if (phase >= 1.0) {
            phase -= 1.0;
        }
    }
    g_audio_state.phase.store(phase);
}

void buzzer_init(int pin) {
    (void)pin; // Pin is ignored in simulation.

    if (g_audio_initialized) {
        return;
    }

    // Explicitly initialize the atomic members using store(). This is the safest way.
    g_audio_state.frequency.store(0.0);
    g_audio_state.phase.store(0.0);
    g_audio_state.is_playing.store(false);

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1; // Mono
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = data_callback;

    if (ma_device_init(NULL, &deviceConfig, &g_audio_device) != MA_SUCCESS) {
        std::cerr << "[BUZZER_SIM] Failed to initialize audio device." << std::endl;
        return;
    }
    
    g_audio_initialized = true;
    std::cout << "[BUZZER_SIM] miniaudio device initialized successfully." << std::endl;
}

void buzzer_play(int frequency, int duration_ms) {
    if (!g_audio_initialized) return;

    if (frequency > 0) {
        std::cout << "[BUZZER_SIM] Playing " << frequency << "Hz for " << duration_ms << "ms" << std::endl;
        buzzer_start(frequency);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        buzzer_stop();
    } else {
        std::cout << "[BUZZER_SIM] Pausing for " << duration_ms << "ms" << std::endl;
        buzzer_stop(); // Ensure silence
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
    // We don't stop the device, just output silence. This is more efficient.
}

void buzzer_play_melody(const Note* melody, int length) {
    std::cout << "[BUZZER_SIM] Playing melody with " << length << " notes." << std::endl;
    for (int i = 0; i < length; ++i) {
        buzzer_play(melody[i].frequency, melody[i].duration_ms);
        // The small delay between notes is now handled by the precise timing of buzzer_play.
    }
    std::cout << "[BUZZER_SIM] Melody finished." << std::endl;
}

// We should uninitialize the device when the program exits, but for this simple
// simulation, we'll let the OS clean it up. A more robust implementation would
// use RAII or an explicit buzzer_deinit() function.

#endif // PLATFORM_PC