
// Include
#include "api.h"
#include "config.h"

#include "../main.h"

#include "../http/session.h"
#include "../http/router.h"
#include "../http/uri.h"
#include "../utils/functions.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <boost/beast/version.hpp>

#include <iostream>

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
		router->add("/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [](HTTP::Response& response, const HTTP::URI& uri) {
			std::cout << "Got API route." << std::endl;
		});

		// DLS
		router->add("/dls/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response & response, const HTTP::URI & uri) {
			auto method = uri.parameter("method");
			if (method == "api.launcher.setTheme") {
				dls_launcher_setTheme(response, uri);
			} else if (method == "api.launcher.listThemes") {
				dls_launcher_listThemes(response, uri);
			} else {
				response.result() = boost::beast::http::status::internal_server_error;
			}
		});

		// Launcher
		router->add("/bootstrap/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response & response, const HTTP::URI & uri) {
			auto version = uri.parameter("version");
			auto build = uri.parameter("build");
			auto method = uri.parameter("method");

			if (version.empty()) { version = "1"; }
			if (build.empty()) { build = "5.3.0.103"; }
			if (method.empty()) { method = "api.config.getConfigs"; }

			if (method.find("api.config.") == 0) {
				if (method == "api.config.getConfigs") {
					bootstrap_config_getConfig(response, uri);
				}
			}
		});

		router->add("/bootstrap/launcher/", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response & response, const HTTP::URI & uri) {
			auto version = uri.parameter("version");
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

		router->add("/bootstrap/launcher/images/([a-zA-Z0-9_.]+)", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, const HTTP::URI& uri) {
			const std::string& resource = uri.resource();

			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_THEMES_PATH) +
				"default/images/" +
				resource.substr(resource.rfind('/') + 1);

			response.version() |= 0x1000'0000;
			response.body() = std::move(path);
		});

		router->add("/bootstrap/launcher/notes", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response & response, const HTTP::URI & uri) {
			std::string path = Config::Get(CONFIG_STORAGE_PATH) +
				"www/" +
				Config::Get(CONFIG_DARKSPORE_LAUNCHER_NOTES_PATH);

			std::string file_data = utils::get_file_text(path);
			utils::string_replace(file_data, "{{dls-version}}", "0.1");
			utils::string_replace(file_data, "{{version-lock}}", Config::GetBool(CONFIG_VERSION_LOCKED) ? "5.3.0.103" : "no");
			utils::string_replace(file_data, "{{game-mode}}", Config::GetBool(CONFIG_SINGLEPLAYER_ONLY) ? "singleplayer" : "multiplayer");
			utils::string_replace(file_data, "{{display-latest-version}}", "none");
			utils::string_replace(file_data, "{{latest-version}}", "yes");

			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = std::move(file_data);
		});
		/*
		@app.route("/bootstrap/launcher/notes")
			def bootstrapLauncherNotes() :
			notesPath = serverConfig.darksporeLauncherNotesPath()
			file = open(pathUtils.join(pathUtils.join(serverConfig.storagePath(), 'www'), notesPath), "r")
			launcherNotesHtml = file.read()

			versionLocked = serverConfig.versionLockEnabled()
			singleplayer = serverConfig.singlePlayerOnly()
			isLatest = (server.gameVersion == darksporeBuild_latestOfficial)

			launcherNotesHtml = launcherNotesHtml.replace("{{dls-version}}", server.version)
			launcherNotesHtml = launcherNotesHtml.replace("{{version-lock}}", server.gameVersion if versionLocked else "no")
			launcherNotesHtml = launcherNotesHtml.replace("{{game-mode}}", "singleplayer" if singleplayer else "multiplayer")
			launcherNotesHtml = launcherNotesHtml.replace("{{display-latest-version}}", "block" if versionLocked else "none")
			launcherNotesHtml = launcherNotesHtml.replace("{{latest-version}}", "yes" if isLatest else "no")

			return Response(launcherNotesHtml, mimetype = 'text/html')
		*/
		// Game
		router->add("/game/api", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, const HTTP::URI& uri) {
			auto version = uri.parameter("version");
			auto build = uri.parameter("build");
			auto method = uri.parameter("method");

			if (method == "api.status.getStatus") {
				game_status_getStatus(response, uri);
			} else {
				empty_xml_response(response);
			}
		});

		// Web
		router->add("/web/sporelabsgame/announceen", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, const HTTP::URI& uri) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = "<html><head><title>x</title></head><body>Announces</body></html>";
		});

		router->add("/web/sporelabsgame/persona", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, const HTTP::URI& uri) {
			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = R"(
<!DOCTYPE html PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
	<head>
		<script type="text/javascript">
			window.onload = function() {
				Client.closeBRSWindow();
			}
		</script>
	</head>
	<body>
	</body>
</html>
)";
		});

		// QOS
		router->add("/qos/qos", { boost::beast::http::verb::get, boost::beast::http::verb::post }, [this](HTTP::Response& response, const HTTP::URI& uri) {
			auto version = uri.parameter("vers");
			auto type = uri.parameter("qtyp");
			auto prompt = uri.parameter("prpt");

			response.set(boost::beast::http::field::content_type, "text/html");
			response.body() = "<html><head><title>x</title></head><body>Announces</body></html>";
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
				add_text_node(config, "blaze_secure", "N");
				add_text_node(config, "blaze_env", "production");
				add_text_node(config, "sporenet_cdn_host", host);
				add_text_node(config, "sporenet_db_host", host);
				add_text_node(config, "sporenet_db_port", "80");
				add_text_node(config, "sporenet_db_name", "darkspore");
				add_text_node(config, "sporenet_host", host);
				add_text_node(config, "sporenet_port", "80");
				add_text_node(config, "http_secure", "N");
				add_text_node(config, "liferay_host", host);
				add_text_node(config, "launcher_action", "2");
				add_text_node(config, "launcher_url", "http://" + host + "/bootstrap/launcher/?version=" + mVersion);
			}
		}

		docResponse.append_child("to_image");
		docResponse.append_child("from_image");

		if (uri.parameterb("include_settings")) {
			if (auto settings = docResponse.append_child("settings")) {
				add_text_node(settings, "open", "false");
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
			docResponse.append_child("broadcasts");
		}

		add_common_keys(docResponse);

		xml_string_writer writer;
		document.save(writer, "\t", 1U, pugi::encoding_latin1);

		response.set(boost::beast::http::field::content_type, "text/xml");
		response.body() = std::move(writer.result);
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
