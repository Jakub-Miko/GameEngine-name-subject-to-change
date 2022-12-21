#include "AudioSystem.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdexcept>
#include <string>
#include <FileManager.h>
#include <filesystem>
#include <fstream>
#include <Application.h>
#include <AsyncTaskDispatcher.h>

AudioSystem* AudioSystem::instance = nullptr;

void AudioSystem::Init()
{
	if (!instance) {
		instance = new AudioSystem();
	}
}

void AudioSystem::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

AudioSystem* AudioSystem::Get()
{
	return instance;
}

std::shared_ptr<AudioObject> AudioSystem::GetAudioObject(const std::string& input_path)
{
	using namespace std::filesystem;
	std::lock_guard<std::mutex> lock(audio_object_mutex);
	std::string absolute_path = absolute(path(input_path)).generic_string();
	std::string relative_path = FileManager::Get()->GetRelativeFilePath(absolute_path);
	auto fnd = audio_object_map.find(relative_path);
	
	if (fnd != audio_object_map.end()) {
		return fnd->second;
	}


    auto object = std::shared_ptr<AudioStandardObject>(new AudioStandardObject(), [](AudioStandardObject* ptr) {
        ALuint buffer_id = ptr->GetBufferId();
        alDeleteBuffers(1, &buffer_id);
        delete ptr;
        });

    object->state = AudioObjectState::LOADING;
    object->buffer_id = std::dynamic_pointer_cast<AudioStandardObject>(GetDefaultAudioObject())->buffer_id;

    auto audio_final = audio_object_map.insert(std::make_pair(relative_path, object)).first->second;


    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<AudioStandardObject>([absolute_path, this]() -> AudioStandardObject {
        return LoadAudioFromFileImpl(absolute_path);
        });

    async_queue->Submit(task);

    audio_load_future future;
    future.future = task->GetFuture();
    future.audio = std::dynamic_pointer_cast<AudioStandardObject>(audio_final);
    future.path = relative_path;

    std::lock_guard<std::mutex> lock2(audio_Load_queue_mutex);
    audio_Load_queue.push_back(future);

    return audio_final; 
}

void AudioSystem::UpdateLoadedSounds()
{
    std::lock_guard<std::mutex> lock(audio_Load_queue_mutex);
    for (auto& loaded_audio : audio_Load_queue) {
        if (!loaded_audio.future.IsAvailable() || loaded_audio.destroyed) continue;
        try {
            *(loaded_audio.audio) = std::move(loaded_audio.future.GetValue());

            loaded_audio.destroyed = true;
        }
        catch (...) {
            loaded_audio.audio->state= AudioObjectState::ERROR;
            std::lock_guard<std::mutex> lock(audio_object_mutex);
            audio_object_map.erase(loaded_audio.path);
            loaded_audio.destroyed = true;
        }
    }


    while (!audio_Load_queue.empty() && audio_Load_queue.front().destroyed) {
        audio_Load_queue.pop_front();
    }

}

AudioStandardObject AudioSystem::LoadAudioFromFileImpl(const std::string& input_path)
{
    AudioStandardObject object;
	std::string extension = std::filesystem::path(input_path).extension().generic_string();
	
	if (extension == ".wav") {
		ImportWaveFormAudioObject(object, input_path);
	}
	else {
		throw std::runtime_error("Only Waveform(.wav) audio format is currently supported");
	}

	return object;
}

