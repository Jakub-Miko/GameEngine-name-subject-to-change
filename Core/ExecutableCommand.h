#pragma once

class ExecutableCommand {
public:
	virtual void Execute() = 0;
	virtual ~ExecutableCommand() {}
};

template<typename F>
class ExecutableCommandAdapter : public ExecutableCommand {
public:
	ExecutableCommandAdapter(F func) : func(func) {}
	
	virtual void Execute() override {
		func();
	}

	virtual ~ExecutableCommandAdapter() override {}
private:
	F func;
};