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

	static GLenum TextureFormatToGLInternalformat(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return GL_RGBA;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return GL_RGB;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureFormatToGLDataType(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return GL_UNSIGNED_BYTE;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return GL_UNSIGNED_BYTE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static int TextureFormatToTexelSize(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return 4*sizeof(unsigned char);
		case TextureFormat::RGB_UNSIGNED_CHAR:				return 3*sizeof(unsigned char);
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureAddressModeToGLWrappingMode(TextureAddressMode mode) {
		switch (mode) {
		case TextureAddressMode::BORDER:				return GL_CLAMP_TO_BORDER;
		case TextureAddressMode::CLAMP:					return GL_CLAMP_TO_EDGE;
		case TextureAddressMode::MIRROR:				return GL_MIRRORED_REPEAT;
		case TextureAddressMode::WRAP:					return GL_REPEAT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureFilterToGLFilter(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::LINEAR_MIN_MAG_MIP:				return GL_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_POINT_MIP:		return GL_LINEAR_MIPMAP_NEAREST;
		case TextureFilter::POINT_MIN_MAG_LINEAR_MIP:		return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFilter::POINT_MIN_MAG_MIP:				return GL_NEAREST;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

};