void AudioSystem::ImportWaveFormAudioObject(AudioStandardObject& object, const std::string& path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error((std::string)"Waveform file: " + path + " could not be opened");

    char buffer[4];

    if (!file.read(buffer, 4) || strncmp(buffer,"RIFF",4) != 0)
    {
        throw std::runtime_error("Could not import Waveform data");
    }

    if (!file.read(buffer, 4))
    {
        throw std::runtime_error("Could not import Waveform data");
    }

    if (!file.read(buffer, 4) || strncmp(buffer, "WAVE", 4) != 0)
    {
        throw std::runtime_error("Could not import Waveform data");
    }


    while (!file.read(buffer, 4) || strncmp(buffer, "fmt", 3) != 0)
    {
        if (file.read(buffer, 4)) {
            uint32_t size;
            memcpy(&size, buffer, 4);
            file.ignore(size);
            continue;
        }
        else {
            throw std::runtime_error("Could not import Waveform data");
        }
    }

    if (!file.read(buffer, 4))
    {
        throw std::runtime_error("Could not import Waveform data");
    }
    uint32_t fmt_size;
    memcpy(&fmt_size, buffer, 4);

    if (!file.read(buffer, 2))
    {
        throw std::runtime_error("Could not import Waveform data");
    }

    if (!file.read(buffer, 2))
    {
        throw std::runtime_error("Could not import Waveform data");
    }
    memcpy(&(object.descriptor.channels), buffer, 2);


    if (!file.read(buffer, 4))
    {
        throw std::runtime_error("Could not import Waveform data");
    }
    memcpy(&(object.descriptor.sample_rate), buffer, 4);

    if (!file.read(buffer, 4))
    {
        throw std::runtime_error("Could not import Waveform data");
    }

    if (!file.read(buffer, 2))
    {
        throw std::runtime_error("Could not import Waveform data");
    }

    if (!file.read(buffer, 2))
    {
        throw std::runtime_error("Could not import Waveform data");
    }
    memcpy(&(object.descriptor.bits_per_sample), buffer, 2);

    file.ignore(fmt_size - 16);

    while (!file.read(buffer, 4) || strncmp(buffer, "data", 4) != 0)
    {
        if (file.read(buffer, 4)) {
            uint32_t size;
            memcpy(&size, buffer, 4);
            file.ignore(size);
            continue;
        }
        else {
            throw std::runtime_error("Could not import Waveform data");
        }
    }

    if (!file.read(buffer, 4))
    {
        throw std::runtime_error("Could not import Waveform data");
    }
    memcpy(&(object.descriptor.buffer_size), buffer, 4);

    char* data = new char[object.descriptor.buffer_size];
    file.read(data, object.descriptor.buffer_size);

    if(file.fail()) throw std::runtime_error("Could not import Waveform data");

    ALenum format;

    if (object.descriptor.channels == 1 && object.descriptor.bits_per_sample == 8)
        format = AL_FORMAT_MONO8;
    else if (object.descriptor.channels == 1 && object.descriptor.bits_per_sample == 16)
        format = AL_FORMAT_MONO16;
    else if (object.descriptor.channels == 2 && object.descriptor.bits_per_sample == 8)
        format = AL_FORMAT_STEREO8;
    else if (object.descriptor.channels == 2 && object.descriptor.bits_per_sample == 16)
        format = AL_FORMAT_STEREO16;
    else {
        throw std::runtime_error("Sound format not supported!");
    }

    unsigned int audio_buffer;
    alGenBuffers(1, &audio_buffer);
    alBufferData(audio_buffer, format, data, object.descriptor.buffer_size, object.descriptor.sample_rate);

    object.buffer_id = audio_buffer;
    object.state = AudioObjectState::LOADED;

    delete[] data;
	file.close();
}

AudioSystem::AudioSystem()
{
	ALCdevice* dev = alcOpenDevice(nullptr);
	if (!dev) {
		throw std::runtime_error("No sound device was found");
	}
	m_Context = alcCreateContext(dev, nullptr);
	alcMakeContextCurrent(m_Context);
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		throw std::runtime_error((std::string)"OpenAL error occured during context creation: \n" + std::to_string((int)error));
	}

    unsigned int buffer;
    alGenBuffers(1, &buffer);
    unsigned int data = 0;
    alBufferData(1, AL_FORMAT_MONO8, &data, 0, 48000);
	default_audio_object = std::shared_ptr<AudioStandardObject>(new AudioStandardObject(), [](AudioStandardObject* ptr) {
        ALuint buffer_id = ptr->GetBufferId();
        alDeleteBuffers(1, &buffer_id);
        delete ptr;
        });
    std::dynamic_pointer_cast<AudioStandardObject>(default_audio_object)->buffer_id = buffer;
    default_audio_object->state = AudioObjectState::LOADED;


}

AudioSystem::~AudioSystem()
{
    std::lock_guard<std::mutex> lock(audio_object_mutex);
    audio_object_map.clear();
    default_audio_object.reset();
	alcMakeContextCurrent(nullptr);
	ALCdevice* dev = alcGetContextsDevice(m_Context);
	alcDestroyContext(m_Context);
	alcCloseDevice(dev);
}
