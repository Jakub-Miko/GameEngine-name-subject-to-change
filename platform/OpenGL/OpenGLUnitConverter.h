#pragma once
#include <GL/glew.h>
#include <Core/UnitConverter.h>
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
		case RenderPrimitiveType::VEC2:				return GL_FLOAT;
		case RenderPrimitiveType::VEC3:				return GL_FLOAT;
		case RenderPrimitiveType::VEC4:				return GL_FLOAT;
		case RenderPrimitiveType::MAT3:				return GL_FLOAT;
		case RenderPrimitiveType::MAT4:				return GL_FLOAT;
		default: 
			throw std::runtime_error("Conversion failed");
		}
	}

	static bool IsPrimitiveInteger(RenderPrimitiveType type) {
		switch (type) {
		case RenderPrimitiveType::CHAR:				return true;
		case RenderPrimitiveType::FLOAT:			return false;
		case RenderPrimitiveType::INT:				return true;
		case RenderPrimitiveType::UNSIGNED_CHAR:	return true;
		case RenderPrimitiveType::UNSIGNED_INT:		return true;
		case RenderPrimitiveType::VEC2:				return false;
		case RenderPrimitiveType::VEC3:				return false;
		case RenderPrimitiveType::VEC4:				return false;
		case RenderPrimitiveType::MAT3:				return false;
		case RenderPrimitiveType::MAT4:				return false;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static int PrimitiveSize(RenderPrimitiveType type) {
		return UnitConverter::PrimitiveSize(type);
	}

	static GLenum TextureFormatToGLInternalformat(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return GL_RGBA8;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return GL_RGB8;
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return GL_DEPTH24_STENCIL8;
		case TextureFormat::RGB_32FLOAT:					return GL_RGBA32F;
		case TextureFormat::RGBA_32FLOAT:					return GL_RGBA32F;
		case TextureFormat::R_UNSIGNED_INT:					return GL_R32UI;
		case TextureFormat::R_UNSIGNED_CHAR:                return GL_RED;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum DepthComparisonModeToGLCompareFunc(DepthComparisonMode mode) {
		switch (mode) {
		case DepthComparisonMode::ALWAYS:					return GL_ALWAYS;
		case DepthComparisonMode::EQUAL:					return GL_EQUAL;
		case DepthComparisonMode::GREATER:				return GL_GREATER;
		case DepthComparisonMode::GREATER_EQUAL:			return GL_GEQUAL;
		case DepthComparisonMode::LESS:					return GL_LESS;
		case DepthComparisonMode::LESS_EQUAL:				return GL_LEQUAL;
		case DepthComparisonMode::NEVER:					return GL_NEVER;
		case DepthComparisonMode::NOT_EQUAL:				return GL_NOTEQUAL;
		case DepthComparisonMode::DISABLED:					return GL_NEVER;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum DepthComparisonModeToGLCompareMode(DepthComparisonMode mode) {
		switch (mode) {
		case DepthComparisonMode::ALWAYS:					return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::EQUAL:					return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::GREATER:					return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::GREATER_EQUAL:			return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::LESS:						return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::LESS_EQUAL:				return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::NEVER:					return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::NOT_EQUAL:				return GL_COMPARE_REF_TO_TEXTURE;
		case DepthComparisonMode::DISABLED:					return GL_NONE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum BlendEquationTOGLenum(BlendEquation type) {
		switch (type) {
		case BlendEquation::ADD:				return GL_FUNC_ADD;
		case BlendEquation::MAX:				return GL_MAX;
		case BlendEquation::MIN:				return GL_MIN;
		case BlendEquation::REVERSE_SUBTRACT:	return GL_FUNC_REVERSE_SUBTRACT;
		case BlendEquation::SUBTRACT:			return GL_FUNC_SUBTRACT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum CullModeTOGLenum(CullMode type) {
		switch (type) {
		case CullMode::BACK:				return GL_BACK;
		case CullMode::FRONT:				return GL_FRONT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureFormatToGLFormat(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return GL_RGBA;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return GL_RGB;
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return GL_DEPTH_STENCIL;
		case TextureFormat::RGB_32FLOAT:					return GL_RGB;
		case TextureFormat::RGBA_32FLOAT:					return GL_RGBA;
		case TextureFormat::R_UNSIGNED_INT:					return GL_RED_INTEGER;
		case TextureFormat::R_UNSIGNED_CHAR:                return GL_RED;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureFormatToGLDataType(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return GL_UNSIGNED_BYTE;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return GL_UNSIGNED_BYTE;
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return GL_UNSIGNED_INT_24_8;
		case TextureFormat::RGB_32FLOAT:					return GL_FLOAT;
		case TextureFormat::RGBA_32FLOAT:					return GL_FLOAT;
		case TextureFormat::R_UNSIGNED_INT:					return GL_UNSIGNED_INT;
		case TextureFormat::R_UNSIGNED_CHAR:                return GL_UNSIGNED_BYTE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static int TextureFormatToTexelSize(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return 4*sizeof(unsigned char);
		case TextureFormat::RGB_UNSIGNED_CHAR:				return 3*sizeof(unsigned char);
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return 4*sizeof(unsigned char);
		case TextureFormat::RGB_32FLOAT:					return 3*sizeof(float);
		case TextureFormat::RGBA_32FLOAT:					return 4*sizeof(float);
		case TextureFormat::R_UNSIGNED_INT:					return sizeof(uint32_t);
		case TextureFormat::R_UNSIGNED_CHAR:                return sizeof(char);
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

	static GLenum TextureFilterToGLMinFilter(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::LINEAR_MIN_MAG:					return GL_LINEAR;
		case TextureFilter::POINT_MIN_MAG:					return GL_NEAREST;
		case TextureFilter::LINEAR_MIN_MAG_MIP:				return GL_LINEAR_MIPMAP_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_POINT_MIP:		return GL_LINEAR_MIPMAP_NEAREST;
		case TextureFilter::POINT_MIN_MAG_LINEAR_MIP:		return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFilter::POINT_MIN_MAG_MIP:				return GL_NEAREST_MIPMAP_NEAREST;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum DepthFunctionToGLenum(DepthFunction filter) {
		switch (filter) {
		case DepthFunction::ALWAYS:					return GL_ALWAYS;
		case DepthFunction::EQUAL:					return GL_EQUAL;
		case DepthFunction::GREATER:				return GL_GREATER;
		case DepthFunction::GREATER_EQUAL:			return GL_GEQUAL;
		case DepthFunction::LESS:					return GL_LESS;
		case DepthFunction::LESS_EQUAL:				return GL_LEQUAL;
		case DepthFunction::NEVER:					return GL_NEVER;
		case DepthFunction::NOT_EQUAL:				return GL_NOTEQUAL;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum TextureFilterToGLMagFilter(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::LINEAR_MIN_MAG:					return GL_LINEAR;
		case TextureFilter::POINT_MIN_MAG:					return GL_NEAREST;
		case TextureFilter::LINEAR_MIN_MAG_MIP:				return GL_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_POINT_MIP:		return GL_LINEAR;
		case TextureFilter::POINT_MIN_MAG_LINEAR_MIP:		return GL_NEAREST;
		case TextureFilter::POINT_MIN_MAG_MIP:				return GL_NEAREST;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum PipelineFlagsToGLenum(PipelineFlags flags) {
		switch (flags) {
			case PipelineFlags::DEFAULT:				return GL_NONE;
			case PipelineFlags::ENABLE_DEPTH_TEST:		return GL_DEPTH_TEST;
			case PipelineFlags::ENABLE_SCISSOR_TEST:	return GL_SCISSOR_TEST;
			case PipelineFlags::ENABLE_STENCIL_TEST:	return GL_STENCIL_TEST;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum PrimitivePolygonRenderModetoGLenum(PrimitivePolygonRenderMode mode) {
		switch (mode) {
		case PrimitivePolygonRenderMode::DEFAULT:				return GL_FILL;
		case PrimitivePolygonRenderMode::WIREFRAME:				return GL_LINE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum BlendFunctiontoGLenum(BlendFunction mode) {
		switch (mode) {
		case BlendFunction::ONE:					return GL_ONE;
		case BlendFunction::ZERO:					return GL_ZERO;
		case BlendFunction::DST_ALPHA:				return GL_DST_ALPHA;
		case BlendFunction::SRC_ALPHA:				return GL_SRC_ALPHA;
		case BlendFunction::ONE_MINUS_DST_ALPHA:	return GL_ONE_MINUS_DST_ALPHA;
		case BlendFunction::ONE_MINUS_SRC_ALPHA:	return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFunction::SRC_COLOR:				return GL_SRC_COLOR;
		case BlendFunction::DST_COLOR:				return GL_DST_COLOR;
		case BlendFunction::ONE_MINUS_SRC_COLOR:	return GL_ONE_MINUS_SRC_COLOR;
		case BlendFunction::ONE_MINUS_DST_COLOR:	return GL_ONE_MINUS_DST_COLOR;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static GLenum CubemapFacetoGLenum(CubemapFace face) {
		switch (face) {
		case CubemapFace::NEGATIVE_X:					return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case CubemapFace::NEGATIVE_Y:					return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case CubemapFace::NEGATIVE_Z:					return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		case CubemapFace::POSITIVE_X:					return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case CubemapFace::POSITIVE_Y:					return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case CubemapFace::POSITIVE_Z:					return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

};

