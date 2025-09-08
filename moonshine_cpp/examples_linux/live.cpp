// live_transcription.cpp
#define SDL_MAIN_HANDLED
#include <moonshine.hpp>
#include <SDL.h>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <csignal>
#include <cmath>

std::atomic<bool> running(true);
void signalHandler(int signum) {
    if (signum == SIGINT) running.store(false);
}

// Audio / timing constants
const int SAMPLE_RATE = 16000;
const int CHUNK_SIZE = 512; // match your Python/silero chunk
const int LOOKBACK_CHUNKS = 5;
const size_t LOOKBACK_SAMPLES = LOOKBACK_CHUNKS * CHUNK_SIZE;
const float MIN_REFRESH_SECS = 0.4f; // partial-refresh cadence
const float MAX_SPEECH_SECS = 5.0f; // cap segment length
const float VAD_START_THRESHOLD = 0.01f; // RMS threshold to consider speech start
const int SILENCE_CHUNKS_TO_END = 10; // number of consecutive quiet chunks to mark end (~0.32s)
const size_t MIN_MODEL_SAMPLES = 1024; // ~64 ms

// Thread-shared audio buffer (use deque for efficient pop_front)
std::mutex audio_mutex;

struct RingBuffer {
    std::vector<float> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t capacity;

    explicit RingBuffer(size_t size) : buffer(size), capacity(size) {}

    [[nodiscard]] size_t size() const {
        if (head >= tail) return head - tail;
        else return capacity - tail + head;
    }

    bool push(const float* data, size_t n) {
        if (n > capacity - size()) return false; // overflow
        for (size_t i = 0; i < n; ++i) {
            buffer[head] = data[i];
            head = (head + 1) % capacity;
        }
        return true;
    }

    size_t pop(float* out, size_t n) {
        size_t avail = size();
        size_t to_pop = std::min(n, avail);
        for (size_t i = 0; i < to_pop; ++i) {
            out[i] = buffer[tail];
            tail = (tail + 1) % capacity;
        }
        return to_pop;
    }
};

float compute_rms(const float* data, size_t n) {
    float s = 0.0f;
    for (size_t i = 0; i < n; ++i) s += data[i] * data[i];
    return (n > 0) ? std::sqrt(s / (float) n) : 0.0f;
}
// helper to print and overwrite single line (keeps terminal tidy)
void print_overwrite(const std::string& text){
    // Clear current line and overwrite
    std::cout << "\r\033[K" << text << std::flush;
}

