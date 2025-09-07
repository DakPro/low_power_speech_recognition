// main.cpp
#include <moonshine.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

#include <sndfile.h>



std::vector<float> readWavFile(const std::string &filename){
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        throw std::runtime_error("Error opening file");
    }

//    std::cerr << "Frames: " << sfinfo.frames << std::endl;
//    std::cerr << "Sample rate: " << sfinfo.samplerate << std::endl;
//    std::cerr << "Channels: " << sfinfo.channels << std::endl;

    // Read audio data
    std::vector<float> buffer(sfinfo.frames * sfinfo.channels);
    sf_readf_float(file, buffer.data(), sfinfo.frames);

    sf_close(file);
    return buffer;
}

std::string transcribe(MoonshineModel &model, std::vector<float> &audio_samples){
    try
    {
        auto tokens = model.generate(audio_samples);
        std::string result = model.detokenize(tokens);
        std::cout << "Detokenized: " << result << "\n";
        return result;
    }
    catch (const Ort::Exception &e) {
        throw std::runtime_error("ONNX Runtime error: "+std::string(e.what()) + "\n");
    }
    catch (const std::exception &e){
        throw std::runtime_error("Error: "+std::string(e.what())+"\n");
    }
}
int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <wav_file> [<models_dir>]\n";
        return 1;
    }

    // Load audio
    auto audio_samples = readWavFile(argv[1]);
    // Load model
    MoonshineModel model(argc==3?argv[1]:"model/base");
    transcribe(model, audio_samples);
}
