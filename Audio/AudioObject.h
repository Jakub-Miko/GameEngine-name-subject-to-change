#pragma once

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

protected:
	friend AudioSystem;
	AudioObjectState state;
};


class AudioStandardObject : public AudioObject {
public:

	virtual AudioObjectType GetAudioObjectType() const override {
		return AudioObjectType::STANDARD;
	}

	unsigned int GetBufferId() const {
		return buffer_id;
	}

private:
	friend AudioSystem;
	unsigned int buffer_id = -1;
};

class AudioStreamObject : public AudioObject {
public:

	virtual AudioObjectType GetAudioObjectType() const override {
		return AudioObjectType::STREAM;
	}

};