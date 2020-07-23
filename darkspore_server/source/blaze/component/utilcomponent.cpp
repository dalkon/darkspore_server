
// Include
#include "utilcomponent.h"
#include "usersessioncomponent.h"
#include "gamemanagercomponent.h"
#include "redirectorcomponent.h"

#include "blaze/client.h"
#include "utils/functions.h"

#include <iostream>

/*
	Values in EXE for label parsing
		0x41 = Address?
*/

enum PacketID : uint16_t {
	FetchClientConfig = 0x01,
	Ping = 0x02,
	SetClientData = 0x03,
	LocalizeStrings = 0x04,
	GetTelemetryServer = 0x05,
	GetTickerServer = 0x06,
	PreAuth = 0x07,
	PostAuth = 0x08,
	UserSettingsLoad = 0x0A,
	UserSettingsSave = 0x0B,
	UserSettingsLoadAll = 0x0C,
	FilterForProfanity = 0x14,
	FetchQosConfig = 0x15,
	SetClientMetrics = 0x16,
	SetConnectionState = 0x17,
	GetPssConfig = 0x18,
	GetUserOptions = 0x19,
	SetUserOptions = 0x1A
};

// Blaze
namespace Blaze {
	// UtilComponent
	uint16_t UtilComponent::GetId() const {
		return Id;
	}

	std::string_view UtilComponent::GetName() const {
		return "Util";
	}

	std::string_view UtilComponent::GetReplyPacketName(uint16_t command) const {
		switch (static_cast<PacketID>(command)) {
			case PacketID::FetchClientConfig: return "fetchClientConfig";
			case PacketID::Ping: return "ping";
			case PacketID::SetClientData: return "setClientData";
			case PacketID::LocalizeStrings: return "localizeStrings";
			case PacketID::GetTelemetryServer: return "getTelemetryServer";
			case PacketID::GetTickerServer: return "getTickerServer";
			case PacketID::PreAuth: return "preAuth";
			case PacketID::PostAuth: return "postAuth";
			case PacketID::UserSettingsLoad: return "userSettingsLoad";
			case PacketID::UserSettingsSave: return "userSettingsSave";
			case PacketID::UserSettingsLoadAll: return "userSettingsLoadAll";
			case PacketID::FilterForProfanity: return "filterForProfanity";
			case PacketID::FetchQosConfig: return "fetchQosConfig";
			case PacketID::SetClientMetrics: return "setClientMetrics";
			case PacketID::SetConnectionState: return "setConnectionState";
			case PacketID::GetPssConfig: return "getPssConfig";
			case PacketID::GetUserOptions: return "getUserOptions";
			case PacketID::SetUserOptions: return "setUserOptions";

			default: return "";
		}
	}

	bool UtilComponent::ParsePacket(Request& request) {
		switch (request.get_command()) {
			case PacketID::FetchClientConfig:
				FetchClientConfig(request);
				break;

			case PacketID::Ping:
				Ping(request);
				break;

			case PacketID::GetTelemetryServer:
				GetTelemetryServer(request);
				break;

			case PacketID::PreAuth:
				PreAuth(request);
				break;

			case PacketID::PostAuth:
				PostAuth(request);
				break;

			case PacketID::UserSettingsSave:
				UserSettingsSave(request);
				break;

			case PacketID::UserSettingsLoadAll:
				UserSettingsLoadAll(request);
				break;

			case PacketID::SetClientMetrics:
				SetClientMetrics(request);
				break;

			default:
				return false;
		}

		return true;
	}

	void UtilComponent::WritePostAuth(TDF::Packet& packet, const PssConfig& pss, const TelemetryServer& telemetry, const TickerServer& tick, const UserOptions& options) {
		packet.push_struct("PSS");
		pss.Write(packet);
		packet.pop();

		packet.push_struct("TELE");
		telemetry.Write(packet);
		packet.pop();

		packet.push_struct("TICK");
		tick.Write(packet);
		packet.pop();

		packet.push_struct("UROP");
		options.Write(packet);
		packet.pop();
	}

	void UtilComponent::WriteGetTickerServer(TDF::Packet& packet, const TickerServer& tick) {
		tick.Write(packet);
	}

	void UtilComponent::WriteUserOptions(TDF::Packet& packet, const UserOptions& options) {
		options.Write(packet);
	}

	void UtilComponent::FetchClientConfig(Request& request) {
		std::cout << "Client " << 0 << " requested client configuration" << std::endl;
	}

	void UtilComponent::Ping(Request& request) {
		TDF::Packet packet;
		packet.put_integer("STIM", utils::get_unix_time());

		request.reply(packet);
	}

