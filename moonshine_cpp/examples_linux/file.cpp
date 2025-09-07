// main.cpp
#include <moonshine.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>

#include <sndfile.h>

const unsigned long long SAMPLE_RATE = 16000;

std::vector<float> readWavFile(const std::string &filename){
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        throw std::runtime_error("Error opening file");
    }


    std::vector<float> buffer(sfinfo.frames * sfinfo.channels);
    sf_readf_float(file, buffer.data(), sfinfo.frames);
    sf_close(file);
    return buffer;
}

std::string transcribe(MoonshineModel &model, std::vector<float> &audio_samples){
    try
    {
        std::ostringstream ans;
        unsigned long long length = audio_samples.size();
        auto start = std::chrono::high_resolution_clock::now();
        for(unsigned long long i=0; i<length; i+=SAMPLE_RATE*30)
        {
            std::vector audio_sample(audio_samples.begin()+i,
                                     audio_samples.begin()+std::min(i+SAMPLE_RATE*30, length));
            auto tokens = model.generate(audio_sample);
            std::string result = model.detokenize(tokens);
            result.erase(0,3);
            result.erase(result.end()-4,result.end());
        }
        std::string ans_str = ans.str();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Time it took to transcribe: " << duration.count() << '\n';
        std::cout << "Transcript:\n" << ans_str << '\n';
        return ans_str;
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
    std::cout << "Initializing model\n";
    MoonshineModel model(argc==3?argv[1]:"model/base");
    std::cout << "Model initialized\n";
    transcribe(model, audio_samples);
}
