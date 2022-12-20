#pragma once 

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

private:
	~AudioSystem();
	AudioSystem();
	static AudioSystem* instance;
	ALCcontext* m_Context;
};