#pragma once 
#include <Audio/AudioSystem.h>
#include <Core/RuntimeTag.h>
#include <string>

enum class AudioComponentState : char {
	STOPPED = 0, PLAY_REQUEST = 1, PLAYING = 2 
};

class AudioComponent {
	RUNTIME_TAG("AudioComponent");
public:
	AudioComponent() {
		audio_source = AudioSystem::Get()->CreateAudioSource();
	};
	AudioComponent(std::shared_ptr<AudioSource> audio_source) : audio_source(audio_source) {}

	AudioComponentState GetAudioState() const {
		return state;
	}

	std::shared_ptr<AudioSource> GetAudioSource() const {
		return audio_source;
	}

	void PlayAudio(const std::string& audio_path) {
		requested_path = audio_path;
		state = AudioComponentState::PLAY_REQUEST;
	}

	const std::string& GetRequestedPath() const {
		return requested_path;
	}

private:
	friend class AudioSystem;
	std::shared_ptr<AudioSource> audio_source = nullptr;
	AudioComponentState state = AudioComponentState::STOPPED;
	std::string requested_path = "";
};