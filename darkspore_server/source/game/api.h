
#ifndef _GAME_API_HEADER
#define _GAME_API_HEADER

// Include
#include <rapidjson/document.h>
#include <pugixml.hpp>

// HTTP
namespace HTTP {
	class URI;
	class Response;
}

// Game
namespace Game {
	// API
	class API {
		public:
			API(const std::string& version);

			void setup();

			// default
			void empty_xml_response(HTTP::Response& response);
			void empty_json_response(HTTP::Response& response);

			// dls
			void dls_launcher_setTheme(HTTP::Response& response, const HTTP::URI& uri);
			void dls_launcher_listThemes(HTTP::Response& response, const HTTP::URI& uri);

			// bootstrap
			void bootstrap_config_getConfig(HTTP::Response& response, const HTTP::URI& uri);

			// game
			void game_status_getStatus(HTTP::Response& response, const HTTP::URI& uri);

		private:
			void add_text_node(pugi::xml_node& node, const std::string& name, const std::string& value);

			void add_common_keys(pugi::xml_node& node);
			void add_common_keys(rapidjson::Document& document);
		
		private:
			std::string mVersion;

			uint32_t mPacketId = 0;
	};
}

#endif
