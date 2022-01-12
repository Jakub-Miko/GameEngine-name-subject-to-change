#pragma once
#include <GL/glew.h>
#include <Renderer/RendererDefines.h>
#include <stdexcept>

class OpenGLUnitConverter {
public:

	static GLenum PrimitiveToGL(RenderPrimitiveType type) {
		switch (type) {
		case RenderPrimitiveType::CHAR:				return GL_BYTE;
		case RenderPrimitiveType::FLOAT:			return GL_FLOAT;
		case RenderPrimitiveType::INT:				return GL_INT;
		case RenderPrimitiveType::UNSIGNED_CHAR:	return GL_UNSIGNED_BYTE;
		case RenderPrimitiveType::UNSIGNED_INT:		return GL_UNSIGNED_INT;
		default: 
			throw std::runtime_error("Conversion failed");
		}
	}

	static int PrimitiveSize(RenderPrimitiveType type) {
		switch (type) {
		case RenderPrimitiveType::CHAR:				return sizeof(char);
		case RenderPrimitiveType::FLOAT:			return sizeof(float);
		case RenderPrimitiveType::INT:				return sizeof(int);
		case RenderPrimitiveType::UNSIGNED_CHAR:	return sizeof(unsigned char);
		case RenderPrimitiveType::UNSIGNED_INT:		return sizeof(unsigned int);
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

};

