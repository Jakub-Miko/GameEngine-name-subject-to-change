#include "AudioSystem.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdexcept>
#include <string>

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
	
}

AudioSystem::~AudioSystem()
{
	alcMakeContextCurrent(nullptr);
	ALCdevice* dev = alcGetContextsDevice(m_Context);
	alcDestroyContext(m_Context);
	alcCloseDevice(dev);
}
