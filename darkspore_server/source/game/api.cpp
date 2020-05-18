
// Include
#include "api.h"
#include "config.h"

#include "http/session.h"
#include "http/router.h"
#include "http/uri.h"
#include "http/multipart.h"

#include "sporenet/instance.h"
#include "sporenet/user.h"
#include "sporenet/creature.h"

#include "utils/functions.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <boost/beast/version.hpp>

#include <iostream>
#include <iomanip>
#include <filesystem>

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

std::map<std::string, std::string> parse_cookies(HTTP::Request& request) {
	std::map<std::string, std::string> data;

	auto cookies = request.data[boost::beast::http::field::cookie];
	if (cookies.empty()) {
		return data;
	}

	auto cookieList = utils::explode_string(';' + cookies.to_string(), ';');
	for (const auto& cookie : cookieList) {
		auto cookieData = utils::explode_string(cookie, '=');
		if (cookieData.size() == 2) {
			data.emplace(cookieData[0], cookieData[1]);
		}
	}

	return data;
}

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

	constexpr std::string_view recapClientScript = R"(
<script>
	var ReCapClient = {};
	ReCapClient.getRequest = function(url, callback) {
		var xmlHttp = new XMLHttpRequest(); 
		xmlHttp.onload = function() {
			callback(xmlHttp.responseText);
		}
		xmlHttp.open("GET", url, true);
		xmlHttp.send(null);
	};
	ReCapClient.request = function(name, params, callback) {
		ReCapClient.getRequest("http://{{host}}/recap/api?method=" + name + (params === undefined ? "" : ("&" + params)), callback);
	};
