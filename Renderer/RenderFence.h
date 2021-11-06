#pragma once 

class RenderFence {
public:

	static RenderFence* CreateFence();
	virtual bool WaitForValue(int desired_value) = 0;
	virtual void Wait() = 0;
	virtual int GetValue() = 0;

	virtual ~RenderFence() {};

};