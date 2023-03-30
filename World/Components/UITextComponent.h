#pragma once
#include <FileManager.h>
#include <Renderer/TextRenderer.h>
#include <string>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <glm/glm.hpp>
#include <Renderer/MeshManager.h>
#include <Renderer/RenderResource.h>

class World;

class UITextComponent {
	RUNTIME_TAG("UITextComponent")
public:
	UITextComponent(const std::string& text = "Text", std::shared_ptr<FontObject> in_font = nullptr) : font(nullptr), text(text) {
		if (in_font) {
			font = in_font;
		}
	}

	UITextComponent(const UITextComponent& other) : font(other.font), text(other.text), text_quads(nullptr) {

	}

	const std::string& GetText() const {
		return text;
	}

	void SetText(const std::string& in_text) {
		text = in_text;
		dirty = true;
	}

	const std::shared_ptr<FontObject> GetFontObject() const {
		return font;
	}

	void SetFontObject(std::shared_ptr<FontObject> in_font) {
		font = in_font;
		dirty = true;
	}

	float GetFontSize() const {
		return font_size;
	}

	void SetFontSize(float in_font_size) {
		font_size = in_font_size;
	}

private:
	friend void from_json(const nlohmann::json& j, UITextComponent& p);
	friend class TextRenderer;
	std::string text = "Text";
	std::shared_ptr<FontObject> font;
	std::shared_ptr<RenderBufferResource> text_quads = nullptr;
	float font_size = 12.0f;
	bool dirty = false;
};


template<typename T>
class ComponentInitProxy;

template<>
class ComponentInitProxy<UITextComponent> {
public:
	static constexpr bool can_copy = true;

	static void OnCreate(World& world, Entity entity);

	static void OnDestroy(World& world, Entity entity);

};

#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const UITextComponent& p) {
	j["text"] = p.GetText();
	if (p.GetFontObject()) {
		j["font"] = p.GetFontObject()->GetFilePath();
	}
	j["font_size"] = p.GetFontSize();

}

inline void from_json(const nlohmann::json& j, UITextComponent& p) {
	if (j.contains("font")) {
		p.font = TextRenderer::Get()->GetFontObject(FileManager::Get()->GetPath(j["font"].get<std::string>()));
	}
	p.text = j["text"].get<std::string>();
	p.font_size = j["font_size"].get<int>();
	p.dirty = true;
}

#pragma endregion