</script>
)";

	// API
	API::API(const std::string& version) : mVersion(version) {
		// Empty
	}

	void API::setup() {
		const auto& router = GetApp().get_http_server()->get_router();

		// Routing
		router->add("/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [](HTTP::Session& session, HTTP::Response& response) {
			std::cout << "Got API route." << std::endl;
		});

		// ReCap
		router->add("/recap/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			const auto& request = session.get_request();

			auto method = request.uri.parameter("method");
			if (method == "api.launcher.setTheme") {
				recap_launcher_setTheme(session, response);
			} else if (method == "api.launcher.listThemes") {
				recap_launcher_listThemes(session, response);
			} else if (method == "api.game.registration") {
				recap_game_registration(session, response);
			} else if (method == "api.game.log") {
				recap_game_log(session, response);
			} else if (method == "api.panel.listUsers") {
				recap_panel_listUsers(session, response);
			} else if (method == "api.panel.getUserInfo") {
				recap_panel_getUserInfo(session, response);
			} else if (method == "api.panel.setUserInfo") {
				recap_panel_setUserInfo(session, response);
			} else {
				response.result() = boost::beast::http::status::internal_server_error;
			}
		});

		// Launcher
		router->add("/bootstrap/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto version = request.uri.parameter("version");
			auto build = request.uri.parameter("build");
			auto method = request.uri.parameter("method");

			if (version.empty()) { version = "1"; }
			if (build.empty()) { build = "5.3.0.127"; }
			if (method.empty()) { method = "api.config.getConfigs"; }

			if (method.find("api.config.") == 0) {
				if (method == "api.config.getConfigs") {
					bootstrap_config_getConfig(session, response);
				}
			}
		});

		router->add("/bootstrap/launcher/", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto version = request.uri.parameter("version");
			if (Config::GetBool(CONFIG_SKIP_LAUNCHER)) {
				response.set(boost::beast::http::field::content_type, "text/html");
				response.body() = skipLauncherScript;
			} else {
				std::string path = Config::Get(CONFIG_STORAGE_PATH) +
					"www/" +
					Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH) +
					"darkui/index.html";

				std::string client_script(recapClientScript);
				utils::string_replace(client_script, "{{host}}", Config::Get(CONFIG_SERVER_HOST));

				std::string file_data = utils::get_file_text(path);
				utils::string_replace(file_data, "</head>", client_script + "</head>");

				response.set(boost::beast::http::field::content_type, "text/html");
				response.body() = std::move(file_data);
			}
		});

		router->add("/bootstrap/launcher/images/([a-zA-Z0-9_.]+)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			const std::string& resource = request.uri.resource();

			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH) +
				"darkui/images/" +
				resource.substr(resource.rfind('/') + 1);

			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/bootstrap/launcher/notes", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_NOTES_PATH);

			std::string file_data = utils::get_file_text(path);
			utils::string_replace(file_data, "{{recap-version}}", "0.1");
			utils::string_replace(file_data, "{{version-lock}}", Config::GetBool(CONFIG_VERSION_LOCKED) ? "5.3.0.127" : "no");
			utils::string_replace(file_data, "{{game-mode}}", Config::GetBool(CONFIG_SINGLEPLAYER_ONLY) ? "singleplayer" : "multiplayer");
			utils::string_replace(file_data, "{{display-latest-version}}", "none");
			utils::string_replace(file_data, "{{latest-version}}", "yes");

			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = std::move(file_data);
		});

		// Game
		router->add("/game/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// TODO: fix all of these things, its so ugly
			auto& request = session.get_request();

			if (request.data.method() == boost::beast::http::verb::post) {
				// TODO: Fetch boundary later from request.data[boost::beast::http::field::content_type]
				
				HTTP::Multipart multipart(request.data.body(), "EA_HTTP_REQUEST_SIMPLE_BOUNDARY");
				for (const auto&[name, value] : multipart) {
					request.uri.set_parameter(name, value);
				}
			}

			// TODO: cookie handler
			auto cookies = request.data[boost::beast::http::field::cookie];
			if (!cookies.empty()) {
				auto cookieMap = parse_cookies(request);
				for (const auto& [name, value] : cookieMap) {
					auto it = request.uri.find(name);
					if (it != request.uri.end()) {
						if (it->second == "cookie") {
							it->second = value;
						}
					}
				}
			}

			for (auto& [name, value] : request.uri) {
				std::cout << name << " = " << value << std::endl;
			}
			std::cout << std::endl;

			auto version = request.uri.parameter("version");
			auto build = request.uri.parameter("build");
			auto token = request.uri.parameter("token");

			if (!token.empty()) {
				const auto& user = GetApp().GetSporeNet().GetUserManager().GetUserByAuthToken(token);
				if (user) {
					session.set_user(user);
				}
			}

			auto method = request.uri.parameter("method");
			if (method.empty()) {
				if (token.empty()) {
					method = "api.account.auth";
				} else {
					method = "api.account.getAccount";
				}
			}

			if (method == "api.account.setNewPlayerStats") {
				method = "api.account.auth";
			}

			if (method == "api.status.getStatus") {
				game_status_getStatus(session, response);
			} else if (method == "api.status.getBroadcastList") {
				game_status_getBroadcastList(session, response);
			} else if (method == "api.inventory.getPartList") {
				game_inventory_getPartList(session, response);
			} else if (method == "api.inventory.getPartOfferList") {
				game_inventory_getPartOfferList(session, response);
			} else if (method == "api.inventory.vendorParts") {
				game_inventory_vendorParts(session, response);
			} else if (method == "api.inventory.updatePartStatus") {
				game_inventory_updatePartStatus(session, response);
			} else if (method == "api.account.auth") {
				game_account_auth(session, response);
			} else if (method == "api.account.getAccount") {
				game_account_getAccount(session, response);
			} else if (method == "api.account.logout") {
				game_account_logout(session, response);
			} else if (method == "api.account.unlock") {
				game_account_unlock(session, response);
			} else if (method == "api.game.getGame" || method == "api.game.getRandomGame") {
				game_game_getGame(session, response);
			} else if (method == "api.game.exitGame") {
				game_game_exitGame(session, response);
			} else if (method == "api.creature.resetCreature") {
				game_creature_resetCreature(session, response);
			} else if (method == "api.creature.unlockCreature") {
				game_creature_unlockCreature(session, response);
			} else if (method == "api.creature.getCreature") {
				game_creature_getCreature(session, response);
			} else if (method == "api.creature.getTemplate") {
				game_creature_getTemplate(session, response);
			} else if (method == "api.deck.updateDecks") {
				game_deck_updateDecks(session, response);
			} else if (method == "api.game.log") {
				recap_game_log(session, response);
			} else {
				std::cout << "Undefined /game/api method: " << method << std::endl;
				for (const auto& [name, value] : request.uri) {
					std::cout << name << " = " << value << std::endl;
				}
				std::cout << std::endl;
				empty_xml_response(response);
			}
		});

		/*
Undefined /game/api method: api.deck.updateDecks
method = api.deck.updateDecks
pve_active_slot = 1
pve_creatures = 10,11,3948469269,0,0,0,0,0,0
pvp_active_slot = 28614456
pvp_creatures = 0,0,0,0,0,0,0,0,0
token = ABCDEFGHIJKLMNOPQRSTUVWXYZ
version = 1
		*/

		// Png
		router->add("/template_png/([a-zA-Z0-9_.]+)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			std::string path = Config::Get(CONFIG_STORAGE_PATH) + request.uri.resource();

			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/creature_png/([a-zA-Z0-9_.]+)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			std::string path = Config::Get(CONFIG_STORAGE_PATH) + request.uri.resource();

			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/game/service/png", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto template_id = request.uri.parameter<uint32_t>("template_id");
			auto size = request.uri.parameter("size");

			std::stringstream stream;
			stream << Config::Get(CONFIG_STORAGE_PATH);
			stream << "template_png/";
			stream << size << "/";
			stream << "0x" << std::setfill('0') << std::setw(8) << std::hex << template_id << std::dec << ".png";

			response.version() |= 0x1000'0000;
			response.body() = stream.str();
		});

		// Icon
		router->add("/favicon.ico", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto version = request.uri.parameter("version");
			auto method = request.uri.parameter("method");

			if (method == "api.survey.getSurveyList") {
				survey_survey_getSurveyList(session, response);
			} else {
				empty_xml_response(response);
			}

			// return send_from_directory(staticFolderPath, 'favicon.ico', mimetype = 'image/vnd.microsoft.icon')
		});

		// Survey
		router->add("/survey/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto version = request.uri.parameter("version");
			auto method = request.uri.parameter("method");

			if (method == "api.survey.getSurveyList") {
				survey_survey_getSurveyList(session, response);
			} else {
				empty_xml_response(response);
			}
		});

		// Web
		router->add("/web/sporelabsgame/announceen", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = "<html><head><title>x</title></head><body>Announces</body></html>";
		});

		router->add("/web/sporelabsgame/persona", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = "";
		});

		// QOS
		router->add("/qos/qos", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			auto& request = session.get_request();

			auto version = request.uri.parameter<int32_t>("vers");
			auto type = request.uri.parameter<int32_t>("qtyp");
			auto port = request.uri.parameter<uint16_t>("prpt");

			if (type == 1 && port == 3659) {
				pugi::xml_document document;

				auto docResponse = document.append_child("response");
				utils::xml_add_text_node(docResponse, "result", "1,0,1");
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

		// Ingame Webviews
		router->add("/ingame/([/a-zA-Z0-9\\-_.]*)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			std::string path = Config::Get(CONFIG_STORAGE_PATH) + "www/" + session.get_request().uri.resource();
			/*
			utils::string_replace(file_data, "{{isDev}}", "true");
			utils::string_replace(file_data, "{{recap-version}}", "0.1");
			utils::string_replace(file_data, "{{host}}", Game::Config::Get(Game::CONFIG_SERVER_HOST));
			utils::string_replace(file_data, "{{game-mode}}", Game::Config::GetBool(Game::CONFIG_SINGLEPLAYER_ONLY) ? "singleplayer" : "multiplayer");
			*/
			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/web/sporelabs/alerts", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame/announce.html");
		});

		router->add("/web/sporelabsgame/announceen", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame/announce.html");
		});

		router->add("/web/sporelabsgame/register", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame/register.html");
		});

		router->add("/web/sporelabsgame/persona", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame/persona.html");
		});

		router->add("/web/sporelabs/resetpassword", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame/resetpassword.html");
		});

		router->add("/web/sporelabsgame/([/a-zA-Z0-9\\-_.]*)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame" + session.get_request().uri.resource().substr(18));
		});

		router->add("/web/sporelabs/([/a-zA-Z0-9\\-_.]*)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Session& session, HTTP::Response& response) {
			// response.body() = utils::EAWebKit::loadFile("www/ingame" + session.get_request().uri.resource().substr(14));
		});
	}

	void API::empty_xml_response(HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::empty_json_response(HTTP::Response& response) {
		response.set(boost::beast::http::field::content_type, "application/json");
	}

	void API::recap_launcher_setTheme(HTTP::Session& session, HTTP::Response& response) {
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

	void API::recap_launcher_listThemes(HTTP::Session& session, HTTP::Response& response) {
		rapidjson::StringBuffer buffer;
		buffer.Clear();

		rapidjson::Document document;
		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// stat
		document.AddMember(rapidjson::Value("stat"), rapidjson::Value("ok"), allocator);
		
		// themes
		{
			std::string themesFolderPath = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" + Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH);

			rapidjson::Value value(rapidjson::kArrayType);
			for (const auto& entry : std::filesystem::directory_iterator(themesFolderPath)) {
				if (entry.is_directory()) {
					value.PushBack(rapidjson::Value(entry.path().filename().string(), allocator), allocator);
				}
			}

			document.AddMember(rapidjson::Value("themes"), value, allocator);
		}

		// selectedTheme
		document.AddMember(rapidjson::Value("selectedTheme"), rapidjson::Value("darkui"), allocator);

		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = buffer.GetString();
	}

	void API::recap_game_registration(HTTP::Session& session, HTTP::Response& response) {
		/*
		auto& request = session.get_request();
		auto name = request.uri.parameter("name");
		auto mail = request.uri.parameter("mail");
		auto pass = request.uri.parameter("pass");
		auto avatar = request.uri.parameteri("avatar");

		const auto& user = Repository::Users::CreateUserWithNameMailAndPassword(name, mail, pass);
		rapidjson::Document document = utils::json::NewDocumentObject();
		if (user == NULL) {
			utils::json::Set(document, "stat", "error");
			response.set(boost::beast::http::field::content_type, "application/json");
			response.body() = utils::json::ToString(document);
			return;
		}

		utils::json::Set(document, "stat", "ok");

		auto actualPartsSize = Repository::CreatureParts::ListAll().size();
		auto parts = Repository::Parts::ListAll();
		uint64_t index = 1;
		for (auto& part : parts) {
			Repository::CreatureParts::Add(std::make_shared<Game::CreaturePart>(actualPartsSize + index++, part->rigblock_asset_id, user->get_account().id));
		}
		Repository::CreatureParts::Save();

		// TODO: Unlocking all creatures from start to test; remove that in the future
		std::vector<Repository::CreatureTemplatePtr> templates = Repository::CreatureTemplates::ListAll();
		user->get_account().creatureRewards = templates.size();
		for (auto& templateCreature : templates) {
			user->UnlockCreature(templateCreature->id);
		}

		// TODO: Unlocking everything from start to test; remove that in the future
		SporeNet::Account& account = user->get_account();
		account.tutorialCompleted = false;
		account.chainProgression = 24;
		account.creatureRewards = 100;
		account.currentGameId = 1;
		account.currentPlaygroupId = 1;
		account.defaultDeckPveId = 1;
		account.defaultDeckPvpId = 1;
		account.level = 100;
		account.avatarId = avatar;
		account.dna = 10000000;
		account.newPlayerInventory = 1;
		account.newPlayerProgress = 9500;
		account.cashoutBonusTime = 1;
		account.starLevel = 10;
		account.unlockCatalysts = 1;
		account.unlockDiagonalCatalysts = 1;
		account.unlockFuelTanks = 1;
		account.unlockInventory = 1;
		account.unlockPveDecks = 2;
		account.unlockPvpDecks = 1;
		account.unlockStats = 1;
		account.unlockInventoryIdentify = 2500;
		account.unlockEditorFlairSlots = 1;
		account.upsell = 1;
		account.xp = 10000;
		account.grantAllAccess = true;
		account.grantOnlineAccess = true;

		for (uint16_t squadSlot = 1; squadSlot <= 3; squadSlot++) {
			uint16_t templateId = squadSlot - 1;
			Squad squad1;
			squad1.id = squadSlot;
			squad1.slot = squadSlot;
			squad1.name = "Slot " + std::to_string(squadSlot);
			squad1.locked = false;
			squad1.creatures.Add(templates[templateId]->id);
			user->get_squads().data().push_back(squad1);
		}

		Repository::Users::SaveUser(user);

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = utils::json::ToString(document);
		*/
	}

	void API::recap_game_log(HTTP::Session& session, HTTP::Response& response) {
		const auto& request = session.get_request();
		/*
		auto postBody = request.data.body();
		std::cout << "js.console.log: " << postBody << std::endl;
		*/
		std::cout << "js.console.log: " << request.uri.parameter("message") << std::endl;
		// logger::log(postBody);
	}

	void API::recap_panel_listUsers(HTTP::Session& session, HTTP::Response& response) {
		/*
		rapidjson::Document document = utils::json::NewDocumentObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// stat
		utils::json::Set(document, "stat", "ok");

		const auto users = Repository::Users::GetAllUserNames();
		const auto loggedUsers = Repository::Users::GetLoggedUserNames();
		rapidjson::Value value = utils::json::NewArray();
		for (const auto& entry : users) {
			bool isLogged = std::find(loggedUsers.begin(), loggedUsers.end(), entry) != loggedUsers.end();

			rapidjson::Value object = utils::json::NewObject();
			utils::json::Set(object, "email", entry, allocator);
			utils::json::Set(object, "logged", isLogged, allocator);
			utils::json::Add(value, object, allocator);
		}
		utils::json::Set(document, "users", value);

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = utils::json::ToString(document);
		*/
	}

	void API::recap_panel_getUserInfo(HTTP::Session& session, HTTP::Response& response) {
		/*
		auto& request = session.get_request();
		auto mail = request.uri.parameter("mail");

		const auto& user = Repository::Users::GetUserByEmail(mail, false);

		rapidjson::Document document = utils::json::NewDocumentObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (user == NULL) {
			// stat
			utils::json::Set(document, "stat", "error");
		} else {
			// stat
			rapidjson::Value users = user->ToJson(allocator);
			utils::json::Set(document, "stat", "ok");
			utils::json::Set(document, "user", users);
		}

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = utils::json::ToString(document);
		*/
	}

	void API::recap_panel_setUserInfo(HTTP::Session& session, HTTP::Response& response) {
		/*
		auto& request = session.get_request();
		auto mail = request.uri.parameter("mail");
		auto userJsonString = request.data.body();

		logger::info(userJsonString);
		rapidjson::Document userJson = utils::json::Parse(userJsonString);

		const auto& user = Repository::Users::GetUserByEmail(mail, false);

		rapidjson::Document document = utils::json::NewDocumentObject();

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		if (user == NULL) {
			// stat
			utils::json::Set(document, "stat", "error");
		} else {
			user->FromJson(userJson);
			Repository::Users::SaveUser(user);

			// stat
			utils::json::Set(document, "stat", "ok");
		}

		response.set(boost::beast::http::field::content_type, "application/json");
		response.body() = utils::json::ToString(document);
		*/
	}

	void API::bootstrap_config_getConfig(HTTP::Session& session, HTTP::Response& response) {
		auto& request = session.get_request();

		pugi::xml_document document;

		const auto& host = Config::Get(CONFIG_SERVER_HOST);

		auto docResponse = document.append_child("response");
		if (auto configs = docResponse.append_child("configs")) {
			if (auto config = configs.append_child("config")) {
				utils::xml_add_text_node(config, "blaze_service_name", "darkspore"); // Directly linked to BlazeServiceName
				utils::xml_add_text_node(config, "blaze_secure", "N"); // Directly linked to BlazeSecure
				utils::xml_add_text_node(config, "blaze_env", "prod"); // Directly linked to BlazeEnvironment, can be { prod, beta, cert, test, dev }
				utils::xml_add_text_node(config, "sporenet_cdn_host", host);
				utils::xml_add_text_node(config, "sporenet_cdn_port", 80);
				utils::xml_add_text_node(config, "sporenet_db_host", host);
				utils::xml_add_text_node(config, "sporenet_db_port", 80);
				utils::xml_add_text_node(config, "sporenet_db_name", "darkspore");
				utils::xml_add_text_node(config, "sporenet_host", host);
				utils::xml_add_text_node(config, "sporenet_port", 80);
				utils::xml_add_text_node(config, "http_secure", "N");
				utils::xml_add_text_node(config, "liferay_host", host);
				utils::xml_add_text_node(config, "liferay_port", 80);
				utils::xml_add_text_node(config, "launcher_action", 2);
				utils::xml_add_text_node(config, "launcher_url", "http://" + host + "/bootstrap/launcher/?version=" + mVersion);
			}
		}

		docResponse.append_child("to_image");
		docResponse.append_child("from_image");

		bool include_settings = request.uri.parameter("include_settings") == "true";
		if (include_settings) {
			if (auto settings = docResponse.append_child("settings")) {
				utils::xml_add_text_node(settings, "open", "true");
				utils::xml_add_text_node(settings, "telemetry-rate", "256");
				utils::xml_add_text_node(settings, "telemetry-setting", "0");
			}
		}

		bool include_patches = request.uri.parameter("include_patches") == "true";
		if (include_patches) {
			docResponse.append_child("patches");
			/*
			target, date, from_version, to_version, id, description, application_instructions,
			locale, shipping, file_url, archive_size, uncompressed_size,
			hashes(attributes, Version, Hash, Size, BlockSize)
			*/
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_status_getStatus(HTTP::Session& session, HTTP::Response& response) {
		auto& request = session.get_request();

		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto status = docResponse.append_child("status")) {
			utils::xml_add_text_node(status, "health", "1");

			if (auto api = status.append_child("api")) {
				utils::xml_add_text_node(api, "health", "1");
				utils::xml_add_text_node(api, "revision", "1");
				utils::xml_add_text_node(api, "version", "1");
			}

			if (auto blaze = status.append_child("blaze")) {
				utils::xml_add_text_node(blaze, "health", "1");
			}

			if (auto gms = status.append_child("gms")) {
				utils::xml_add_text_node(gms, "health", "1");
			}

			if (auto nucleus = status.append_child("nucleus")) {
				utils::xml_add_text_node(nucleus, "health", "1");
			}

			if (auto game = status.append_child("game")) {
				utils::xml_add_text_node(game, "health", "1");
				utils::xml_add_text_node(game, "countdown", "90");
				utils::xml_add_text_node(game, "open", "1");
				utils::xml_add_text_node(game, "throttle", "0");
				utils::xml_add_text_node(game, "vip", "0");
			}
			/*
			if (auto unk = status.append_child("$\x84")) {
				utils::xml_add_text_node(unk, "health", "1");
				utils::xml_add_text_node(unk, "revision", "1");
				utils::xml_add_text_node(unk, "db_version", "1");
			}

			if (auto unk = status.append_child("$\x8B")) {
				utils::xml_add_text_node(unk, "health", "1");
			}
			*/
		}

		bool include_broadcasts = request.uri.parameter("include_broadcasts") == "true";
		if (include_broadcasts) {
			add_broadcasts(docResponse);
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_status_getBroadcastList(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		add_broadcasts(docResponse);
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_inventory_getPartList(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		if (auto docResponse = document.append_child("response")) {
			const auto& user = session.get_user();
			if (user) {
				user->get_parts().Write(docResponse, true);
			} else {
				docResponse.append_child("parts");
			}
			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_inventory_getPartOfferList(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		if (auto docResponse = document.append_child("response")) {
			auto timestamp = utils::get_unix_time();

			utils::xml_add_text_node(docResponse, "expires", timestamp + (3 * 60 * 60 * 1000));
			if (auto parts = docResponse.append_child("parts")) {
				/*
				if (auto part = parts.append_child("part")) {
					utils::xml_add_text_node(part, "is_flair", "0");
					utils::xml_add_text_node(part, "cost", "100");
					utils::xml_add_text_node(part, "creature_id", static_cast<uint64_t>(CreatureID::BlitzAlpha));
					utils::xml_add_text_node(part, "id", "1");
					utils::xml_add_text_node(part, "level", "1");
					utils::xml_add_text_node(part, "market_status", "1");
					utils::xml_add_text_node(part, "prefix_asset_id", 0x010C);
					utils::xml_add_text_node(part, "prefix_secondary_asset_id", "0");
					utils::xml_add_text_node(part, "rarity", "1");
					utils::xml_add_text_node(part, "reference_id", 1);
					utils::xml_add_text_node(part, "rigblock_asset_id", 0x27A1); // 0xA14538E5
					utils::xml_add_text_node(part, "status", "1");
					utils::xml_add_text_node(part, "suffix_asset_id", 0x0005);
					utils::xml_add_text_node(part, "usage", "1");
					utils::xml_add_text_node(part, "creation_date", timestamp);
				}
				*/
				if (auto part = parts.append_child("part")) {
					utils::xml_add_text_node(part, "is_flair", 0);
					utils::xml_add_text_node(part, "cost", 100);
					utils::xml_add_text_node(part, "creature_id", static_cast<uint64_t>(SporeNet::CreatureID::BlitzAlpha));
					utils::xml_add_text_node(part, "id", 1);
					utils::xml_add_text_node(part, "level", 1);
					utils::xml_add_text_node(part, "market_status", 0);
					utils::xml_add_text_node(part, "prefix_asset_id", utils::hash_id("_Generated/lootprefix1.lootprefix"));
					utils::xml_add_text_node(part, "prefix_secondary_asset_id", utils::hash_id("_Generated/lootprefix2.lootprefix"));
					utils::xml_add_text_node(part, "rarity", 1);
					utils::xml_add_text_node(part, "reference_id", 1);
					utils::xml_add_text_node(part, "rigblock_asset_id", utils::hash_id("_Generated/lootrigblock1.lootrigblock"));
					utils::xml_add_text_node(part, "status", 1);
					utils::xml_add_text_node(part, "suffix_asset_id", utils::hash_id("_Generated/lootsuffix1.lootsuffix"));
					utils::xml_add_text_node(part, "usage", 1);
					utils::xml_add_text_node(part, "creation_date", timestamp);
				}
			}
			
			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_inventory_vendorParts(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		auto& request = session.get_request();

		const auto& transactionsString = request.uri.parameter("transactions");
		if (!transactionsString.empty()) {
			for (const auto& transaction : utils::explode_string(transactionsString, ';')) {
				// w = weapon, check for more later
				char type = transaction[0];

				int64_t index = utils::to_number<int64_t>(&transaction[1]);
				// Stuff
			}
		}

		// Add part to user inventory
		// Resend all user parts?

		if (auto docResponse = document.append_child("response")) {
			auto timestamp = utils::get_unix_time();
			if (auto parts = docResponse.append_child("parts")) {
				if (auto part = parts.append_child("part")) {
					utils::xml_add_text_node(part, "is_flair", 0);
					utils::xml_add_text_node(part, "cost", 100);
					utils::xml_add_text_node(part, "creature_id", static_cast<uint64_t>(SporeNet::CreatureID::BlitzAlpha));
					utils::xml_add_text_node(part, "id", 1);
					utils::xml_add_text_node(part, "level", 18);
					utils::xml_add_text_node(part, "market_status", 0);
					utils::xml_add_text_node(part, "prefix_asset_id", utils::hash_id("_Generated/lootprefix1.lootprefix"));
					utils::xml_add_text_node(part, "prefix_secondary_asset_id", utils::hash_id("_Generated/lootprefix2.lootprefix"));
					utils::xml_add_text_node(part, "rarity", 1);
					utils::xml_add_text_node(part, "reference_id", 1);
					utils::xml_add_text_node(part, "rigblock_asset_id", utils::hash_id("_Generated/lootrigblock1.lootrigblock"));
					utils::xml_add_text_node(part, "status", 1);
					utils::xml_add_text_node(part, "suffix_asset_id", utils::hash_id("_Generated/lootsuffix1.lootsuffix"));
					utils::xml_add_text_node(part, "usage", 1);
					utils::xml_add_text_node(part, "creation_date", timestamp);
				}
			}

			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_inventory_updatePartStatus(HTTP::Session& session, HTTP::Response& response) {
		const auto& user = session.get_user();
		if (!user) {
			// Error
			return;
		}

		const auto& request = session.get_request();

		auto partIds = utils::explode_string(request.uri.parameter("part_id"), ',');
		auto statuses = utils::explode_string(request.uri.parameter("status"), ',');

		size_t len = std::min<size_t>(partIds.size(), statuses.size());
		if (len > 0) {
			auto& parts = user->get_parts();
			for (size_t i = 0; i < len; ++i) {
				auto partIndex = utils::to_number<uint32_t>(partIds[i]);
				auto status = utils::to_number<uint8_t>(statuses[i]);
				parts.data()[partIndex].SetStatus(status);
			}
		}

		empty_xml_response(response);
	}

	void API::game_account_auth(HTTP::Session& session, HTTP::Response& response) {
		const auto& request = session.get_request();

		auto key = request.uri.parameter("key");
		if (!key.empty()) {
			// key = auth_token::0
			auto keyData = utils::explode_string(key, "::");

			const auto& user = GetApp().GetSporeNet().GetUserManager().GetUserByAuthToken(keyData.front());
			if (user) {
				session.set_user(user);
			}
		}

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			const auto& user = session.get_user();
			if (user) {
				if (auto docAccount = docResponse.append_child("account")) {
					// Write "account" data
					auto& account = user->get_account();

					auto newPlayerProgress = request.uri.parameter<uint32_t>("new_player_progress");
					if (newPlayerProgress != 0) {
						std::cout << "Old progress: " << account.newPlayerProgress << ", New: " << newPlayerProgress << std::endl;
						account.newPlayerProgress = newPlayerProgress;
					}

					account.Write(docAccount);
				}

				if (request.uri.parameter("include_creatures") == "true") {
					user->get_creatures().WriteApi(docResponse);
				}

				if (request.uri.parameter("include_decks") == "true") {
					user->WriteSquadsAPI(docResponse);
				}

				if (request.uri.parameter("include_feed") == "true") {
					user->get_feed().Write(docResponse);
				}

				if (request.uri.parameter("include_server_tuning") == "true") {
					if (auto server_tuning = docResponse.append_child("server_tuning")) {
						auto timestamp = utils::get_unix_time();
						utils::xml_add_text_node(server_tuning, "itemstore_offer_period", timestamp);
						utils::xml_add_text_node(server_tuning, "itemstore_current_expiration", timestamp + (3 * 60 * 60 * 1000));
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_basic", 1);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_uncommon", 1.1);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_rare", 1.2);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_epic", 1.3);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_unique", 1.4);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_rareunique", 1.5);
						utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_epicunique", 1.6);
					}
				}

				if (request.uri.parameter("include_settings") == "true") {
					docResponse.append_child("settings");
				}

				if (request.uri.parameter("cookie") == "true") {
					response.set(boost::beast::http::field::set_cookie, "token=" + user->get_auth_token());
				}
			} else {
				// Ignore
			}

			utils::xml_add_text_node(docResponse, "timestamp", utils::get_unix_time());
			add_common_keys(docResponse);
		}

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
		/*
		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			utils::xml_add_text_node(docResponse, "timestamp", utils::get_unix_time());
			if (auto docAccount = docResponse.append_child("account")) {
				// Write "account" data
				if (user) {
					auto& account = user->get_account();

					auto newPlayerProgress = request.uri.parameter<uint32_t>("new_player_progress");
					if (newPlayerProgress != 0) {
						std::cout << "Old progress: " << account.newPlayerProgress << ", New: " << newPlayerProgress << std::endl;
						account.newPlayerProgress = newPlayerProgress;
					}

					account.Write(docAccount);
				} else {
					SporeNet::Account account;
					account.Write(docAccount);
				}
			}

			// Other
			if (request.uri.parameter("include_creatures") == "true") {
				if (user) {
					user->get_creatures().WriteApi(docResponse);
				} else {
					docResponse.append_child("creatures");
				}
			}

			if (request.uri.parameter("include_decks") == "true") {
				if (user) {
					user->get_squads().Write(docResponse);
				} else {
					docResponse.append_child("decks");
				}
			}

			if (request.uri.parameter("include_feed") == "true") {
				if (user) {
					user->get_feed().Write(docResponse);
				} else {
					docResponse.append_child("feed");
				}
			}

			if (request.uri.parameter("include_server_tuning") == "true") {
				if (auto server_tuning = docResponse.append_child("server_tuning")) {
					auto timestamp = utils::get_unix_time();
					utils::xml_add_text_node(server_tuning, "itemstore_offer_period", timestamp);
					utils::xml_add_text_node(server_tuning, "itemstore_current_expiration", timestamp + (3 * 60 * 60 * 1000));
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_basic", 1);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_uncommon", 1.1);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_rare", 1.2);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_epic", 1.3);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_unique", 1.4);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_rareunique", 1.5);
					utils::xml_add_text_node(server_tuning, "itemstore_cost_multiplier_epicunique", 1.6);
				}
			}

			if (request.uri.parameter("include_settings") == "true") {
				docResponse.append_child("settings");
			}

			if (request.uri.parameter("cookie") == "true") {
				std::string cookie;
				if (user) {
					cookie = user->get_auth_token();
				} else {
					cookie = "TESTING";
				}
				response.set(boost::beast::http::field::set_cookie, "token=" + cookie);
			}

			add_common_keys(docResponse);
		}
		*/
		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_account_getAccount(HTTP::Session& session, HTTP::Response& response) {
		const auto& user = session.get_user();
		if (!user) {
			// Send some error?
			return;
		}

		const auto& request = session.get_request();
		const auto& account = user->get_account();

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			add_common_keys(docResponse);

			utils::xml_add_text_node(docResponse, "blaze_id", account.id);
			utils::xml_add_text_node(docResponse, "name", user->get_name());
			utils::xml_add_text_node(docResponse, "grant_online_access", account.grantOnlineAccess);
			utils::xml_add_text_node(docResponse, "cashout_bonus_time", account.cashoutBonusTime);

			if (auto docAccount = docResponse.append_child("account")) {
				account.Write(docAccount);
			}

			if (request.uri.parameter("include_creatures") == "true") {
				user->get_creatures().WriteApi(docResponse);
			}

			if (request.uri.parameter("include_decks") == "true") {
				user->WriteSquadsAPI(docResponse);
			}

			if (request.uri.parameter("include_feed") == "true") {
				user->get_feed().Write(docResponse);
			}

			if (request.uri.parameter("include_stats") == "true") {
				auto stats = docResponse.append_child("stats");
				auto stat = stats.append_child("stat");
				utils::xml_add_text_node(stat, "wins", 0);
			}
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::host, "127.0.0.1");
		response.set(boost::beast::http::field::user_agent, "Darkspore/5.3.0.127 (Pollinator; 6.2.9200)");
		response.set(boost::beast::http::field::content_language, "en-us");
		response.set(boost::beast::http::field::date, utils::get_utc_date_string());
		response.set(boost::beast::http::field::content_type, "text/xml");

		response.body() = std::move(writer.result);
	}

	void API::game_account_logout(HTTP::Session& session, HTTP::Response& response) {
		const auto& user = session.get_user();
		if (user) {
			user->Logout();
		}
	}

	void API::game_account_unlock(HTTP::Session& session, HTTP::Response& response) {
		auto& request = session.get_request();

		bool success = false;
		if (const auto& user = session.get_user()) {
			auto unlockId = request.uri.parameter<uint32_t>("unlock_id");
			success = user->UnlockUpgrade(unlockId);
		}

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			const auto& user = session.get_user();
			if (user) {
				user->get_account().Write(docResponse);
			} else {
				SporeNet::Account account;
				account.Write(docResponse);
			}

			add_common_keys(docResponse, success);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_game_getGame(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto game = docResponse.append_child("game")) {
			utils::xml_add_text_node(game, "game_id", 1);
			utils::xml_add_text_node(game, "cashed_out", 0);
			utils::xml_add_text_node(game, "finished", 0);
			utils::xml_add_text_node(game, "starting_difficulty", 1);
			utils::xml_add_text_node(game, "start", 1);
			/*
			utils::xml_add_text_node(game, "rounds", 0);
			utils::xml_add_text_node(game, "chain_id", 0);
			utils::xml_add_text_node(game, "finish", 0);
			utils::xml_add_text_node(game, "planet_id", 0);
			utils::xml_add_text_node(game, "success", 0);
			*/
			if (auto players = game.append_child("players")) {
				if (auto player = players.append_child("player")) {
					/*
					utils::xml_add_text_node(player, "deaths", 0);
					utils::xml_add_text_node(player, "kills", 0);
					*/
					utils::xml_add_text_node(player, "account_id", 1);
					utils::xml_add_text_node(player, "result", 0);
					utils::xml_add_text_node(player, "creature1_id", 1);
					utils::xml_add_text_node(player, "creature1_version", 1);
					utils::xml_add_text_node(player, "creature2_id", 0);
					utils::xml_add_text_node(player, "creature2_version", 0);
					utils::xml_add_text_node(player, "creature3_id", 0);
					utils::xml_add_text_node(player, "creature3_version", 0);
				}
			}
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_game_exitGame(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_creature_resetCreature(HTTP::Session& session, HTTP::Response& response) {
		auto& request = session.get_request();

		pugi::xml_document document;

		auto docResponse = document.append_child("response");

		const auto& user = session.get_user();
		if (user) {
			auto creature = user->GetCreatureById(request.uri.parameter<uint32_t>("id"));
			if (creature) {
				if (auto creatureNode = docResponse.append_child("creature")) {
					creature->Write(creatureNode);
				}
			} else {
				// Error, creature not found.
			}
		} else {
			// Error, not logged in.
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_creature_unlockCreature(HTTP::Session& session, HTTP::Response& response) {
		const auto& request = session.get_request();

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			auto templateId = request.uri.parameter<uint32_t>("template_id");

			const auto& user = session.get_user();
			if (user) {
				user->UnlockCreature(templateId);
			} else {
				// Send some error
			}

			utils::xml_add_text_node(docResponse, "creature_id", templateId);

			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_creature_getCreature(HTTP::Session& session, HTTP::Response& response) {
		const auto& request = session.get_request();

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			bool include_abilities = request.uri.parameter("include_abilities") == "true";
			bool include_parts = request.uri.parameter("include_parts") == "true";

			const auto& user = session.get_user();
			if (user) {
				auto creature = user->GetCreatureById(request.uri.parameter<uint32_t>("id"));
				if (creature) {
					creature->WriteApi(docResponse, true, include_abilities, include_parts);
				} else {
					// Unknown creature.
				}
			} else {
				// TODO: Error, no user logged in.
			}

			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_creature_getTemplate(HTTP::Session& session, HTTP::Response& response) {
		const auto& request = session.get_request();

		const auto& user = session.get_user();

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			bool include_abilities = request.uri.parameter("include_abilities") == "true";
			if (user) {
				auto templateCreature = SporeNet::Get().GetTemplateDatabase().Get(request.uri.parameter<uint32_t>("id"));
				if (templateCreature) {
					templateCreature->WriteApi(docResponse, include_abilities);
				} else {
					// Unknown creature.
				}
			} else {
				// TODO: Error, no user logged in.
			}

			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_language, "en-US");
		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::game_deck_updateDecks(HTTP::Session& session, HTTP::Response& response) {
		auto& request = session.get_request();

		const auto& user = session.get_user();

		pugi::xml_document document;
		if (auto docResponse = document.append_child("response")) {
			if (user) {
				user->UpdateSquad(
					request.uri.parameter<uint32_t>("pve_active_slot"),
					request.uri.parameter("pve_creatures"),
					false
				);

				user->UpdateSquad(
					request.uri.parameter<uint32_t>("pvp_active_slot"),
					request.uri.parameter("pvp_creatures"),
					true
				);
			} else {
				// TODO: Error, no user logged in.
			}

			add_common_keys(docResponse);
		}

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	/*
/game/api?version=1&token=cookie
cost = 0
gear = 0.000
id = 749013658
large = iVBORw0KGgoAAAANSUhEUgAAAQAAAAEACAYAAABccqhmAAAgAElEQVR4nOy9Z7Bl13WY+e1wwk0v
large_crc = 692908162
method = api.creature.updateCreature
parts = 117957934
points = 300.000
stats = STR,14,0;DEX,13,0;MIND,23,0;HLTH,100,70;MANA,125,23;PDEF,50,78;EDEF,150,138;CRTR,50,52
stats_ability_keyvalues = 885660025!minDamage,5;885660025!maxDamage,8;885660025!percentToHeal,20;1152331895!duration,20;1152331895!spawnMax,2;424126604!radius,8;424126604!healing,5;424126604!duration,6;424126604!minHealing,21;424126604!maxHealing,32;1577880566!Enrage.damage,9;1577880566!Enrage.duration,30;1577880566!Enrage.healing,35;1829107826!diameter,12;1829107826!damage,6;1829107826!duration,10;1829107826!speedDebuff,75
thumb = iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAgAElEQVR4nGy8adRm11kduJ9zh3f6
thumb_crc = 1921048798
token = ABCDEFGHIJKLMNOPQRSTUVWXYZ
version = 1
	*/

	void API::survey_survey_getSurveyList(HTTP::Session& session, HTTP::Response& response) {
		pugi::xml_document document;

		auto docResponse = document.append_child("response");
		if (auto surveys = docResponse.append_child("surveys")) {
			// Empty
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", pugi::format_default, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
	}

	void API::add_broadcasts(pugi::xml_node& node) {
		if (auto broadcasts = node.append_child("broadcasts")) {
			/*
			if (auto broadcast = broadcasts.append_child("broadcast")) {
				utils::xml_add_text_node(broadcast, "id", "0");
				utils::xml_add_text_node(broadcast, "start", "0");
				utils::xml_add_text_node(broadcast, "type", "0");
				utils::xml_add_text_node(broadcast, "message", "Hello World!");
				if (auto tokens = broadcast.append_child("tokens")) {
				}
			}
			*/
		}
	}

	void API::add_common_keys(pugi::xml_node& node, bool success) {
		if (success) {
			utils::xml_add_text_node(node, "code", boost::beast::http::status::ok);
			utils::xml_add_text_node(node, "stat", "ok");
			utils::xml_add_text_node(node, "result", 1);
		} else {
			utils::xml_add_text_node(node, "code", boost::beast::http::status::internal_server_error);
			utils::xml_add_text_node(node, "stat", "error");
			utils::xml_add_text_node(node, "result", 0);
		}
		// utils::xml_add_text_node(node, "version", mVersion);
		// utils::xml_add_text_node(node, "timestamp", utils::get_unix_time());
		// utils::xml_add_text_node(node, "exectime", ++mPacketId);
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
