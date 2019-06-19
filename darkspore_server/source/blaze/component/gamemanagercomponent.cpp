
// Include
#include "gamemanagercomponent.h"
#include "usersessioncomponent.h"

#include "../client.h"
#include "../../utils/functions.h"

#include <iostream>

/*
	Gamemodes
		Tutorial = 1
		Chain(Matched/Premade) = 2
		Arena = 3
		KillRace = 4
		Juggernaut = 5
		Quickplay = 6
		DirectEntry = 7

	Packet IDs
		0x01 = CreateGame
		0x02 = DestroyGame
		0x03 = AdvanceGameState
		0x04 = SetGameSettings
		0x05 = SetPlayerCapacity
		0x07 = SetGameAttributes
		0x08 = SetPlayerAttributes
		0x09 = JoinGame
		0x0B = RemovePlayer
		0x0D = StartMatchmaking
		0x0E = CancelMatchmaking
		0x0F = FinalizeGameCreation
		0x11 = ListGames
		0x12 = SetPlayerCustomData
		0x13 = ReplayGame
		0x14 = ReturnDedicatedServerToPool
		0x15 = JoinGameByGroup
		0x16 = LeaveGameByGroup
		0x17 = MigrateGame
		0x18 = UpdateGameHostMigrationStatus
		0x19 = ResetDedicatedServer
		0x1A = UpdateGameSession
		0x1B = BanPlayer
		0x1D = UpdateMeshConnection
		0x1F = RemovePlayerFromBannedList
		0x20 = ClearBannedList
		0x21 = GetBannedList
		0x26 = AddQueuedPlayerToGame
		0x27 = UpdateGameName
		0x64 = GetGameListSnapshot
		0x65 = GetGameListSubscription
		0x66 = DestroyGameList
		0x67 = GetFullGameData
		0x68 = GetMatchmakingConfig
		0x69 = GetGameDataFromId
		0x6A = AddAdminPlayer
		0x6B = RemoveAdminPlayer
		0x6C = SetPlayerTeam
		0x6D = ChangePlayerTeam
		0x6E = MigrateAdminPlayer
		0x6F = GetUserSetGameListSubscription
		0x96 = RegisterDynamicDedicatedServerCreator
		0x97 = UnregisterDynamicDedicatedServerCreator

	Notification Packet IDs
		0x0A = NotifyMatchmakingFailed
		0x0C = NotifyMatchmakingAsyncStatus
		0x0F = NotifyGameCreated
		0x10 = NotifyGameRemoved
		0x14 = NotifyGameSetup
		0x15 = NotifyPlayerJoining
		0x1E = NotifyPlayerJoinCompleted
		0x28 = NotifyPlayerRemoved
		0x3C = NotifyHostMigrationCompleted
		0x46 = NotifyHostMigrationStart
		0x47 = NotifyPlatformHostInitialized
		0x50 = NotifyGameAttribChange
		0x5A = NotifyPlayerAttribChange
		0x5F = NotifyPlayerCustomDataChange
		0x64 = NotifyGameStateChange
		0x6E = NotifyGameSettingsChange
		0x6F = NotifyGameCapacityChange
		0x70 = NotifyGameReset
		0x71 = NotifyGameReportingIdChange
		0x73 = NotifyGameSessionUpdated
		0x74 = NotifyGamePlayerStateChange
		0x75 = NotifyGamePlayerTeamChange
		0x76 = NotifyGameTeamIdChange
		0x77 = NotifyProcessQueue
		0xC9 = NotifyGameListUpdate
		0xCA = NotifyAdminListChange
		0xDC = NotifyCreateDynamicDedicatedServerGame
		0xE6 = NotifyGameNameChange

	Blaze Fields
		PDAT
			BLOB = 0x20
			EXID = 0x30
			GID = 0x38
			LOC = 0x38
			NAME = 0x24
			PATT = 0x54
			PID = 0x34
			PNET = 0x14
			SID = 0x48
			SLOT = 0x1C
			STAT = 0x1C
			TIDX = 0x40
			TIME = 0x34
			UGID = 0x08
			UID = 0x28

		ReplicatedPlayerData
			BLOB = 0x20
			EXID = 0x30
			GID = 0x38
			LOC = 0x38
			NAME = 0x24
			PATT = 0x54
			PID = 0x34
			PNET = 0x14
			SID = 0x48
			SLOT = 0x1C
			STAT = 0x1C
			TIDX = 0x40
			TIME = 0x34
			UGID = 0x08
			UID = 0x38

		ReplicatedGameData
			ADMN = 0x58
			ATTR = 0x54
			CAP = 0x58
			CRIT = 0x54
			GID = 0x38
			GNAM = 0x24
			GPVH = 0x30
			GSET = 0x28
			GSID = 0x30
			GSTA = 0x1C
			GTYP = 0x24
			HNET = 0x58
			HSES = 0x38
			IGNO = 0x50
			MATR = 0x54
			MCAP = 0x40
			NQOS = 0x18
			NRES = 0x50
			NTOP = 0x1C
			PGID = 0x24
			PGSR = 0x20
			PHST = 0x18
			PRES = 0x1C
			PSAS = 0x24
			QCAP = 0x40
			SEED = 0x38
			TCAP = 0x40
			THST = 0x18
			TIDS = 0x58
			UUID = 0x24
			VOIP = 0x1C
			VSTR = 0x24
			XNNC = 0x20
			XSES = 0x20

		IndirectJoinGameSetupContext
			GRID = 0x08
			RPVC = 0x50

		ResetDedicatedServerSetupContext
			ERR = 0x38

		GameBrowserGameData
			ADMN = 0x58
			ATTR = 0x54
			CAP = 0x58
			CRIT = 0x54
			GID = 0x38
			GNAM = 0x24
			GSET = 0x28
			GSTA = 0x1C
			HNET = 0x58
			HOST = 0x34
			PCNT = 0x58
			PRES = 0x1C
			PSID = 0x24
			QCAP = 0x40
			QCNT = 0x40
			ROST = 0x58
			SID = 0x30
			TCAP = 0x40
			TINF = 0x58
			VOIP = 0x1C
			VSTR = 0x24

	Request Packets
		CreateGame
			ADMN = 0x58
			ATTR = 0x54
			BTPL = 0x08
			CRIT = 0x54
			GCTR = 0x24
			GNAM = 0x24
			GSET = 0x28
			GTYP = 0x24
			GURL = 0x24
			HNET = 0x58
			IGNO = 0x50
			MATR = 0x54
			NRES = 0x50
			NTOP = 0x1C
			PATT = 0x54
			PCAP = 0x58
			PGID = 0x24
			PGSC = 0x20
			PMAX = 0x40
			PRES = 0x1C
			QCAP = 0x40
			RGID = 0x38
			SEAT = 0x58
			SIDL = 0x58
			SLOT = 0x1C
			TCAP = 0x40
			TIDS = 0x58
			TIDX = 0x40
			VOIP = 0x1C
			VSTR = 0x24

		JoinGame
			ATTR = 0x54
			BTPL = 0x08
			GID = 0x38
			GVER = 0x24
			JMET = 0x1C
			PNET = 0x14
			SLID = 0x48
			SLOT = 0x1C
			TIDX = 0x40
			USER = 0x18

		ResetDedicatedServer
			same as CreateGame?

	Response Packets
		CreateGame
			

		DestroyGame
			GID = 0x38

		JoinGame
			GID = 0x38

	Notify Packets
		NotifyGameCreated
			GID = 0x38

		NotifyGameRemoved


		NotifyGameSetup
			GAME = 0x18
			PROS = 0x58
			REAS = 0x14

		NotifyHostMigrationCompleted
			GID = 0x38

		NotifyCreateDynamicDedicatedServerGame
			GREQ = 0x18
			MID = 0x24

		NotifyPlatformHostInitialized
			GID = 0x38
			PHST = 0x48
*/

