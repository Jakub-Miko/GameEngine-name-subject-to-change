#pragma once 
#include <Audio/AudioSystem.h>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>
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

	AudioComponent(const AudioComponent& other) {
		audio_source = AudioSystem::Get()->CreateAudioSource();
		audio_source->SetGain(other.audio_source->GetGain());
		audio_source->SetLooping(other.audio_source->GetLooping());
		audio_source->SetMaxDistance(other.audio_source->GetMaxDistance());
		audio_source->SetPitch(other.audio_source->GetPitch());
		audio_source->SetRolloffFactor(other.audio_source->GetRolloffFactor());
		audio_source->SetSourcePosition(other.audio_source->GetSourcePosition());
		audio_source->SetVelocity(other.audio_source->GetVelocity());
		default_path = other.default_path;
		requested_path = other.requested_path;
		if (other.state == AudioComponentState::PLAYING || other.state == AudioComponentState::PLAY_REQUEST) {
			state = AudioComponentState::PLAY_REQUEST;
		}
		else {
			state = AudioComponentState::STOPPED;
		}
	};

	AudioComponent(std::shared_ptr<AudioSource> audio_source) : audio_source(audio_source) {}

	AudioComponentState GetAudioState() const {
		return state;
	}

	std::shared_ptr<AudioSource> GetAudioSource() const {
		return audio_source;
	}

	void SetDefaultAudioPath(const std::string& audio_path) {
		default_path = audio_path;
	}

	const std::string& GetDefaultAudioPath() const {
		return default_path;
	}

	void PlayAudio(const std::string& audio_path = "") {
		if (audio_path.empty()) {
			if (default_path.empty()) {
				requested_path = default_path;
				state = AudioComponentState::STOPPED;
				audio_source->Stop();
				return;
			}
			else {
				requested_path = default_path;
			}
		}
		else {
			requested_path = audio_path;
		}
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
	std::string default_path = "";
};

template<>
class ComponentInitProxy<AudioComponent> {
public:
	static constexpr bool can_copy = true;

};

#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const AudioComponent& p) {
	j["gain"] = p.GetAudioSource()->GetGain();
	j["pitch"] = p.GetAudioSource()->GetPitch();
	j["max_distance"] = p.GetAudioSource()->GetMaxDistance();
	j["rolloff_factor"] = p.GetAudioSource()->GetRolloffFactor();
	j["looping"] = p.GetAudioSource()->GetLooping();
	j["default"] = p.GetDefaultAudioPath();
}

inline void from_json(const nlohmann::json& j, AudioComponent& p) {
	p.GetAudioSource()->SetGain(j["gain"].get<float>());
	p.GetAudioSource()->SetPitch(j["pitch"].get<float>());
	p.GetAudioSource()->SetMaxDistance(j["max_distance"].get<float>());
	p.GetAudioSource()->SetRolloffFactor(j["rolloff_factor"].get<float>());
	p.GetAudioSource()->SetLooping(j["looping"].get<bool>());
	if (j.contains("default") && !j["default"].get<std::string>().empty()) {
		p.SetDefaultAudioPath(j["default"].get<std::string>());
		p.PlayAudio();
	}
}

#pragma endregion
