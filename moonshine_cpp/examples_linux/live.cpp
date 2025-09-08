// live_transcription.cpp
#define SDL_MAIN_HANDLED
#include <moonshine.hpp>
#include <SDL.h>
#include <iostream>
#include <vector>
#include <deque>
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
const float MIN_REFRESH_SECS = 0.2f; // partial-refresh cadence
const float MAX_SPEECH_SECS = 15.0f; // cap segment length
const float VAD_START_THRESHOLD = 0.01f; // RMS threshold to consider speech start
const int SILENCE_CHUNKS_TO_END = 10; // number of consecutive quiet chunks to mark end (~0.192s)

// Thread-shared audio buffer (use deque for efficient pop_front)
std::deque<float> audio_buffer;
std::mutex audio_mutex;

// SDL audio callback: push samples into shared deque
void audioCallback([[maybe_unused]] void* userdata, Uint8* stream, int len){
    auto* samples = reinterpret_cast<float*>(stream);
    int sample_count = int(len / sizeof(float));
    std::lock_guard<std::mutex> lock(audio_mutex);
    for (int i = 0; i < sample_count; ++i) audio_buffer.push_back(samples[i]);
}

// compute RMS of a chunk
float compute_rms(const std::vector<float>& chunk){
    double s = 0.0;
    for (float v : chunk) s += double(v) * double(v);
    if (chunk.empty()) return 0.0f;
    return float(std::sqrt(s / (double)chunk.size()));
}

// helper to print and overwrite single line (keeps terminal tidy)
void print_overwrite(const std::string& text){
    // Clear current line and overwrite
    std::cout << "\r\033[K" << text << std::flush;
}