	void UtilComponent::GetTelemetryServer(Request& request) {
		TelemetryServer telemetry;
		telemetry.address = "127.0.0.1";
		telemetry.anonymous = true;
		telemetry.disa = "AD,AF,AG,AI,AL,AM,AN,AO,AQ,AR,AS,AW,AX,AZ,BA,BB,BD,BF,BH,BI,BJ,BM,BN,BO,BR,BS,BT,BV,BW,BY,BZ,CC,CD,CF,CG,CI,CK,CL,CM,CN,CO,CR,CU,CV,CX,DJ,DM,DO,DZ,EC,EG,EH,ER,ET,FJ,FK,FM,FO,GA,GD,GE,GF,GG,GH,GI,GL,GM,GN,GP,GQ,GS,GT,GU,GW,GY,HM,HN,HT,ID,IL,IM,IN,IO,IQ,IR,IS,JE,JM,JO,KE,KG,KH,KI,KM,KN,KP,KR,KW,KY,KZ,LA,LB,LC,LI,LK,LR,LS,LY,MA,MC,MD,ME,MG,MH,ML,MM,MN,MO,MP,MQ,MR,MS,MU,MV,MW,MY,MZ,NA,NC,NE,NF,NG,NI,NP,NR,NU,OM,PA,PE,PF,PG,PH,PK,PM,PN,PS,PW,PY,QA,RE,RS,RW,SA,SB,SC,SD,SG,SH,SJ,SL,SM,SN,SO,SR,ST,SV,SY,SZ,TC,TD,TF,TG,TH,TJ,TK,TL,TM,TN,TO,TT,TV,TZ,UA,UG,UM,UY,UZ,VA,VC,VE,VG,VN,VU,WF,WS,YE,YT,ZM,ZW,ZZ";
		telemetry.filter = "";
		telemetry.location = request.get_client().data().lang;
		telemetry.nook = "US,CA,MX";
		telemetry.port = 9988;
		telemetry.sdly = 15000;
		telemetry.session = "telemetry_session";
		telemetry.skey = "telemetry_key";
		telemetry.spct = 75;

		TDF::Packet packet;
		telemetry.Write(packet);

		request.reply(packet);
	}

	void UtilComponent::PreAuth(Request& request) {
		const auto& clientInfo = request["CINF"];

		auto& data = request.get_client().data();
		data.Read(request["CDAT"]);

		TDF::Packet packet;
		packet.put_string("ASRC", "321915");

		// completely unknown what this does...
		packet.push_list("CIDS", TDF::Type::Integer);
		for (auto cid : { 1, 25, 4, 27, 28, 6, 7, 9, 10, 11, 30720, 30721, 30722, 30723, 20, 30725, 30726, 2000 }) {
			packet.put_integer("", cid);
		}
		packet.pop();

		packet.push_struct("CONF");
		{
			packet.push_map("CONF", TDF::Type::String, TDF::Type::String);
#if 0
			packet.put_string("associationListSkipInitialSet", "1");
			packet.put_string("blazeServerClientId", "GOS-Darkspore-PC");
			packet.put_string("bytevaultHostname", "127.0.0.1");
			packet.put_string("bytevaultPort", "42210");
			packet.put_string("bytevaultSecure", "false");
			packet.put_string("capsStringValidationUri", "127.0.0.1");
			packet.put_string("connIdleTimeout", "90000");
			packet.put_string("defaultRequestTimeout", "60000");
			packet.put_string("identityDisplayUri", "console2/welcome");
			packet.put_string("identityRedirectUri", "http://127.0.0.1/success");
			packet.put_string("nucleusConnect", "http://127.0.0.1");
			packet.put_string("nucleusProxy", "http://127.0.0.1/");
			packet.put_string("pingPeriod", "20000");
			packet.put_string("userManagerMaxCachedUsers", "0");
			packet.put_string("voipHeadsetUpdateRate", "1000");
			packet.put_string("xblTokenUrn", "http://127.0.0.1");
			packet.put_string("xlspConnectionIdleTimeout", "300");
#else
			packet.put_string("connIdleTimeout", "90000");
			packet.put_string("defaultRequestTimeout", "80000");
			packet.put_string("pingPeriod", "20000");
			packet.put_string("voipHeadsetUpdateRate", "1000");
			packet.put_string("xlspConnectionIdleTimeout", "300");
#endif
			packet.pop();
		}
		packet.pop();

		packet.put_string("INST", data.serviceName);
		packet.put_string("NASP", "cem_ea_id");
		packet.put_string("PILD", "");
		packet.put_string("PLAT", clientInfo["PLAT"].GetString());
		
		packet.push_struct("QOSS");
		{
			// if client qos port == 0, use 7673 (but the client seems to set it to 3659 internally either way)

			QosConfigInfo qosConfig;
			qosConfig.lnp = 1;
			qosConfig.svid = 1161889797;
			
			QosPingSiteInfo& qosPingSiteInfo = qosConfig.pingSiteInfoByAlias.try_emplace("ams").first->second;
			qosPingSiteInfo.address = "127.0.0.1";
			qosPingSiteInfo.port = 17502;
			qosPingSiteInfo.name = "ams";

			qosConfig.Write(packet);
		}
		packet.pop();

		packet.put_string("RSRC", "321915");
		packet.put_string("SVER", "Blaze 3.9.3.1");

		request.reply(packet);
	}

