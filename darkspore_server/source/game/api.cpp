
// Include
#include "api.h"
#include "config.h"

#include "../main.h"

#include "../http/session.h"
#include "../http/router.h"
#include "../http/uri.h"
#include "../http/multipart.h"
#include "../utils/functions.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <boost/beast/version.hpp>

#include <iostream>

/*
	api.account.auth
		response
			timestamp
				{TIMESTAMP}

			account
				tutorial_completed
				chain_progression
				creature_rewards
				current_game_id
				current_playgroup_id
				default_deck_pve_id
				default_deck_pvp_id
				level
				avatar_id
				id
				new_player_inventory
				new_player_progress
				cashout_bonus_time
				star_level
				unlock_catalysts
				unlock_diagonal_catalysts
				unlock_fuel_tanks
				unlock_inventory
				unlock_pve_decks
				unlock_pvp_decks
				unlock_stats
				unlock_inventory_identify
				unlock_editor_flair_slots
				upsell
				xp
				grant_all_access
				cap_level
				cap_progression

			creatures
				id
				name
				png_thumb_url
				noun_id
				version
				gear_score
				item_points

			decks
				name
				category
				id
				slot
				locked
				creatures
					{SAME_AS_CREATURES_BEFORE}

			feed
				items

			settings

			server_tuning
				itemstore_offer_period
				itemstore_current_expiration
				itemstore_cost_multiplier_basic
				itemstore_cost_multiplier_uncommon
				itemstore_cost_multiplier_rare
				itemstore_cost_multiplier_epic
				itemstore_cost_multiplier_unique
				itemstore_cost_multiplier_rareunique
				itemstore_cost_multiplier_epicunique

*/

// XML Writer
struct xml_string_writer : pugi::xml_writer {
	std::string result;

	void write(const void* data, size_t size) override {
		result.append(static_cast<const char*>(data), size);
	}
};

// Game
namespace Game {
	constexpr std::string_view skipLauncherScript = R"(
<!DOCTYPE html PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
	<head>
		<script type="text/javascript">
			window.onload = function() {
				Client.playCurrentApp();
			}
		</script>
	</head>
	<body>
	</body>
</html>
)";

	constexpr std::string_view dlsClientScript = R"(
<script>
	var DLSClient = {};
	DLSClient.getRequest = function(url, callback) {
		var xmlHttp = new XMLHttpRequest(); 
		xmlHttp.onload = function() {
			callback(xmlHttp.responseText);
		}
		xmlHttp.open("GET", url, true);
		xmlHttp.send(null);
	};
	DLSClient.request = function(name, params, callback) {
		DLSClient.getRequest("http://{{host}}/dls/api?method=" + name + (params === undefined ? "" : ("&" + params)), callback);
	};