// Memory allocation before runtime
RingBuffer audio_buffer(SAMPLE_RATE * 5); // 5 sec buffer
RingBuffer speech_buffer((size_t)(MAX_SPEECH_SECS * SAMPLE_RATE));
std::vector<float> warmup(SAMPLE_RATE, 0.0f);
float dummy_arr[SAMPLE_RATE];

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <models_dir>\n";
        return 1;
    }
    std::signal(SIGINT, signalHandler);

    try
    {
        MoonshineModel model(argv[1]);
        try
        {
            model.generate(warmup);
        }
        catch (...)
        {
        }  // Warmup

        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
            return 1;
        }

        SDL_AudioSpec spec;
        SDL_zero(spec);
        spec.freq = SAMPLE_RATE;
        spec.format = AUDIO_F32;
        spec.channels = 1;
        spec.samples = CHUNK_SIZE;

        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &spec, nullptr, 0);
        if (!dev)
        {
            std::cerr << "Could not open audio device: " << SDL_GetError() << "\n";
            SDL_Quit();
            return 1;
        }

        SDL_PauseAudioDevice(dev, 0);

        std::thread capture_thread(
            [&]()
            {
                std::vector<float> tmp(CHUNK_SIZE);
                while (running.load())
                {
                    int queued_bytes = (int)SDL_GetQueuedAudioSize(dev);
                    if (queued_bytes >= (int)(CHUNK_SIZE * sizeof(float)))
                    {
                        int got =
                            (int)SDL_DequeueAudio(dev, tmp.data(), CHUNK_SIZE * sizeof(float));
                        if (got > 0) audio_buffer.push(tmp.data(), CHUNK_SIZE);
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
            });

        std::thread transcribe_thread(
            [&]()
            {
                std::vector<float> chunk(CHUNK_SIZE);
                bool recording = false;
                int silence_chunks = 0;
                auto last_infer_time = std::chrono::steady_clock::now();

                while (running.load())
                {
                    if (audio_buffer.size() < CHUNK_SIZE){
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }
                    audio_buffer.pop(chunk.data(), CHUNK_SIZE);
                    float rms = compute_rms(chunk.data(), CHUNK_SIZE);

                    // Push into speech buffer
                    speech_buffer.push(chunk.data(), CHUNK_SIZE);

                    if (!recording)
                        while(speech_buffer.size() > LOOKBACK_SAMPLES) // keep look-back
                            speech_buffer.pop(dummy_arr,std::min(SAMPLE_RATE, int(speech_buffer.size()-LOOKBACK_SAMPLES)));


                    // VAD start
                    if (!recording && rms > VAD_START_THRESHOLD)
                    {
                        recording = true;
                        silence_chunks = 0;
                        last_infer_time = std::chrono::steady_clock::now();
                        std::cout << "\n[Speech started]\n";
                    }

                    if (recording)
                    {
                        if (rms <= VAD_START_THRESHOLD)
                            ++silence_chunks;
                        else
                            silence_chunks = 0;

                        // Check end conditions
                        if (silence_chunks >= SILENCE_CHUNKS_TO_END ||
                            speech_buffer.size() >= speech_buffer.capacity)
                        {
                            // Convert speech buffer to vector
                            std::vector<float> speech(speech_buffer.size());
                            speech_buffer.pop(speech.data(), speech.size());

                            try
                            {
                                auto tokens = model.generate(speech);
                                std::string result = model.detokenize(tokens);
                                print_overwrite("Transcription: " + result + "\n");
                            }
                            catch (const std::exception& e)
                            {
                                print_overwrite(std::string("Transcription error: ") + e.what() +
                                                "\n");
                            }

                            recording = false;
                            silence_chunks = 0;
                            std::cout << "[Speech ended]\n";
                            continue;
                        }

                        // Partial transcription
                        auto now = std::chrono::steady_clock::now();
                        std::chrono::duration<double> elapsed = now - last_infer_time;
                        if (elapsed.count() >= MIN_REFRESH_SECS)
                        {
                            if (speech_buffer.size() >= MIN_MODEL_SAMPLES) {
                                std::vector<float> speech(speech_buffer.size());
                                // copy without popping
                                for (size_t i = 0; i < speech_buffer.size(); ++i) {
                                    speech[i] = speech_buffer.buffer[(speech_buffer.tail + i) % speech_buffer.capacity];
                                }

                                try {
                                    auto tokens = model.generate(speech);
                                    std::string partial = model.detokenize(tokens);
                                    print_overwrite("Partial: " + partial);
                                }
                                catch (...) {}
                            }
                            last_infer_time = now;
                        }
                    }
                }

                // Flush remaining
                if (speech_buffer.size() > 0)
                {
                    std::vector<float> speech(speech_buffer.size());
                    speech_buffer.pop(speech.data(), speech.size());
                    try
                    {
                        auto tokens = model.generate(speech);
                        std::string final = model.detokenize(tokens);
                        print_overwrite("Final: " + final + "\n");
                    }
                    catch (...)
                    {
                    }
                }
            });

        std::cout << "Recording started (press Ctrl+C to stop)\n";
        while (running.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

        SDL_PauseAudioDevice(dev, 1);
        capture_thread.join();
        transcribe_thread.join();
        SDL_CloseAudioDevice(dev);
        SDL_Quit();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}