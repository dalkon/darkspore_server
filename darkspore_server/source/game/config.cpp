
// Include
#include "config.h"

#include <vector>
#include <iostream>
#include <pugixml.hpp>

// Game
namespace Game {
	// Config
	std::array<std::string, CONFIG_END> Config::mConfig;

	bool Config::Load(const std::string& path) {
		const auto get_path_value = [](std::string& value) -> std::string& {
			if (value.back() != '/') {
				value.push_back('/');
			}
			return value;
		};

		const auto parse_config_value = [&](pugi::xml_node& node) {
			std::string name = node.attribute("key").value();
			std::string value = node.attribute("value").value();
			if (name == "SKIP_LAUNCHER") {
				mConfig[CONFIG_SKIP_LAUNCHER] = value;
			} else if (name == "VERSION_LOCKED") {
				mConfig[CONFIG_VERSION_LOCKED] = value;
			} else if (name == "SINGLEPLAYER_ONLY") {
				mConfig[CONFIG_SINGLEPLAYER_ONLY] = value;
			} else if (name == "SERVER_HOST") {
				mConfig[CONFIG_SERVER_HOST] = value;
			} else if (name == "STORAGE_PATH") {
				mConfig[CONFIG_STORAGE_PATH] = get_path_value(value);
			} else if (name == "DARKSPORE_INDEX_PAGE_PATH") {
				mConfig[CONFIG_DARKSPORE_INDEX_PAGE_PATH] = value;
			} else if (name == "DARKSPORE_LAUNCHER_NOTES_PATH") {
				mConfig[CONFIG_DARKSPORE_LAUNCHER_NOTES_PATH] = value;
			} else if (name == "DARKSPORE_LAUNCHER_THEMES_PATH") {
				mConfig[CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH] = get_path_value(value);
			} else {
				std::cout << "Game::Config: Unknown config value '" << name << "'" << std::endl;
			}
		};

		mConfig[CONFIG_SKIP_LAUNCHER] = "false";
		mConfig[CONFIG_VERSION_LOCKED] = "false";
		mConfig[CONFIG_SINGLEPLAYER_ONLY] = "true";
		mConfig[CONFIG_SERVER_HOST] = "127.0.0.1";
		mConfig[CONFIG_STORAGE_PATH] = "storage/";
		mConfig[CONFIG_DARKSPORE_INDEX_PAGE_PATH] = "index.html";
		mConfig[CONFIG_DARKSPORE_LAUNCHER_NOTES_PATH] = "bootstrap/launcher/notes.html";
		mConfig[CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH] = "bootstrap/launcher/";

		pugi::xml_document document;
		if (auto parse_result = document.load_file(path.c_str())) {
			for (auto& child : document.child("configs")) {
				parse_config_value(child);
			}
			return true;
		} else {
			return false;
		}
	}

	const std::string& Config::Get(ConfigValue key) {
		return mConfig[key];
	}

	bool Config::GetBool(ConfigValue key) {
		const auto& str = Get(key);
		if (str == "1") {
			return true;
		} else if (_stricmp(str.c_str(), "true") == 0) {
			return true;
		} else {
			return false;
		}
 	}

	void Config::Set(ConfigValue key, const std::string& value) {
		mConfig[key] = value;
	}
}
