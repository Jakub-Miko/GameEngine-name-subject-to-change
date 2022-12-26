#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include "AudioSource.h"
#include <AL/al.h>
#include <glm/gtc/type_ptr.hpp>

void AudioSource::SetSourcePosition(const glm::vec3& position)
{
	alSourcefv(source_id, AL_POSITION, glm::value_ptr(position));
}

void AudioSource::SetGain(float gain)
{
	alSourcef(source_id, AL_GAIN, gain);
}

void AudioSource::SetPitch(float pitch)
{
	alSourcef(source_id, AL_PITCH, pitch);
}

void AudioSource::SetMaxDistance(float max_distance)
{
	alSourcef(source_id, AL_MAX_DISTANCE, max_distance);
}

void AudioSource::SetRolloffFactor(float rolloff_factor)
{
	alSourcef(source_id, AL_ROLLOFF_FACTOR, rolloff_factor);
}

void AudioSource::SetLooping(bool should_loop)
{
	if (should_loop) {
		alSourcei(source_id, AL_LOOPING, AL_TRUE);
	}
	else {
		alSourcei(source_id, AL_LOOPING, AL_FALSE);
	}
}

void AudioSource::SetVelocity(const glm::vec3& velocity)
{
	alSourcefv(source_id, AL_VELOCITY, glm::value_ptr(velocity));
}

glm::vec3 AudioSource::GetSourcePosition(const glm::vec3& position)
{
	glm::vec3 pos;
	alGetSourcefv(source_id, AL_POSITION, glm::value_ptr(pos));
	return pos;
}

float AudioSource::GetGain()
{
	float gain;
	alGetSourcef(source_id, AL_GAIN, &gain);
	return gain;
}

float AudioSource::GetPitch()
{
	float pitch;
	alGetSourcef(source_id, AL_PITCH, &pitch);
	return pitch;
}

float AudioSource::GetMaxDistance()
{
	float max_distance;
	alGetSourcef(source_id, AL_MAX_DISTANCE, &max_distance);
	return max_distance;
}

float AudioSource::GetRolloffFactor()
{
	float rolloff_factor;
	alGetSourcef(source_id, AL_ROLLOFF_FACTOR, &rolloff_factor);
	return rolloff_factor;
}

bool AudioSource::GetLooping()
{
	int looping;
	alGetSourcei(source_id, AL_LOOPING, &looping);
	if (looping == AL_TRUE) {
		return true;
	}
	else {
		return false;
	}
}

glm::vec3 AudioSource::GetVelocity()
{
	glm::vec3 vel;
	alGetSourcefv(source_id, AL_VELOCITY, glm::value_ptr(vel));
	return vel;
}

void AudioSource::Play()
{
	alSourcePlay(source_id);
}

void AudioSource::Pause()
{
	alSourcePause(source_id);
}

void AudioSource::Stop()
{
	alSourceStop(source_id);
}

void AudioSource::SetAudioObject(std::shared_ptr<AudioObject> audio_object_in)
{
	Stop();
	audio_object = audio_object_in;
	alSourcei(source_id, AL_BUFFER, std::dynamic_pointer_cast<AudioStandardObject>(audio_object)->GetBufferId());
}

void AudioSource::ResetAudioObject()
{
	if (audio_object) {
		alSourcei(source_id, AL_BUFFER, std::dynamic_pointer_cast<AudioStandardObject>(audio_object)->GetBufferId());
	}
}

AudioSourceState AudioSource::GetSourceState() const
{
	ALint state;
	alGetSourcei(source_id, AL_SOURCE_STATE, &state);
	switch (state)
	{
	case AL_PLAYING:
		return AudioSourceState::PLAYING;
		break;
	case AL_STOPPED:
		return AudioSourceState::STOPPPED;
		break;
	default:
		return AudioSourceState::ERROR;
		break;
	}
}

std::shared_ptr<AudioObject> AudioSource::GetAudioObject() const
{
	return audio_object;
}