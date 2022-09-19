#include "EntityParser.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <json.hpp>
#include <sstream>

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
	auto end_scr = script.find("@", end + 1);
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
		result.properties = parser["Properties"];
	}

	if (parser.contains("Children")) {
		
		for (auto prop : parser.at("Children")) {
			result.children.push_back(prop.get<std::string>());
		}
	}

	if (parser.contains("Components")) {

		result.component_json = parser["Components"].dump();
	}

	auto construct = script.find("@Entity:Construction_Script", 0);
	if (construct != script.npos) {
		construct += strlen("@Entity:Construction_Script");
		auto end = script.find("@", construct);
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