</script>
)";

	// API
	API::API(const std::string& version) : mVersion(version) {
		// Empty
	}

	void API::setup() {
		const auto& router = Application::GetApp().get_http_server()->get_router();

		// Routing
		router->add("/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [](HTTP::Response& response, HTTP::Request& request) {
			std::cout << "Got API route." << std::endl;
		});

		// DLS
		router->add("/dls/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			auto method = request.uri.parameter("method");
			if (method == "api.launcher.setTheme") {
				dls_launcher_setTheme(response, request.uri);
			} else if (method == "api.launcher.listThemes") {
				dls_launcher_listThemes(response, request.uri);
			} else {
				response.result() = boost::beast::http::status::internal_server_error;
			}
		});

		// Launcher
		router->add("/bootstrap/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			auto version = request.uri.parameter("version");
			auto build = request.uri.parameter("build");
			auto method = request.uri.parameter("method");

			if (version.empty()) { version = "1"; }
			if (build.empty()) { build = "5.3.0.127"; }
			if (method.empty()) { method = "api.config.getConfigs"; }

			if (method.find("api.config.") == 0) {
				if (method == "api.config.getConfigs") {
					bootstrap_config_getConfig(response, request.uri);
				}
			}
		});

		router->add("/bootstrap/launcher/", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			auto version = request.uri.parameter("version");
			if (Config::GetBool(CONFIG_SKIP_LAUNCHER)) {
				response.set(boost::beast::http::field::content_type, "text/html");
				response.body() = skipLauncherScript;
			} else {
				std::string path = Config::Get(CONFIG_STORAGE_PATH) +
					"www/" +
					Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH) +
					"default/index.html";

				std::string client_script(dlsClientScript);
				utils::string_replace(client_script, "{{host}}", Config::Get(CONFIG_SERVER_HOST));

				std::string file_data = utils::get_file_text(path);
				utils::string_replace(file_data, "</head>", client_script + "</head>");

				response.set(boost::beast::http::field::content_type, "text/html");
				response.body() = std::move(file_data);
			}
		});

		router->add("/bootstrap/launcher/images/([a-zA-Z0-9_.]+)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			const std::string& resource = request.uri.resource();

			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH) +
				"default/images/" +
				resource.substr(resource.rfind('/') + 1);

			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/bootstrap/launcher/notes", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_NOTES_PATH);

			std::string file_data = utils::get_file_text(path);
			utils::string_replace(file_data, "{{dls-version}}", "0.1");
			utils::string_replace(file_data, "{{version-lock}}", Config::GetBool(CONFIG_VERSION_LOCKED) ? "5.3.0.127" : "no");
			utils::string_replace(file_data, "{{game-mode}}", Config::GetBool(CONFIG_SINGLEPLAYER_ONLY) ? "singleplayer" : "multiplayer");
			utils::string_replace(file_data, "{{display-latest-version}}", "none");
			utils::string_replace(file_data, "{{latest-version}}", "yes");

			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = std::move(file_data);
		});

		// Game
		router->add("/game/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			if (request.data.method() == boost::beast::http::verb::post) {
				HTTP::Multipart multipart(request.data.body(), "EA_HTTP_REQUEST_SIMPLE_BOUNDARY");
				for (const auto&[name, value] : multipart) {
					request.uri.set_parameter(name, value);
				}
			}

			auto version = request.uri.parameter("version");
			auto build = request.uri.parameter("build");
			auto method = request.uri.parameter("method");

			if (method.empty()) {
				if (request.uri.parameter("token") == "cookie") {
					method = "api.account.auth";
				} else {
					method = "api.account.getAccount";
				}
			}

			if (method == "api.status.getStatus") {
				game_status_getStatus(response, request.uri);
			} else if (method == "api.status.getBroadcastList") {
				game_status_getBroadcastList(response, request.uri);
			} else if (method == "api.inventory.getPartList") {
				game_inventory_getPartList(response, request.uri);
			} else if (method == "api.account.auth") {
				game_account_auth(response, request.uri);
			} else if (method == "api.account.getAccount") {
				game_account_getAccount(response, request.uri);
			} else if (method == "api.game.getGame") {
				game_game_getGame(response, request.uri);
			} else {
				std::cout << "Undefined /game/api method: " << method << std::endl;
				empty_xml_response(response);
			}
		});

		// Survey
		router->add("/survey/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			auto version = request.uri.parameter("version");
			auto method = request.uri.parameter("method");

			if (method == "api.survey.getSurveyList") {
				survey_survey_getSurveyList(response, request.uri);
			} else {
				empty_xml_response(response);
			}
		});

		// Web
		router->add("/web/sporelabsgame/announceen", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = "<html><head><title>x</title></head><body>Announces</body></html>";
		});

		router->add("/web/sporelabsgame/persona", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = R"html(
<!DOCTYPE html PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
	<head>
		<script type="text/javascript">
			window.onload = function() {
				Client.inGameRegistration();
			}
		</script>
	</head>
	<body>
		<font color="white">Testing testing 123, does this show up?</font>
	</body>
</html>
)html";
		});

		// QOS
		router->add("/qos/qos", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, HTTP::Request& request) {
			auto version = request.uri.parameteri("vers");
			auto type = request.uri.parameteri("qtyp");
			auto port = request.uri.parameteri("prpt");

			if (type == 1 && port == 3659) {
				pugi::xml_document document;

				auto docResponse = document.append_child("response");
				add_text_node(docResponse, "result", "1,0,1");
				add_common_keys(docResponse);

				xml_string_writer writer;
				document.save(writer, "\t", 1U, pugi::encoding_latin1);

				response.set(boost::beast::http::field::content_type, "text/xml");
				response.body() = std::move(writer.result);
			} else {
				response.set(boost::beast::http::field::content_type, "text/plain");
				response.body() = "";
			}
		});
	}

	void API::empty_xml_response(HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::empty_json_response(HTTP::Response& response) {
		response.set(boost::beast::http::field::content_type, "application/json");
	}

	void API::dls_launcher_setTheme(HTTP::Response& response, const HTTP::URI& uri) {
		/*
		theme = requestUtils.get(request, 'theme', str)
		server.setActiveTheme(theme)
		*/
		rapidjson::StringBuffer buffer;
		buffer.Clear();

		rapidjson::Document document;

		// stat
		document.AddMember(rapidjson::Value("stat"), rapidjson::Value("ok"), document.GetAllocator());

		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = buffer.GetString();
	}

	void API::dls_launcher_listThemes(HTTP::Response& response, const HTTP::URI& uri) {
		rapidjson::StringBuffer buffer;
		buffer.Clear();

		rapidjson::Document document;
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// stat
		document.AddMember(rapidjson::Value("stat"), rapidjson::Value("ok"), allocator);
		
		// themes
		{
			rapidjson::Value value(rapidjson::kArrayType);
			value.PushBack(rapidjson::Value("default"), allocator);

			document.AddMember(rapidjson::Value("themes"), value, allocator);
		}

		// selectedTheme
		document.AddMember(rapidjson::Value("selectedTheme"), rapidjson::Value("default"), allocator);

		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = buffer.GetString();
	}

	void API::bootstrap_config_getConfig(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		const auto& host = Config::Get(CONFIG_SERVER_HOST);

		auto docResponse = document.append_child("response");
		if (auto configs = docResponse.append_child("configs")) {
			if (auto config = configs.append_child("config")) {
				add_text_node(config, "blaze_service_name", "darkspore");
				add_text_node(config, "blaze_secure", "Y");
				add_text_node(config, "blaze_env", "production");
				add_text_node(config, "sporenet_cdn_host", host);
				add_text_node(config, "sporenet_cdn_port", "80");
				add_text_node(config, "sporenet_db_host", host);
				add_text_node(config, "sporenet_db_port", "80");
				add_text_node(config, "sporenet_db_name", "darkspore");
				add_text_node(config, "sporenet_host", host);
				add_text_node(config, "sporenet_port", "80");
				add_text_node(config, "http_secure", "N");
				add_text_node(config, "liferay_host", host);
				add_text_node(config, "liferay_port", "80");
				add_text_node(config, "launcher_action", "2");
				add_text_node(config, "launcher_url", "http://" + host + "/bootstrap/launcher/?version=" + mVersion);
			}
		}

		docResponse.append_child("to_image");
		docResponse.append_child("from_image");

		if (uri.parameterb("include_settings")) {
			if (auto settings = docResponse.append_child("settings")) {
				add_text_node(settings, "open", "true");
				add_text_node(settings, "telemetry-rate", "256");
				add_text_node(settings, "telemetry-setting", "0");
			}
		}

		if (uri.parameterb("include_patches")) {
			docResponse.append_child("patches");
			/*
			target, date, from_version, to_version, id, description, application_instructions,
			locale, shipping, file_url, archive_size, uncompressed_size,
			hashes(attributes, Version, Hash, Size, BlockSize)
			*/
		}

		add_common_keys(docResponse);
		// document.child("version").set_value(mVersion.c_str());

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_status_getStatus(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto status = docResponse.append_child("status")) {
			add_text_node(status, "health", "1"); // ok

			if (auto api = status.append_child("api")) {
				add_text_node(api, "health", "1");
				add_text_node(api, "revision", "1");
				add_text_node(api, "version", "1");
			}

			if (auto blaze = status.append_child("blaze")) {
				add_text_node(blaze, "health", "1"); // ok
			}

			if (auto gms = status.append_child("gms")) {
				add_text_node(gms, "health", "1");
			}

			if (auto nucleus = status.append_child("nucleus")) {
				add_text_node(nucleus, "health", "1");
			}

			if (auto game = status.append_child("game")) {
				add_text_node(game, "health", "1"); // ok
				add_text_node(game, "countdown", "0"); // ok
				add_text_node(game, "open", "1"); // ok
				add_text_node(game, "throttle", "0"); // ok
				add_text_node(game, "vip", "0");
			}
			/*
			if (auto unk = status.append_child("$\x84")) {
				add_text_node(unk, "health", "1");
				add_text_node(unk, "revision", "1");
				add_text_node(unk, "db_version", "1");
			}

			if (auto unk = status.append_child("$\x8B")) {
				add_text_node(unk, "health", "1");
			}
			*/
		}

		if (uri.parameterb("include_broadcasts")) {
			add_broadcasts(docResponse);
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_status_getBroadcastList(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		add_broadcasts(docResponse);
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_inventory_getPartList(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto parts = docResponse.append_child("parts")) {
			if (auto part = parts.append_child("part")) {
				add_text_node(part, "is_flair", "0");
				add_text_node(part, "cost", "1");
				add_text_node(part, "creature_id", "1");
				add_text_node(part, "id", "1");
				add_text_node(part, "level", "1");
				add_text_node(part, "market_status", "0");
				add_text_node(part, "prefix_asset_id", "1");
				add_text_node(part, "prefix_secondary_asset_id", "1");
				add_text_node(part, "rarity", "1");
				add_text_node(part, "reference_id", "1");
				add_text_node(part, "rigblock_asset_id", "0");
				add_text_node(part, "status", "0");
				add_text_node(part, "suffix_asset_id", "0");
				add_text_node(part, "usage", "0");
				add_text_node(part, "creation_date", "0");
			}
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_account_auth(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;
		/*
			timestamp
				{TIMESTAMP}

			account
				tutorial_completed
				chain_progression
				creature_rewards
				current_game_id
				current_playgroup_id
				default_deck_pve_id
				default_deck_pvp_id
				level
				avatar_id
				id
				new_player_inventory
				new_player_progress
				cashout_bonus_time
				star_level
				unlock_catalysts
				unlock_diagonal_catalysts
				unlock_fuel_tanks
				unlock_inventory
				unlock_pve_decks
				unlock_pvp_decks
				unlock_stats
				unlock_inventory_identify
				unlock_editor_flair_slots
				upsell
				xp
				grant_all_access
				cap_level
				cap_progression

			creatures
				id
				name
				png_thumb_url
				noun_id
				version
				gear_score
				item_points

			decks
				name
				category
				id
				slot
				locked
				creatures
					{SAME_AS_CREATURES_BEFORE}

			feed
				items

			settings

			server_tuning
				itemstore_offer_period
				itemstore_current_expiration
				itemstore_cost_multiplier_basic
				itemstore_cost_multiplier_uncommon
				itemstore_cost_multiplier_rare
				itemstore_cost_multiplier_epic
				itemstore_cost_multiplier_unique
				itemstore_cost_multiplier_rareunique
				itemstore_cost_multiplier_epicunique
		*/
		auto docResponse = document.append_child("response");
		if (auto account = docResponse.append_child("account")) {
			add_text_node(account, "tutorial_completed", "N");
			add_text_node(account, "chain_progression", "0");
			add_text_node(account, "creature_rewards", "3");
			add_text_node(account, "current_game_id", "1");
			add_text_node(account, "current_playgroup_id", "1");
			add_text_node(account, "default_deck_pve_id", "1");
			add_text_node(account, "default_deck_pvp_id", "1");
			add_text_node(account, "level", "3");
			add_text_node(account, "avatar_id", "0");
			add_text_node(account, "id", "1");
			add_text_node(account, "new_player_inventory", "0");
			add_text_node(account, "new_player_progress", "0");
			add_text_node(account, "cashout_bonus_time", "0");
			add_text_node(account, "star_level", "0");
			add_text_node(account, "unlock_catalysts", "0");
			add_text_node(account, "unlock_diagonal_catalysts", "0");
			add_text_node(account, "unlock_fuel_tanks", "0");
			add_text_node(account, "unlock_inventory", "0");
			add_text_node(account, "unlock_pve_decks", "0");
			add_text_node(account, "unlock_pvp_decks", "0");
			add_text_node(account, "unlock_stats", "0");
			add_text_node(account, "unlock_inventory_identify", "0");
			add_text_node(account, "unlock_editor_flair_slots", "0");
			add_text_node(account, "upsell", "0");
			add_text_node(account, "xp", "0");
			add_text_node(account, "grant_all_access", "0");
			add_text_node(account, "cap_level", "0");
			add_text_node(account, "cap_progression", "0");
		}

		/*
		auto creatures = docResponse.append_child("creatures");
		if (creatures) {
			if (auto creature = creatures.append_child("creature")) {
				add_text_node(creature, "id", "1");
				add_text_node(creature, "name", "Blitz");
				add_text_node(creature, "png_thumb_url", "");
				add_text_node(creature, "noun_id", "1");
				add_text_node(creature, "version", "1");
				add_text_node(creature, "gear_score", "1");
				add_text_node(creature, "item_points", "1");
			}

			if (auto creature = creatures.append_child("creature")) {
				add_text_node(creature, "id", "2");
				add_text_node(creature, "name", "Sage");
				add_text_node(creature, "png_thumb_url", "");
				add_text_node(creature, "noun_id", "2");
				add_text_node(creature, "version", "1");
				add_text_node(creature, "gear_score", "1");
				add_text_node(creature, "item_points", "1");
			}
		}
			
		if (auto decks = docResponse.append_child("decks")) {
			if (auto deck = decks.append_child("deck")) {
				add_text_node(deck, "name", "Squad 1");
				add_text_node(deck, "id", "1");
				add_text_node(deck, "category", "0");
				add_text_node(deck, "slot", "1");
				add_text_node(deck, "locked", "0");
				deck.append_copy(creatures);
			}
		}
		*/

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_account_getAccount(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;
		
		auto docResponse = document.append_child("response");
		if (auto account = docResponse.append_child("account")) {
			add_text_node(account, "tutorial_completed", "N");
			add_text_node(account, "chain_progression", "0");
			add_text_node(account, "creature_rewards", "3");
			add_text_node(account, "current_game_id", "1");
			add_text_node(account, "current_playgroup_id", "1");
			add_text_node(account, "default_deck_pve_id", "1");
			add_text_node(account, "default_deck_pvp_id", "1");
			add_text_node(account, "level", "3");
			add_text_node(account, "avatar_id", "0");
			add_text_node(account, "id", "1");
			add_text_node(account, "new_player_inventory", "0");
			add_text_node(account, "new_player_progress", "0");
			add_text_node(account, "cashout_bonus_time", "0");
			add_text_node(account, "star_level", "0");
			add_text_node(account, "unlock_catalysts", "0");
			add_text_node(account, "unlock_diagonal_catalysts", "0");
			add_text_node(account, "unlock_fuel_tanks", "0");
			add_text_node(account, "unlock_inventory", "0");
			add_text_node(account, "unlock_pve_decks", "0");
			add_text_node(account, "unlock_pvp_decks", "0");
			add_text_node(account, "unlock_stats", "0");
			add_text_node(account, "unlock_inventory_identify", "0");
			add_text_node(account, "unlock_editor_flair_slots", "0");
			add_text_node(account, "upsell", "0");
			add_text_node(account, "xp", "0");
			add_text_node(account, "grant_all_access", "0");
			add_text_node(account, "cap_level", "0");
			add_text_node(account, "cap_progression", "0");
		}
		
		if (auto decks = docResponse.append_child("decks")) {
			if (auto deck = decks.append_child("deck")) {
				add_text_node(deck, "name", "This be muh squad");
				add_text_node(deck, "id", "1");
				add_text_node(deck, "category", "0");
				add_text_node(deck, "slot", "1");
				add_text_node(deck, "locked", "0");
				if (auto creatures = deck.append_child("creatures")) {
					if (auto creature = creatures.append_child("creature")) {
						add_text_node(creature, "id", "1");
						add_text_node(creature, "name", "Blitz");
						add_text_node(creature, "png_thumb_url", "Blitz.png");
						add_text_node(creature, "noun_id", "1");
						add_text_node(creature, "version", "1");
						add_text_node(creature, "gear_score", "1");
						add_text_node(creature, "item_points", "1");
					}

					if (auto creature = creatures.append_child("creature")) {
						add_text_node(creature, "id", "2");
						add_text_node(creature, "name", "Sage");
						add_text_node(creature, "png_thumb_url", "Sage.png");
						add_text_node(creature, "noun_id", "2");
						add_text_node(creature, "version", "1");
						add_text_node(creature, "gear_score", "1");
						add_text_node(creature, "item_points", "1");
					}
				}
			}
		}
		
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_game_getGame(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto game = docResponse.append_child("game")) {
			add_text_node(game, "game_id", "1");
			add_text_node(game, "cashed_out", "0");
			add_text_node(game, "finished", "0");
			add_text_node(game, "starting_difficulty", "1");
			add_text_node(game, "start", "1");
			/*
			add_text_node(game, "rounds", "0");
			add_text_node(game, "chain_id", "0");
			add_text_node(game, "finish", "0");
			add_text_node(game, "planet_id", "0");
			add_text_node(game, "success", "0");
			*/
			if (auto players = game.append_child("players")) {
				if (auto player = players.append_child("player")) {
					/*
					add_text_node(player, "deaths", "0");
					add_text_node(player, "kills", "0");
					*/
					add_text_node(player, "account_id", "1");
					add_text_node(player, "result", "0");
					add_text_node(player, "creature1_id", "1");
					add_text_node(player, "creature1_version", "1");
					add_text_node(player, "creature2_id", "0");
					add_text_node(player, "creature2_version", "0");
					add_text_node(player, "creature3_id", "0");
					add_text_node(player, "creature3_version", "0");
				}
			}
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::survey_survey_getSurveyList(HTTP::Response& response, const HTTP::URI& uri) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto surveys = docResponse.append_child("surveys")) {
			// Empty
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::add_broadcasts(pugi::xml_node& node) {
		if (auto broadcasts = node.append_child("broadcasts")) {
			/*
			if (auto broadcast = broadcasts.append_child("broadcast")) {
				add_text_node(broadcast, "id", "0");
				add_text_node(broadcast, "start", "0");
				add_text_node(broadcast, "type", "0");
				add_text_node(broadcast, "message", "Hello World!");
				if (auto tokens = broadcast.append_child("tokens")) {
				}
			}
			*/
		}
	}

	void API::add_text_node(pugi::xml_node& node, const std::string& name, const std::string& value) {
		node.append_child(name.c_str()).append_child(pugi::node_pcdata).set_value(value.c_str());
	}

	void API::add_common_keys(pugi::xml_node& node) {
		add_text_node(node, "stat", "ok");
		add_text_node(node, "version", mVersion);
		add_text_node(node, "timestamp", std::to_string(utils::get_unix_time()));
		add_text_node(node, "exectime", std::to_string(++mPacketId));
	}

	void API::add_common_keys(rapidjson::Document& document) {
		/*
		obj["stat"] = 'ok'
		obj["version"] = self.server.gameVersion
		obj["timestamp"] = self.timestamp()
		obj["exectime"] = self.exectime()
		*/
	}
}