	void UtilComponent::PostAuth(Request& request) {
		PssConfig pss;
		pss.address = "127.0.0.1";
		pss.pjid = "123071"; // Random numbers (means nothing)
		pss.port = 8443;
		pss.rprt = 9;
		pss.tiid = 0;

		TelemetryServer telemetry;
		telemetry.address = "127.0.0.1";
		telemetry.anonymous = true;
		telemetry.disa = "AD,AF,AG,AI,AL,AM,AN,AO,AQ,AR,AS,AW,AX,AZ,BA,BB,BD,BF,BH,BI,BJ,BM,BN,BO,BR,BS,BT,BV,BW,BY,BZ,CC,CD,CF,CG,CI,CK,CL,CM,CN,CO,CR,CU,CV,CX,DJ,DM,DO,DZ,EC,EG,EH,ER,ET,FJ,FK,FM,FO,GA,GD,GE,GF,GG,GH,GI,GL,GM,GN,GP,GQ,GS,GT,GU,GW,GY,HM,HN,HT,ID,IL,IM,IN,IO,IQ,IR,IS,JE,JM,JO,KE,KG,KH,KI,KM,KN,KP,KR,KW,KY,KZ,LA,LB,LC,LI,LK,LR,LS,LY,MA,MC,MD,ME,MG,MH,ML,MM,MN,MO,MP,MQ,MR,MS,MU,MV,MW,MY,MZ,NA,NC,NE,NF,NG,NI,NP,NR,NU,OM,PA,PE,PF,PG,PH,PK,PM,PN,PS,PW,PY,QA,RE,RS,RW,SA,SB,SC,SD,SG,SH,SJ,SL,SM,SN,SO,SR,ST,SV,SY,SZ,TC,TD,TF,TG,TH,TJ,TK,TL,TM,TN,TO,TT,TV,TZ,UA,UG,UM,UY,UZ,VA,VC,VE,VG,VN,VU,WF,WS,YE,YT,ZM,ZW,ZZ";
		telemetry.filter = "";
		telemetry.location = request.get_client().data().lang;
		telemetry.nook = "US,CA,MX";
		telemetry.port = 9988;
		telemetry.sdly = 15000;
		telemetry.session = "telemetry_session";
		telemetry.skey = "telemetry_key";
		telemetry.spct = 75;

		TickerServer tick;
		tick.address = "127.0.0.1";
		tick.port = 8999;
		tick.skey = "0,127.0.0.1:8999,darkspore-pc,10,50,50,50,50,0,0";

		UserOptions options;
		options.value = TelemetryOpt::OptOut;

		TDF::Packet packet;
		WritePostAuth(packet, pss, telemetry, tick, options);

		request.reply(packet);
	}

	void UtilComponent::UserSettingsSave(Request& request) {
		/*
		Log.Info(string.Format("Client {0} saving user settings for user {1}", request.Client.ID, request.Client.User.ID));

		var data = (TdfString)request.Data["DATA"];

		Directory.CreateDirectory(string.Format(".\\data\\{0}", request.Client.User.ID));

		if (File.Exists(string.Format(".\\data\\{0}\\user_settings", request.Client.User.ID)))
		{
			File.Delete(string.Format(".\\data\\{0}\\user_settings", request.Client.User.ID));
		}

		File.WriteAllBytes(string.Format(".\\data\\{0}\\user_settings", request.Client.User.ID), Encoding.ASCII.GetBytes(data.Value));

		request.Reply();
		*/
	}

	void UtilComponent::UserSettingsLoadAll(Request& request) {
		/*
		Log.Info(string.Format("Client {0} loading all user settings for user {1}", request.Client.ID, request.Client.User.ID));

        if (File.Exists(string.Format(".\\data\\{0}\\user_settings", request.Client.User.ID)))
        {
            var userSettings = File.ReadAllBytes(string.Format(".\\data\\{0}\\user_settings", request.Client.User.ID));

            var data = new List<Tdf>
            {
                new TdfMap("SMAP", TdfBaseType.String, TdfBaseType.String, new Dictionary<object, object>
                {
                    { "cust", userSettings.ToString() }
                })
            };

            request.Reply(0, data);
        }
        else
        {
            request.Reply();
        }
		*/
	}

	void UtilComponent::SetClientMetrics(Request& request) {
		// UDEV = router info?
		// USTA = upnp on or off

		request.reply();
	}
}
