#pragma once
#include <Renderer/TextRenderer.h>
#include <string>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <glm/glm.hpp>
#include <Renderer/MeshManager.h>
#include <Renderer/RenderResource.h>

class UITextComponent {
	RUNTIME_TAG("UITextComponent")
public:
	UITextComponent(const std::string& text = "Text", std::shared_ptr<FontObject> in_font = nullptr) : font(nullptr), text(text) {
		if (in_font) {
			font = in_font;
		}
	}

	UITextComponent(const UITextComponent& other) : font(other.font), text(other.text), size(other.size), offset(other.offset), text_quads(nullptr) {

	}

	const std::string& GetText() const {
		return text;
	}

	void SetText(const std::string& in_text) {
		text = in_text;
		dirty = true;
	}

	const glm::vec2 GetOffset() const {
		return offset;
	}

	const glm::vec2 GetSize() const {
		return size;
	}

	void SetOffset(const glm::vec2 in_offset) {
		offset = in_offset;
	}

	void SetSize(const glm::vec2 in_size) {
		size = in_size;
	}

	const std::shared_ptr<FontObject> GetFontObject() const {
		return font;
	}

	void SetFontObject(std::shared_ptr<FontObject> in_font) {
		font = in_font;
		dirty = true;
	}


private:
	friend class TextRenderer;
	std::string text = "Text";
	std::shared_ptr<FontObject> font;
	std::shared_ptr<RenderBufferResource> text_quads = nullptr;
	glm::vec2 size = {-1,-1};
	glm::vec2 offset = {0,0};
	bool dirty = false;
};


template<typename T>
class ComponentInitProxy;

template<>
class ComponentInitProxy<UITextComponent> {
public:
	static constexpr bool can_copy = true;

};