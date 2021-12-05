#include "EntityParser.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <json.hpp>
#include <sstream>

template<typename T>
class PropertyParser {
public:

	static T Parse(const nlohmann::json::value_type& json) {
		return json["Value"].get<T>();
	}

};

template<>
class PropertyParser<glm::vec2> {
public:

	static glm::vec2 Parse(const nlohmann::json::value_type& json) {
		glm::vec2 value;
		std::stringstream stream(json["Value"].get<std::string>());
		stream >> value.x >> value.y;
		return value;
	}

};

template<>
class PropertyParser<glm::vec3> {
public:

	static glm::vec3 Parse(const nlohmann::json::value_type& json) {
		glm::vec3 value;
		std::stringstream stream(json["Value"].get<std::string>());
		stream >> value.x >> value.y >> value.z;
		return value;
	}

};

template<>
class PropertyParser<glm::vec4> {
public:

	static glm::vec4 Parse(const nlohmann::json::value_type& json) {
		glm::vec4 value;
		std::stringstream stream(json["Value"].get<std::string>());
		stream >> value.x >> value.y >> value.z >> value.w;
		return value;
	}

};



EntityParseResult EntityParser::ParseEntity(const std::string& script)
{
	std::string json_script;
	if (script[0] != '@') {
		throw std::runtime_error("Invalid entity descriptor format");
	}

	size_t bg = 1;
	size_t end = script.find_first_of(" \t\n", bg);

	if (end == script.npos) {
		throw std::runtime_error("Invalid entity descriptor format");
	}

	std::string tag = script.substr(bg, end - bg);

	if (tag != "Entity") {
		throw std::runtime_error("Invalid entity descriptor format");
	}
	auto end_scr = script.find("@Entity", end + 1);
	if (end_scr == script.npos) {
		json_script = script.substr(end + 1, script.npos);
	}
	else {
		json_script = script.substr(end + 1, end_scr - (end + 1));
	}

	std::stringstream json_stream(json_script);
	nlohmann::json parser;
	json_stream >> parser;

	EntityParseResult result;

	
	if (parser.contains("Properties")) {
		for (auto prop : parser.at("Properties")) {
			std::string name = prop["Name"].get<std::string>();
			std::string type_name = prop["Type"].get<std::string>();
			Script_Variant_type val;

			if (type_name == "string") val = PropertyParser<std::string>::Parse(prop);
			else if (type_name == "int") val = PropertyParser<int>::Parse(prop);
			else if (type_name == "float") val = PropertyParser<float>::Parse(prop);
			else if (type_name == "double") val = PropertyParser<double>::Parse(prop);
			else if (type_name == "vec2") val = PropertyParser<glm::vec2>::Parse(prop);
			else if (type_name == "vec3") val = PropertyParser<glm::vec3>::Parse(prop);
			else if (type_name == "vec3") val = PropertyParser<glm::vec4>::Parse(prop);

			result.properties.m_Properties.insert(std::make_pair(name,val));
		}
	}

	if (parser.contains("Children")) {
		
		for (auto prop : parser.at("Children")) {
			result.children.push_back(prop.get<std::string>());
		}
	}

	auto construct = script.find("@Entity:Construction_Script", 0);
	if (construct != script.npos) {
		construct += strlen("@Entity:Construction_Script");
		auto end = script.find("@Entity", construct);
		std::string construction_script;
		if (end == script.npos) {
			construction_script = script.substr(construct, script.npos);
		}
		else {
			construction_script = script.substr(construct, end - construct);
		}
		result.construction_script = construction_script;
	}

	auto inline_script_bg = script.find("@Entity:Inline_Script", 0);
	if (inline_script_bg != script.npos) {
		inline_script_bg += strlen("@Entity:Inline_Script");
		auto end = script.find("@Entity", inline_script_bg);
		std::string inline_script;
		if (end == script.npos) {
			inline_script = script.substr(inline_script_bg, script.npos);
		}
		else {
			inline_script = script.substr(inline_script_bg, end - inline_script_bg);
		}
		result.inline_script = inline_script;
		result.has_inline = true;
	}

	return result;
}