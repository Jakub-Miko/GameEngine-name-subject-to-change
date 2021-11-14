#pragma once
#include <Core/ExecutableCommand.h>

class OpenGLPresent : public ExecutableCommand {
public:
	virtual void Execute() override;
	virtual ~OpenGLPresent() {}
};