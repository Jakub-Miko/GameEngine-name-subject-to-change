#pragma once
#include <stdint.h>
class AudioSystem;

enum class AudioObjectType : char {
	STANDARD = 0, STREAM = 1 
};

enum class AudioObjectState : char {
	UNINITIALIZED = 0, LOADING = 1, LOADED = 2, ERROR = 3
};

class AudioObject {
public:

	virtual AudioObjectType GetAudioObjectType() const = 0;

	AudioObjectState GetAudioObjectState() const {
		return state;
	}

	virtual ~AudioObject() {}

protected:
	friend AudioSystem;
	AudioObjectState state;
};

struct AudioStandardObjectDescriptor {
	int duration = -1;
	int buffer_size = -1;
	int32_t sample_rate = -1;
	uint16_t bits_per_sample = -1;
	uint16_t channels = -1;
};

class AudioStandardObject : public AudioObject {
public:

	virtual AudioObjectType GetAudioObjectType() const override {
		return AudioObjectType::STANDARD;
	}

	unsigned int GetBufferId() const {
		return buffer_id;
	}

	const AudioStandardObjectDescriptor& GetAudioObjectDescriptor() const {
		return descriptor;
	}


private:
	friend AudioSystem;
	AudioStandardObjectDescriptor descriptor;
	unsigned int buffer_id = -1;
};

class AudioStreamObject : public AudioObject {
public:

	virtual AudioObjectType GetAudioObjectType() const override {
		return AudioObjectType::STREAM;
	}

	virtual ~AudioStreamObject() {}
};