#pragma once 
#include <cstdint>

class RenderFence {
public:

	static RenderFence* CreateFence(uint32_t initial_value = 0);
	virtual bool WaitForValue(int desired_value) = 0;
	virtual void Wait() = 0;
	virtual int GetValue() = 0;

	virtual ~RenderFence() {};

};