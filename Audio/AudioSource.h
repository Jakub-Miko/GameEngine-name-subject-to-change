#pragma once
#include <Audio/AudioObject.h>
#include <memory>
#include <glm/glm.hpp>

class AudioSystem;

enum class AudioSourceState : char {
	STOPPPED = 0, PLAYING = 1, ERROR = 2
};

class AudioSource {
public:

	void SetSourcePosition(const glm::vec3& position);
	void SetGain(float gain);
	void SetPitch(float pitch);
	void SetMaxDistance(float max_distance);
	void SetRolloffFactor(float rolloff_factor);
	void SetLooping(bool should_loop);
	void SetVelocity(const glm::vec3& velocity);

	glm::vec3 GetSourcePosition();
	float GetGain();
	float GetPitch();
	float GetMaxDistance();
	float GetRolloffFactor();
	bool GetLooping();
	glm::vec3 GetVelocity();

	void Play();
	void Pause();
	void Stop();
	void SetAudioObject(std::shared_ptr<AudioObject> audio_object);
	void ResetAudioObject();
	unsigned int GetSourceId() const {
		return source_id;
	}
	AudioSourceState GetSourceState() const;
	std::shared_ptr<AudioObject> GetAudioObject() const;

private:
	AudioSource() {}
	friend AudioSystem;
	std::shared_ptr<AudioObject> audio_object;
	unsigned int source_id = -1;
};