// Blaze
namespace Blaze {
	// GameManagerComponent
	void GameManagerComponent::Parse(Client* client, const Header& header) {
		switch (header.command) {
			case 0x01:
				CreateGame(client, header);
				break;

			case 0x09:
				JoinGame(client, header);
				break;

			case 0x19:
				ResetDedicatedServer(client, header);
				break;

			default:
				std::cout << "Unknown gamemanager command: 0x" << std::hex << header.command << std::dec << std::endl;
				break;
		}
	}

	void GameManagerComponent::SendCreateGame(Client* client, uint32_t gameId) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", gameId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x01;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void GameManagerComponent::SendJoinGame(Client* client, uint32_t gameId) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", gameId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x09;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void GameManagerComponent::SendStartMatchmaking(Client* client, uint32_t id) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "MSID", id);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x0D;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyGameCreated(Client* client) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", 1);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x0F;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyGameRemoved(Client* client) {

	}

	void GameManagerComponent::NotifyGameSetup(Client* client) {
		auto& request = client->get_request();

		std::string gameName = request["GNAM"].GetString();
		uint32_t gameSet = request["GSET"].GetUint();
		std::string gameType = request["GTYP"].GetString();
		bool gameIgnore = request["IGNO"].GetUint();

		bool networkResetable = request["NRES"].GetUint();
		uint32_t networkTopology = request["NTOP"].GetUint();

		uint32_t presenceMode = request["PRES"].GetUint();
		uint32_t queueCapacity = request["QCAP"].GetUint();

		uint32_t tCap = request["TCAP"].GetUint();
		uint32_t voipTopology = request["VOIP"].GetUint();
		std::string versionString = request["VSTR"].GetString();

		TDF::Packet packet;
		{
			auto& gameStruct = packet.CreateStruct(nullptr, "GAME");
			{
				auto& admnList = packet.CreateList(&gameStruct, "ADMN", TDF::Type::Integer);
				packet.PutInteger(&admnList, "", 1);
			} {
				auto& attrMap = packet.CreateMap(&gameStruct, "ATTR", TDF::Type::String, TDF::Type::String);
				packet.PutString(&attrMap, "ExpectedPlayerCount", "1");
				packet.PutString(&attrMap, "GameType", "1");
				packet.PutString(&attrMap, "Ranked", "0");
				packet.PutString(&attrMap, "SelectedDifficulty", "1");
			} {
				auto& capList = packet.CreateList(&gameStruct, "CAP", TDF::Type::Integer);
				packet.PutInteger(&capList, "", 1);
				packet.PutInteger(&capList, "", 0);
			}
			packet.PutInteger(&gameStruct, "GID", 1);
			packet.PutString(&gameStruct, "GNAM", gameName);
			packet.PutInteger(&gameStruct, "GPVH", 666);
			packet.PutInteger(&gameStruct, "GSET", gameSet);
			packet.PutInteger(&gameStruct, "GSID", 1);
			packet.PutInteger(&gameStruct, "GSTA", static_cast<uint32_t>(GameState::Initializing));
			packet.PutString(&gameStruct, "GTYP", gameType);
			{
				const auto& hnetData = request["HNET"]["_Content"];

				auto& hnetList = packet.CreateList(&gameStruct, "HNET", TDF::Type::Struct, true);
				{
					auto& exipStruct = packet.CreateStruct(&hnetList, "");
					packet.PutInteger(&exipStruct, "IP", hnetData[0]["IP"].GetUint());
					packet.PutInteger(&exipStruct, "PORT", hnetData[0]["PORT"].GetUint());
				} {
					auto& inipStruct = packet.CreateStruct(&hnetList, "");
					packet.PutInteger(&inipStruct, "IP", hnetData[1]["IP"].GetUint());
					packet.PutInteger(&inipStruct, "PORT", hnetData[1]["PORT"].GetUint());
				}
			}
			packet.PutInteger(&gameStruct, "HSES", 13666);
			packet.PutInteger(&gameStruct, "IGNO", gameIgnore);
			packet.PutInteger(&gameStruct, "MCAP", 1);
			{
				auto& nqosStruct = packet.CreateStruct(&gameStruct, "NQOS");
				packet.PutInteger(&nqosStruct, "DBPS", 100);
				packet.PutInteger(&nqosStruct, "NATT", static_cast<uint32_t>(NatType::Open));
				packet.PutInteger(&nqosStruct, "UBPS", 100);
			}
			packet.PutInteger(&gameStruct, "NRES", networkResetable);
			packet.PutInteger(&gameStruct, "NTOP", networkTopology);
			packet.PutString(&gameStruct, "PGID", "d4c9d072-24d7-4142-b648-b43c8fe97b4d"); // b6852db1-ba37-4b40-aea3-0bd16efba4f9
			packet.PutBlob(&gameStruct, "PGSR", nullptr, 0);
			{
				auto& phstStruct = packet.CreateStruct(&gameStruct, "PHST");
				packet.PutInteger(&phstStruct, "HPID", 1);
				packet.PutInteger(&phstStruct, "HSLT", 1);
			}
			packet.PutInteger(&gameStruct, "PRES", presenceMode);
			packet.PutString(&gameStruct, "PSAS", "ams");
			packet.PutInteger(&gameStruct, "QCAP", queueCapacity);
			packet.PutInteger(&gameStruct, "SEED", 2291);
			packet.PutInteger(&gameStruct, "TCAP", tCap);
			{
				auto& thstStruct = packet.CreateStruct(&gameStruct, "THST");
				packet.PutInteger(&thstStruct, "HPID", 1);
				packet.PutInteger(&thstStruct, "HSLT", 0);
			} {
				auto& tidsList = packet.CreateList(&gameStruct, "TIDS", TDF::Type::Integer);
				packet.PutInteger(&tidsList, "", 1);
			}
			packet.PutString(&gameStruct, "UUID", "71bc4bdb-82ec-494d-8d75-ca5123b827ac"); // 714b05dc-93bc-49ac-961c-cb38b574f30a
			packet.PutInteger(&gameStruct, "VOIP", voipTopology);
			packet.PutString(&gameStruct, "VSTR", versionString);
			packet.PutBlob(&gameStruct, "XNNC", nullptr, 0);
			packet.PutBlob(&gameStruct, "XSES", nullptr, 0);
		} {
			auto& prosList = packet.CreateList(nullptr, "PROS", TDF::Type::Struct);
			{
				auto& playerStruct = packet.CreateStruct(&prosList, "");
				packet.PutBlob(&playerStruct, "BLOB", nullptr, 0);
				packet.PutInteger(&playerStruct, "EXID", 0);
				packet.PutInteger(&playerStruct, "GID", 1);
				packet.PutInteger(&playerStruct, "LOC", 1);
				packet.PutString(&playerStruct, "NAME", "Dalkon");
				{
					// PATT
				}
				packet.PutInteger(&playerStruct, "PID", 1);
				{
					// PNET (union)
				}
				packet.PutInteger(&playerStruct, "SID", 0);
				packet.PutInteger(&playerStruct, "SLOT", 0);
				packet.PutInteger(&playerStruct, "STAT", 0);
				packet.PutInteger(&playerStruct, "TIDX", 0xFFFF);
				packet.PutInteger(&playerStruct, "TIME", utils::get_unix_time());
				packet.PutVector3(&playerStruct, "UGID", 0, 0, 0);
				packet.PutInteger(&playerStruct, "UID", 1);
			}
		} {
			auto& reasUnion = packet.CreateUnion(nullptr, "REAS", NetworkAddressMember::XboxClientAddress);
			auto& valuStruct = packet.CreateStruct(&reasUnion, "VALU");
			packet.PutInteger(&valuStruct, "DCTX", 0);
		}

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x14;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyPlayerJoining(Client* client) {
		auto& request = client->get_request();

		uint32_t gameId = request["GID"].GetUint();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", gameId);
		{
			auto& pdatStruct = packet.CreateStruct(nullptr, "PDAT");
			packet.PutBlob(&pdatStruct, "BLOB", nullptr, 0);
			packet.PutInteger(&pdatStruct, "EXID", 0);
			packet.PutInteger(&pdatStruct, "LOC", request["CINF"]["LOC"].GetUint64());
			packet.PutString(&pdatStruct, "NAME", "Dalkon");
			{
				auto& pattMap = packet.CreateMap(&pdatStruct, "PATT", TDF::Type::String, TDF::Type::String);
				packet.PutString(&pattMap, "Premium", "False");
			}
			packet.PutInteger(&pdatStruct, "PID", 1);
			{
				const auto& valuData = request["PNET"]["VALU"];

				auto& pnetUnion = packet.CreateUnion(nullptr, "PNET", NetworkAddressMember::IPPAirAddress);
				auto& valuStruct = packet.CreateStruct(&pnetUnion, "VALU");
				{
					auto& exipStruct = packet.CreateStruct(&pnetUnion, "EXIP");
					packet.PutInteger(&exipStruct, "IP", valuData["EXIP"]["IP"].GetUint());
					packet.PutInteger(&exipStruct, "PORT", valuData["EXIP"]["PORT"].GetUint());
				} {
					auto& inipStruct = packet.CreateStruct(&pnetUnion, "INIP");
					packet.PutInteger(&inipStruct, "IP", valuData["INIP"]["IP"].GetUint());
					packet.PutInteger(&inipStruct, "PORT", valuData["INIP"]["PORT"].GetUint());
				}
			}
			packet.PutInteger(&pdatStruct, "SID", 0);
			packet.PutInteger(&pdatStruct, "SLOT", 0);
			packet.PutInteger(&pdatStruct, "STAT", 0);
			packet.PutInteger(&pdatStruct, "TIDX", 0xFFFF);
			packet.PutInteger(&pdatStruct, "TIME", 0);
			packet.PutVector3(&pdatStruct, "UGID", 0, 0, 0);
			packet.PutInteger(&pdatStruct, "UID", 1);
		}
		/*
			new TdfStruct("PDAT", new List<Tdf>
			{
				new TdfInteger("EXID", 0),
				new TdfInteger("GID", client.GameID),
				new TdfInteger("LOC", client.Localization),
				new TdfString("NAME", client.User.Name),
				new TdfMap("PATT", TdfBaseType.String, TdfBaseType.String, new Dictionary<object, object>
				{
					{ "Premium", "False" }
				}),
				new TdfInteger("PID", client.User.ID),
				new TdfUnion("PNET", NetworkAddressMember.IPPAirAddress, new List<Tdf>
				{
					new TdfStruct("VALU", new List<Tdf>
					{
						new TdfStruct("EXIP", new List<Tdf>
						{
							new TdfInteger("IP", client.ExternalIP),
							new TdfInteger("PORT", client.ExternalPort)
						}),
						new TdfStruct("INIP", new List<Tdf>
						{
							new TdfInteger("IP", client.InternalIP),
							new TdfInteger("PORT", client.InternalPort)
						})
					})
				}),
				new TdfInteger("SID", (ulong)slotID),
				new TdfInteger("SLOT", 0),
				new TdfInteger("STAT", 0),
				new TdfInteger("TIDX", 65535),
				new TdfInteger("TIME", 0),
				new TdfVector3("UGID", 0, 0, 0),
				new TdfInteger("UID", (ulong)client.ID)
			})
		*/
		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x15;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyPlayerJoinCompleted(Client* client) {
		auto& request = client->get_request();
		Log(request);

		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", request["GID"].GetUint());
		packet.PutInteger(nullptr, "PID", 1);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x1E;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyGameStateChange(Client* client, uint64_t gameId, uint64_t gameState) {
		auto& request = client->get_request();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "GID", gameId);
		packet.PutInteger(nullptr, "GSTA", gameState);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0x64;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::NotifyCreateDynamicDedicatedServerGame(Client* client) {
		// Sending this should make the client send back a CreateGameRequest or ResetDedicatedServer

		auto& request = client->get_request();

		std::string gameCtr = request["GCTR"].GetString();
		std::string gameName = request["GNAM"].GetString();
		uint32_t gameSet = request["GSET"].GetUint();
		std::string gameType = request["GTYP"].GetString();
		std::string gameUrl = request["GURL"].GetString();
		bool gameIgnore = request["IGNO"].GetUint();

		bool networkResetable = request["NRES"].GetUint();
		uint32_t networkTopology = request["NTOP"].GetUint();

		std::string playerGameId = request["PGID"].GetString();
		uint32_t playerMax = request["PMAX"].GetUint();

		uint32_t presenceMode = request["PRES"].GetUint();
		uint32_t queueCapacity = request["QCAP"].GetUint();

		uint32_t rgId = request["RGID"].GetUint();
		uint32_t slot = request["SLOT"].GetUint();
		uint32_t tidx = request["TIDX"].GetUint();
		uint32_t tCap = request["TCAP"].GetUint();
		uint32_t voipTopology = request["VOIP"].GetUint();
		std::string versionString = request["VSTR"].GetString();

		TDF::Packet packet;
		{
			/*
			PATT = 0x54
			*/
			auto& greqStruct = packet.CreateStruct(nullptr, "GREQ");
			{
				auto& admnList = packet.CreateList(&greqStruct, "ADMN", TDF::Type::Integer);
				packet.PutInteger(&admnList, "", 1);
			} {
				auto& attrMap = packet.CreateMap(&greqStruct, "ATTR", TDF::Type::String, TDF::Type::String);
				packet.PutString(&attrMap, "ExpectedPlayerCount", "1");
				packet.PutString(&attrMap, "GameType", "1");
				packet.PutString(&attrMap, "Ranked", "0");
				packet.PutString(&attrMap, "SelectedDifficulty", "1");
			}
			packet.PutVector3(&greqStruct, "BTPL", 0, 0, 0);
			packet.PutString(&greqStruct, "GCTR", gameCtr);
			packet.PutString(&greqStruct, "GNAM", gameName);
			packet.PutInteger(&greqStruct, "GSET", gameSet);
			packet.PutString(&greqStruct, "GTYP", gameType);
			packet.PutString(&greqStruct, "GURL", gameUrl);
			{
				const auto& hnetData = request["HNET"]["_Content"];

				auto& hnetList = packet.CreateList(&greqStruct, "HNET", TDF::Type::Struct, true);
				{
					auto& exipStruct = packet.CreateStruct(&hnetList, "");
					packet.PutInteger(&exipStruct, "IP", hnetData[0]["IP"].GetUint());
					packet.PutInteger(&exipStruct, "PORT", hnetData[0]["PORT"].GetUint());
				} {
					auto& inipStruct = packet.CreateStruct(&hnetList, "");
					packet.PutInteger(&inipStruct, "IP", hnetData[1]["IP"].GetUint());
					packet.PutInteger(&inipStruct, "PORT", hnetData[1]["PORT"].GetUint());
				}
			}
			packet.PutInteger(&greqStruct, "IGNO", gameIgnore);
			packet.PutInteger(&greqStruct, "NRES", networkResetable);
			packet.PutInteger(&greqStruct, "NTOP", networkTopology);
			packet.PutString(&greqStruct, "PGID", playerGameId);
			packet.PutBlob(&greqStruct, "PGSC", nullptr, 0);
			{
				auto& capList = packet.CreateList(&greqStruct, "PCAP", TDF::Type::Integer);
				packet.PutInteger(&capList, "", 1);
				packet.PutInteger(&capList, "", 0);
			}
			packet.PutInteger(&greqStruct, "PMAX", playerMax);
			packet.PutInteger(&greqStruct, "PRES", presenceMode);
			packet.PutInteger(&greqStruct, "QCAP", queueCapacity);
			packet.PutInteger(&greqStruct, "RGID", rgId);
			{
				auto& sidlList = packet.CreateList(&greqStruct, "SIDL", TDF::Type::Integer);
				packet.PutInteger(&sidlList, "", 1);
			} {
				auto& seatList = packet.CreateList(&greqStruct, "SEAT", TDF::Type::Integer);
				packet.PutInteger(&seatList, "", 1);
			}
			packet.PutInteger(&greqStruct, "SLOT", slot);
			packet.PutInteger(&greqStruct, "TCAP", tCap);
			{
				auto& tidsList = packet.CreateList(&greqStruct, "TIDS", TDF::Type::Integer);
				packet.PutInteger(&tidsList, "", 1);
			}
			packet.PutInteger(&greqStruct, "TIDX", tidx);
			packet.PutInteger(&greqStruct, "VOIP", voipTopology);
			packet.PutString(&greqStruct, "VSTR", versionString);
		}
		packet.PutString(nullptr, "MID", "matchmaking_id");

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::GameManager;
		header.command = 0xDC;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void GameManagerComponent::CreateGame(Client* client, Header header) {
		SendCreateGame(client, 1);

		NotifyGameStateChange(client, 1, static_cast<uint32_t>(GameState::Initializing));
		NotifyGameSetup(client);
	}

	void GameManagerComponent::JoinGame(Client* client, Header header) {
		auto& request = client->get_request();
		SendJoinGame(client, request["GID"].GetUint());

		UserSessionComponent::NotifyUserAdded(client, 1, "Dalkon");
		UserSessionComponent::NotifyUserUpdated(client, 1);

		NotifyPlayerJoining(client);
	}

	void GameManagerComponent::ResetDedicatedServer(Client* client, Header header) {
		// Log(client->get_current_request());
		SendCreateGame(client, 1);
		
		// NotifyCreateDynamicDedicatedServerGame(client);
		NotifyGameCreated(client);
		NotifyGameStateChange(client, 1, static_cast<uint32_t>(GameState::Initializing));
		NotifyGameSetup(client);
	}
}
