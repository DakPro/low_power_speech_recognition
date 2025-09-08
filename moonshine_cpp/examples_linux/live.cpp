// live.cpp
#define SDL_MAIN_HANDLED
#include <moonshine.hpp>
#include <SDL.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include "alt_conio.hpp"
#include <csignal> // Required for signal handling
#include <onnxruntime_cxx_api.h>

std::atomic<bool> running(true);

void signalHandler(int signum) {
    if (signum == SIGINT) {
        running.store(false);
    }
}

const int SAMPLE_RATE = 16000;
const int BUFFER_SIZE = 4096;

void audioCallback(void* userdata, Uint8* stream, int len){
    auto* buffer = static_cast<std::vector<float>*>(userdata);
    auto* samples = reinterpret_cast<float*>(stream);
    int sample_count = int(len / sizeof(float));
    buffer->insert(buffer->end(), samples, samples + sample_count);
}

void listAudioDevices(){
    int count = SDL_GetNumAudioDevices(SDL_TRUE);  // SDL_TRUE for recording devices
    std::cout << "Available recording devices:\n";
    for (int i = 0; i < count; ++i){
        const char* name = SDL_GetAudioDeviceName(i, SDL_TRUE);
        std::cout << i << ": " << (name ? name : "Unknown Device") << "\n";
    }
}

int main(int argc, char* argv[]){
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "moonshine");

    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(4);

    // Inter-op threads: number of ops that can run in parallel
    session_options.SetInterOpNumThreads(4);
    session_options.SetExecutionMode(ORT_PARALLEL);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);


    std::cout << "Moonshine Live Transcription\n";
    SDL_SetMainReady();  // Tell SDL we'll handle the main entry point
    if (argc != 2){
        std::cerr << "Usage: " << argv[0] << " <models_dir>\n";
        return 1;
    }

    try{
        // Initialize model
        std::cout << "Initializing...\n";
        MoonshineModel model(argv[1]);
        std::cout << "Model initialized\n";

        // Register signal handler for graceful shutdown
        std::signal(SIGINT, signalHandler);

        // Initialize SDL
        if (SDL_Init(SDL_INIT_AUDIO) < 0){
            std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
            return 1;
        }
        std::cout << "SDL initialized\n";

        // List available devices
        listAudioDevices();

        // Set up audio capture
        SDL_AudioSpec desired_spec;
        SDL_AudioSpec obtained_spec;
        SDL_zero(desired_spec);
        desired_spec.freq = SAMPLE_RATE;
        desired_spec.format = AUDIO_F32;
        desired_spec.channels = 1;
        desired_spec.samples = BUFFER_SIZE;
        desired_spec.callback = audioCallback;

        std::vector<float> audio_buffer;
        desired_spec.userdata = &audio_buffer;

        // Open the default recording device
        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL,      // device name (NULL for default)
                                                    SDL_TRUE,  // is_capture (recording)
                                                    &desired_spec,   // desired spec
                                                    &obtained_spec,  // obtained spec
                                                    SDL_AUDIO_ALLOW_FORMAT_CHANGE);

        if (dev == 0)
        {
            std::cerr << "Could not open audio device: " << SDL_GetError() << "\n";
            SDL_Quit();
            return 1;
        }

        std::cout << "Audio device opened: " << SDL_GetAudioDeviceName(0, SDL_TRUE) << "\n";
        // print the obtained spec
        std::cout << "Obtained spec: " << obtained_spec.freq << " Hz, "
                  << SDL_AUDIO_BITSIZE(obtained_spec.format) << " bits, "
                  << (obtained_spec.channels == 1 ? "mono" : "stereo") << "\n";

        // Start audio capture
        SDL_PauseAudioDevice(dev, 0);

        std::thread transcription_thread(
                [&]()
                {
                    std::cout << "Transcribing...\n";
                    size_t last_buffer_size = 0;
                    while (running)
                    {
                        if (audio_buffer.size() >= SAMPLE_RATE)
                        {
                            if (audio_buffer.size() == last_buffer_size)
                            {
                                // No new audio data
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                continue;
                            }
                            last_buffer_size = audio_buffer.size();

                            // Process audio buffer
                            std::vector<float> buffer(audio_buffer.begin(), audio_buffer.end());

                            // Limit the buffer size to 10 seconds
                            if (audio_buffer.size() > 10 * SAMPLE_RATE)
                            {
                                audio_buffer.erase(audio_buffer.begin(), audio_buffer.end());
                            }

                            // Generate tokens
                            auto start = std::chrono::high_resolution_clock::now();
                            auto tokens = model.generate(buffer);
                            auto end = std::chrono::high_resolution_clock::now();
                            std::chrono::duration<double> duration = end - start;

                            // Detokenize tokens
                            std::string result = model.detokenize(tokens);

                            // erase the last console line
                            std::cout << "\x1b[A";
                            // clear the line
                            std::cout << "\r\033[K";

                            std::cout << "Transcription: " << result << "\n";
                        }
                        else
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                    }
                    std::cout << "Transcription thread finished\n";
                });

        std::cout << "Recording... Press Ctrl+C to stop.\n";
        try{
            while (running)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }

        // Stop audio capture
        SDL_PauseAudioDevice(dev, 1);

        // Wait for transcription thread to finish
        transcription_thread.join();

        // Clean up
        SDL_CloseAudioDevice(dev);
        SDL_Quit();
    }
    catch (const Ort::Exception& e)
    {
        std::cerr << "ONNX Runtime error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}