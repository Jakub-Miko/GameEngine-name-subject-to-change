#pragma once

class ExecutableCommand {
public:
	virtual void Execute() = 0;
	virtual ~ExecutableCommand() {}
};