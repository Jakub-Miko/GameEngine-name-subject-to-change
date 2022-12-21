#pragma once 
#include <Promise.h>
#include <unordered_map>
#include <mutex>
#include <deque>
#include <string>
#include <memory>
#include <Audio/AudioObject.h>

struct ALCcontext;

class AudioSystem {
public:
	AudioSystem(const AudioSystem& ref) = delete;
	AudioSystem(AudioSystem&& ref) = delete;
	AudioSystem& operator=(const AudioSystem& ref) = delete;
	AudioSystem& operator=(AudioSystem&& ref) = delete;

	static void Init();
	static void Shutdown();
	static AudioSystem* Get();

	std::shared_ptr<AudioObject> GetAudioObject(const std::string& path);
	std::shared_ptr<AudioObject> GetDefaultAudioObject() const {
		return default_audio_object;
	}

	void UpdateLoadedSounds();

private:
	~AudioSystem();
	AudioSystem();
	static AudioSystem* instance;

	AudioStandardObject LoadAudioFromFileImpl(const std::string& path);
	void ImportWaveFormAudioObject(AudioStandardObject& object, const std::string& path);


	struct audio_load_future {
		std::shared_ptr<AudioStandardObject> audio;
		Future<AudioStandardObject> future;
		bool destroyed = false;
		std::string path;
	};

	ALCcontext* m_Context;
	std::shared_ptr<AudioObject> default_audio_object;
	std::unordered_map<std::string, std::shared_ptr<AudioObject>> audio_object_map;
	std::mutex audio_object_mutex;
	std::mutex audio_Load_queue_mutex;
	std::deque<audio_load_future> audio_Load_queue;

};