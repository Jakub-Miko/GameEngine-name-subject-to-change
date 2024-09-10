#pragma once 
#include <Audio/AudioSystem.h>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>
#include <string>

/**
 * @brief Represents the state of the AudioComponent
 */
enum class AudioComponentState : char {
	STOPPED = 0, ///< No audio is playing
	PLAY_REQUEST = 1, ///< Audio should play, but the AudioUpdateSystem has not yet executed or the audio is not loaded yet
	PLAYING = 2 ///< Audio is playing
};

/**
 * @brief Represent the Entities ability play and emit sound
 * 
 * @see @ref AudioSystem
 */
class AudioComponent {
	RUNTIME_TAG("AudioComponent");
public:
	/**
	 * @brief Default constructor. Creates as new AudioSource.
	 */
	AudioComponent() {
		audio_source = AudioSystem::Get()->CreateAudioSource();
	};

	/**
	 * @brief Copy constructor
	 * @param other instance of AudioComponent to copy 
	 */
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

	/**
	 * @brief Constructor which uses already existing AudioSource.
	 * @param audio_source AudioSource to use
	 */
	AudioComponent(std::shared_ptr<AudioSource> audio_source) : audio_source(audio_source) {}

	/**
	 * @brief Gets the components AudioComponentState
	 * @return AudioComponentState of this component
	 */
	AudioComponentState GetAudioState() const {
		return state;
	}

	/**
	 * @brief Gets the components AudioSource instance
	 * @return AudioSource instance used by this component
	 */
	std::shared_ptr<AudioSource> GetAudioSource() const {
		return audio_source;
	}

	/**
	 * @brief Sets the path of the default audio to load into this component after deserialization.
	 * @param audio_path  the path to the default audio file
	 */
	void SetDefaultAudioPath(const std::string& audio_path) {
		default_path = audio_path;
	}

	/**
	 * @brief Gets the default audio path
	 * @return The default audio path used by this component.
	 * @see @ref SetDefaultAudioPath
	 */
	const std::string& GetDefaultAudioPath() const {
		return default_path;
	}

	/**
	 * @brief Play an audio file through this Component
	 * @param audio_path The path to the audio to play
	 */
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

	/**
	 * @brief Gets the path of the audio that has been requested to play or is playing.
	 * @return the path of the playing audio.
	 */
	const std::string& GetRequestedPath() const {
		return requested_path;
	}

private:
	friend class AudioSystem;
	std::shared_ptr<AudioSource> audio_source = nullptr; ///< An Audio source handled by the @ref AudioSystem
	AudioComponentState state = AudioComponentState::STOPPED; ///< The playing state of this component
	std::string requested_path = ""; ///< The path of the audio that has been requested to play or is playing.
	std::string default_path = ""; ///< The path of the audio file which is serialized when an entity is a part of a scene of a prefab.
};


/**
 * @brief A specialization of the ComponentInitProxy specifying the properties of this component
 */
template<>
class ComponentInitProxy<AudioComponent> {
public:
	static constexpr bool can_copy = true; ///< Declares this component as copyable
};

#pragma region Json_Serialization

/**
 * @brief Handles serializing this component to a JSON object used when saving to a file.
 * @param j the output json object 
 * @param p the instance of AudioComponent to serialize
 */
inline void to_json(nlohmann::json& j, const AudioComponent& p) {
	j["gain"] = p.GetAudioSource()->GetGain();
	j["pitch"] = p.GetAudioSource()->GetPitch();
	j["max_distance"] = p.GetAudioSource()->GetMaxDistance();
	j["rolloff_factor"] = p.GetAudioSource()->GetRolloffFactor();
	j["looping"] = p.GetAudioSource()->GetLooping();
	j["default"] = p.GetDefaultAudioPath();
}

/**
 * @brief Initializes an instance of AudioComponent from a JSON object 
 * @param j the json object containing the AudioComponent serialized by @ref to_json(nlohmann::json&, const AudioComponent&) "to_json"
 * @param p the AudioComponent instance to initialize
 */
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