int main(int argc, char* argv[]){
    if (argc != 2){
        std::cerr << "Usage: " << argv[0] << " <models_dir>\n";
        return 1;
    }

    std::signal(SIGINT, signalHandler);

    try{
        std::cout << "Loading Moonshine model...\n";
        MoonshineModel model(argv[1]);

        // Warmup inference (one second of silence) to avoid long first inference
        std::vector<float> warmup(SAMPLE_RATE, 0.0f);
        try {
            auto dummy = model.generate(warmup);
            (void)dummy;
        } catch (...) {
            // ignore warmup failures (but report)
            std::cerr << "Warning: warmup inference failed (continuing)\n";
        }

        // Initialize SDL for audio capture
        if (SDL_Init(SDL_INIT_AUDIO) < 0){
            std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
            return 1;
        }

        SDL_AudioSpec desired_spec;
        SDL_AudioSpec obtained_spec;
        SDL_zero(desired_spec);
        desired_spec.freq = SAMPLE_RATE;
        desired_spec.format = AUDIO_F32;
        desired_spec.channels = 1;
        desired_spec.samples = CHUNK_SIZE;
        desired_spec.callback = audioCallback;

        // Open default recording device
        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &desired_spec, &obtained_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        if (dev == 0){
            std::cerr << "Could not open audio device: " << SDL_GetError() << "\n";
            SDL_Quit();
            return 1;
        }

        std::cout << "Recording started (press Ctrl+C to stop)\n";
        SDL_PauseAudioDevice(dev, 0);

        // Transcription thread: consumes from audio_buffer using simple energy VAD
        std::thread transcribe_thread([&](){
                                          std::vector<float> speech; // collected speech while recording
                                          bool recording = false;
                                          size_t silence_chunks = 0;
                                          auto last_infer_time = std::chrono::steady_clock::now();
                                          double total_speech_secs = 0.0;
                                          size_t last_audio_size = 0;

                                          while (running.load()){
                                              // collect a chunk worth of samples from shared buffer
                                              std::vector<float> chunk;
                                              {
                                                  std::lock_guard<std::mutex> lock(audio_mutex);
                                                  while (audio_buffer.size() >= CHUNK_SIZE && chunk.size() < CHUNK_SIZE){
                                                      chunk.push_back(audio_buffer.front());
                                                      audio_buffer.pop_front();
                                                  }
                                              }

                                              if (chunk.empty()){
                                                  std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                  continue;
                                              }

                                              float rms = compute_rms(chunk);

                                              // If not recording, keep only lookback buffer
                                              if (!recording){
                                                  // append lookback chunk size, but cap to LOOKBACK_SAMPLES
                                                  speech.insert(speech.end(), chunk.begin(), chunk.end());
                                                  if (speech.size() > LOOKBACK_SAMPLES) speech.erase(speech.begin(), speech.begin() + (long)(speech.size() - LOOKBACK_SAMPLES));
                                              } else {
                                                  // while recording, append everything
                                                  speech.insert(speech.end(), chunk.begin(), chunk.end());
                                              }

                                              // simple VAD heuristics
                                              if (!recording && rms > VAD_START_THRESHOLD){
                                                  recording = true;
                                                  silence_chunks = 0;
                                                  last_infer_time = std::chrono::steady_clock::now();
                                                  // keep a small lookback for context
                                                  if (speech.size() > LOOKBACK_SAMPLES) {
                                                      // already capped above
                                                  }
                                                  std::cout << "\n[Speech started]\n";
                                              }

                                              if (recording){
                                                  if (rms <= VAD_START_THRESHOLD) {
                                                      ++silence_chunks;
                                                  } else {
                                                      silence_chunks = 0;
                                                  }

                                                  // If we've detected enough silence, or the speech is too long, treat as end
                                                  double speech_duration = double(speech.size()) / SAMPLE_RATE;
                                                  if (silence_chunks >= SILENCE_CHUNKS_TO_END || speech_duration > MAX_SPEECH_SECS){
                                                      // Final inference
                                                      try {
                                                          auto start = std::chrono::steady_clock::now();
                                                          auto tokens = model.generate(speech);
                                                          std::string result = model.detokenize(tokens);
                                                          auto end = std::chrono::steady_clock::now();
                                                          std::chrono::duration<double> d = end - start;
                                                          print_overwrite(std::string("Transcription: ") + result + "\n");
                                                      } catch (const std::exception& e){
                                                          print_overwrite(std::string("Transcription error: ") + e.what() + "\n");
                                                      }

                                                      // reset
                                                      speech.clear();
                                                      recording = false;
                                                      silence_chunks = 0;
                                                      std::cout << "[Speech ended]\n";
                                                      continue; // go fetch more audio
                                                  }

                                                  // Incremental refresh: run partial transcription every MIN_REFRESH_SECS
                                                  auto now = std::chrono::steady_clock::now();
                                                  std::chrono::duration<double> since_last = now - last_infer_time;
                                                  if (since_last.count() >= MIN_REFRESH_SECS){
                                                      try {
                                                          auto tokens = model.generate(speech);
                                                          std::string partial = model.detokenize(tokens);
                                                          print_overwrite(std::string("Partial: ") + partial);
                                                      } catch (const std::exception& e){
                                                          print_overwrite(std::string("Partial error: ") + e.what());
                                                      }
                                                      last_infer_time = now;
                                                  }
                                              }
                                          }

                                          // On exit: flush remaining speech if any
                                          if (!speech.empty()){
                                              try {
                                                  auto tokens = model.generate(speech);
                                                  std::string final = model.detokenize(tokens);
                                                  print_overwrite(std::string("Final: ") + final + "\n");
                                              } catch (...) {
                                                  // swallow
                                              }
                                          }
                                      });

        // main loop just waits for SIGINT
        while (running.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Stop audio
        SDL_PauseAudioDevice(dev, 1);
        transcribe_thread.join();

        SDL_CloseAudioDevice(dev);
        SDL_Quit();

    } catch (const Ort::Exception& e){
        std::cerr << "ONNX Runtime error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e){
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
