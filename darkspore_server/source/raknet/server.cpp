
// Include
#include "server.h"

#include "sporenet/instance.h"
#include "sporenet/user.h"
#include "sporenet/creature.h"

#include "utils/functions.h"
#include "game/instance.h"

#include <MessageIdentifiers.h>
#include <RakSleep.h>
#include <RakNetTypes.h>
#include <RakNetworkFactory.h>
#include <BitStream.h>
#include <GetTime.h>

#include <time.h>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <array>

/*

Spaceship state:
	[19:23:30.818] cTransport: registered message handler for kGmsConnected(130)[cb=0x26AAB298]
	[19:23:30.819] cTransport: registered message handler for kGmsHelloPlayer(128)[cb=0x26AAC6B8]
	[19:23:30.823] cTransport: registered message handler for kGmsPlayerJoined(132)[cb=0x26BFC968]
	[19:23:30.824] cTransport: registered message handler for kGmsPlayerDeparted(134)[cb=0x26BB23E0]

	[19:23:30.826] cClientSession: state kSessionStateIdle => kSessionStateConnecting
	[19:23:30.827] cTransport: registered message handler for kGmsGameState(138)[cb=0x1122620]
	[19:23:30.828] cTransport: registered message handler for kGmsDirectorState(139)[cb=0x1122620]
	[19:23:30.831] cTransport: registered message handler for kGmsObjectCreate(140)[cb=0x1122620]
	[19:23:30.833] cTransport: registered message handler for kGmsObjectUpdate(141)[cb=0x1122620]
	[19:23:30.835] cTransport: registered message handler for kGmsObjectDelete(142)[cb=0x1122620]
	[19:23:30.837] cTransport: registered message handler for kGmsObjectJump(143)[cb=0x1122620]
	[19:23:30.838] cTransport: registered message handler for kGmsObjectTeleport(144)[cb=0x1122620]
	[19:23:30.841] cTransport: registered message handler for kGmsObjectPlayerMove(145)[cb=0x1122620]
	[19:23:30.843] cTransport: registered message handler for kGmsForcePhysicsUpdate(146)[cb=0x1122620]
	[19:23:30.845] cTransport: registered message handler for kGmsPhysicsChanged(147)[cb=0x1122620]
	[19:23:30.847] cTransport: registered message handler for kGmsLocomotionDataUpdate(148)[cb=0x1122620]
	[19:23:30.849] cTransport: registered message handler for kGmsLocomotionDataUnreliableUpdate(149)[cb=0x1122620]
	[19:23:30.852] cTransport: registered message handler for kGmsAttributeDataUpdate(150)[cb=0x1122620]
	[19:23:30.854] cTransport: registered message handler for kGmsCombatantDataUpdate(151)[cb=0x1122620]
	[19:23:30.856] cTransport: registered message handler for kGmsInteractableDataUpdate(152)[cb=0x1122620]
	[19:23:30.858] cTransport: registered message handler for kGmsAgentBlackboardUpdate(153)[cb=0x1122620]
	[19:23:30.861] cTransport: registered message handler for kGmsLootDataUpdate(154)[cb=0x1122620]
	[19:23:30.863] cTransport: registered message handler for kGmsLabsPlayerUpdate(161)[cb=0x1122620]
	[19:23:30.865] cTransport: registered message handler for kGmsServerEvent(155)[cb=0x1122620]
	[19:23:30.867] cTransport: registered message handler for kGmsModifierCreated(162)[cb=0x1122620]
	[19:23:30.869] cTransport: registered message handler for kGmsModifierUpdated(163)[cb=0x1122620]
	[19:23:30.872] cTransport: registered message handler for kGmsModifierDeleted(164)[cb=0x1122620]
	[19:23:30.874] cTransport: registered message handler for kGmsGamePrepareForStart(176)[cb=0x1122620]
	[19:23:07.302] cTransport: registered message handler for kGmsGameStart(177)[cb=0x1122620]
	[19:23:30.879] cTransport: registered message handler for kGmsSetAnimationState(165)[cb=0x1122620]
	[19:23:30.883] cTransport: registered message handler for kGmsSetObjectGfxState(166)[cb=0x1122620]
	[19:23:30.885] cTransport: registered message handler for kGmsPlayerCharacterDeploy(167)[cb=0x1122620]
	[19:23:30.886] cTransport: registered message handler for kGmsActionCommandResponse(168)[cb=0x1122620]
	[19:23:30.888] cTransport: registered message handler for kGmsObjectivesInitForLevel(183)[cb=0x1122620]
	[19:23:30.892] cTransport: registered message handler for kGmsObjectiveUpdated(184)[cb=0x1122620]
	[19:23:30.893] cTransport: registered message handler for kGmsObjectivesComplete(185)[cb=0x1122620]
	[19:23:30.895] cTransport: registered message handler for kGmsObjectiveAdd(202)[cb=0x1122620]
	[19:23:30.897] cTransport: registered message handler for kGmsCombatEvent(186)[cb=0x1122620]
	[19:23:30.899] cTransport: registered message handler for kGmsReloadLevel(191)[cb=0x1122620]
	[19:23:30.902] cTransport: registered message handler for kGmsGravityForceUpdate(192)[cb=0x1122620]
	[19:23:30.904] cTransport: registered message handler for kGmsCooldownUpdate(193)[cb=0x1122620]
	[19:23:30.906] cTransport: registered message handler for kGmsCrystalMessage(195)[cb=0x1122620]
	[19:23:30.907] cTransport: registered message handler for kGmsVoteKickStarted(135)[cb=0x1122620]
	[19:23:30.910] cTransport: registered message handler for kGmsPartyMergeComplete(133)[cb=0x1122620]
	[19:23:30.913] cTransport: registered message handler for kGmsDebugPing(204)[cb=0x1122620]

ChainVote state:
	[19:23:31.501] cTransport: registered message handler for kGmsChainVoteGMSMsgs(169)[cb=0x11B507D8]

PreDungeon state:
	[19:23:31.577] cTransport: registered message handler for kGmsArenaGameGMSMsgs(181)[cb=0x11B55DE8]

Dungeon state:
	[19:23:35.119] cTransport: registered message handler for kGmsChainGameGMSMsgs(173)[cb=0x11B64DC8]
	[19:23:35.121] cTransport: registered message handler for kGmsQuickGameGMSMsgs(175)[cb=0x11B73238]
	[19:23:35.123] cTransport: registered message handler for kGmsTutorialGameGMSMsgs(200)[cb=0x11B3B368]
	[19:23:35.124] cTransport: registered message handler for kGmsArenaGameGMSMsgs(181)[cb=0x11B65598]
	[19:23:35.126] cTransport: registered message handler for kGmsJuggernautGameGMSMsgs(189)[cb=0x11B47308]
	[19:23:35.129] cTransport: registered message handler for kGmsKillRaceGameGMSMsgs(198)[cb=0x11DEE328]
	[19:23:35.130] cTransport: registered message handler for kGmsCinematicGMSMsgs(201)[cb=0x11BEAC40]
*/

/*
	C2S or C2C
		???

	S2C or C2C
		ID_CONNECTION_REQUEST,
		ID_CONNECTION_REQUEST_ACCEPTED,
		ID_CONNECTION_ATTEMPT_FAILED,
		ID_ALREADY_CONNECTED, (should never happen?)
		ID_NEW_INCOMING_CONNECTION,
		ID_DISCONNECTION_NOTIFICATION,
		ID_CONNECTION_LOST,
		ID_MODIFIED_PACKET, (should never happen?)
		ID_REMOTE_DISCONNECTION_NOTIFICATION, (unexpected in client)
		ID_REMOTE_CONNECTION_LOST, (unexpected in client)
		ID_REMOTE_NEW_INCOMING_CONNECTION (unexpected in client)

	Client packet reliability
		UNRELIABLE
		UNRELIABLE_SEQUENCED
		RELIABLE
		RELIABLE_ORDERED

	Client packet priority
		HIGH_PRIORITY
		MEDIUM_PRIORITY (default)
		LOW_PRIORITY
*/

// Abilities
/*
namespace Ability {
	enum : uint32_t {
		Strength = 0,
		Dexterity,
		Mind,
		MaxHealthIncrease,
		MaxHealth,
		MaxMana,
		DamageReduction,
		PhysicalDefense,
		PhysicalDamageReduction,
		EnergyDefense,
		CriticalRating,
		NonCombatSpeed,
		CombatSpeed,
		DamageBuff,
		Silence,
		Immobilized,
		DefenseBoostBasicDamage,
		PhysicalDamageIncrease,
		PhysicalDamageIncreaseFlat,
		AutoCrit,
		BehindDirectDamageIncrease,
		BehindOrSideDirectDamageIncrease,
		CriticalDamageIncrease,
		AttackSpeedScale,
		CooldownScale,
		Frozen,
		ProjectileSpeedIncrease,
		AoeResistance,
		EnergyDamageBuff,
		Intangible,
		HealingReduction,
		EnergyDamageIncrease,
		EnergyDamageIncreaseFlat,
		Immune,
		StealthDetection,
		LifeSteal,
		RejectModifier,
		AoeDamage,
		TechnologyTypeDamage,
		SpacetimeTypeDamage,
		LifeTypeDamage,
		ElementsTypeDamage,
		SupernaturalTypeDamage,
		TechnologyTypeResistance,
		SpacetimeTypeResistance,
		LifeTypeResistance,
		ElementsTypeResistance,
		SupernaturalTypeResistance,
		MovementSpeedBuff,
		ImmuneToDebuffs,
		BuffDuration,
		DebuffDuration,
		ManaSteal,
		DebuffDurationIncrease,
		EnergyDamageReduction,
		Incorporeal,
		DotDamageIncrease,
		MindControlled,
		SwapDisabled,
		ImmuneToRandomTeleport,
		ImmuneToBanish,
		ImmuneToKnockback,
		AoeRadius,
		PetDamage,
		PetHealth,
		CrystalFind,
		DNADropped,
		RangeIncrease,
		OrbEffectiveness,
		OverdriveBuildup,
		OverdriveDuration,
		LootFind,
		Surefooted,
		ImmuneToStunned,
		ImmuneToSleep,
		ImmuneToTerrified,
		ImmuneToSilence,
		ImmuneToCursed,
		ImmuneToPoisonOrDisease,
		ImmuneToBurning,
		ImmuneToRooted,
		ImmuneToSlow,
		ImmuneToPull,
		DotDamageDoneIncrease,
		AggroIncrease,
		AggroDecrease,
		PhysicalDamageDoneIncrease,
		PhysicalDamageDoneByAbilityIncrease,
		EnergyDamageDoneIncrease,
		EnergyDamageDoneByAbilityIncrease,
		ChannelTimeDecrease,
		CrowdControlDurationDecrease,
		DotDurationDecrease,
		AoeDurationIncrease,
		HealIncrease,
		OnLockdown,
		HoTDoneIncrease,
		ProjectileDamageIncrease,
		DeployBonusInvincibilityTime,
		PhysicalDamageDecreaseFlat,
		EnergyDamageDecreaseFlat,
		MinWeaponDamage,
		MaxWeaponDamage,
		MinWeaponDamagePercent,
		MaxWeaponDamagePercent,
		DirectAttackDamage,
		DirectAttackDamagePercent,
		GetHitAnimDisabled,
		XPBoost,
		InvisibleToSecurityTeleporters,
		BodyScale,
		Count
	};
}
*/

// RakNet
namespace RakNet {
	/*
		Mapped Transport packets
			

		Mapped GMS packets
			FirstRegMessage
				GameState
				DirectorState
				ObjectCreate
				ObjectUpdate
				ObjectDelete
				ObjectJump
				ObjectTeleport
				ObjectPlayerMove
				ForcePhysicsUpdate
				PhysicsChanged
				LocomotionDataUpdate
				LocomotionUnreliableDataUpdate
				CombatantDataUpdate
				InteractableDataUpdate
				AgentBlackboardUpdate
				LootDataUpdate
				LabsPlayerUpdate
				ServerEvent
				ModifierCreated
				ModifierUpdated
				ModifierDeleted
				GamePrepareForStart
				GameStart
				SetAnimationState
				SetObjectGfxState
				PlayerCharacterDeploy
				ActionCommandResponse
				ObjectivesInitForLevel
				ObjectiveUpdated
				ObjectivesComplete
				ObjectiveAdd
				CombatEvent
				GravityForceUpdate
				CooldownUpdate
				CrystalMessage
				VoteKickStarted
				PartyMergeComplete
				DebugPing

			sub_451080 (observer?)
				ArenaGameMsgs
				JuggernautGameMsgs
				KillRaceGameMsgs

			sub_453C20, sub_452D50 (unreg?)
				ChainGameMsgs
				QuickGameMsgs
				TutorialGameMsgs
				ArenaGameMsgs
				JuggernautGameMsgs
				KillRaceGameMsgs
				CinematicMsgs

			sub_454890, sub_4544E0
				JuggernautResultsMsgs

			sub_454DD0
				KillRaceLobbyMsgs

			sub_454F30, sub_454B10
				KillRaceResultsMsgs

			sub_516C00, sub_516230
				ReconnectPlayer

			sub_52FA20, sub_52F800
				ArenaGameMsgs

			sub_9C1A40
				PlayerStatusUpdate
				ActionCommandMsgs
				CrystalDragMessage
				LootDropMessage
				DebugPing

			TryConnect
				Connected
				HelloPlayer
				PlayerJoined
				PlayerDeparted


	*/

	void WriteDebugData(RakNet::BitStream& stream, size_t length) {
		constexpr std::array<uint8_t, 8> bytes { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
		constexpr size_t bytesMask = bytes.size() - 1;
		for (size_t i = 0; i < length; ++i) {
			Write<uint8_t>(stream, bytes[i & bytesMask]);
		}
	}

	void PrintDebugStream(RakNet::BitStream& stream) {
		auto bytes = stream.GetNumberOfUnreadBits() / 8;
		auto bytesMinusOne = bytes - 1;

		uint8_t value;
		for (uint32_t i = 0; i < bytes; ++i) {
			stream.Read<uint8_t>(value);
			std::cout << "0x" << std::hex << (int)value << std::dec;
			if (i != bytesMinusOne) {
				std::cout << ", ";
			}
		}

		std::cout << std::endl;
	}

	enum GType : uint8_t {
		Unknown = 0,
		Tutorial,				// 0x01
		Chain,					// 0x02
		Arena,					// 0x03
		KillRace,				// 0x04
		Juggernaut,				// 0x05
		QuickPlay,				// 0x06
		DirectEntry				// 0x07
	};

	enum GState : uint32_t {
		Invalid = 0xFFFFFFFF,
		Start = 0,

		Boot = Start,			// 0x00
		Login,					// 0x01
		Spaceship,				// 0x02
		Editor,					// 0x03
		LevelEditor,			// 0x04
		PreDungeon,				// 0x05
		Dungeon,				// 0x06
		Observer,				// 0x07
		Cinematic,				// 0x08
		Spectator,				// 0x09
		Replay,					// 0x0A
		ChainVoting,			// 0x0B
		ChainCashOut,			// 0x0C
		GameOver,				// 0x0D
		Quit,					// 0x0E
		ArenaLobby,				// 0x0F
		ArenaRoundResults,		// 0x10
		JuggernautLobby,		// 0x11
		JuggernautResults,		// 0x12
		KillRaceLobby,			// 0x13
		KillRaceResults,		// 0x14

		Count					// 0x15
	};

	// ObjectiveData
	void ObjectiveData::WriteTo(RakNet::BitStream& stream) const {
		constexpr size_t maxDescriptionLength = 0x30;
		Write<uint32_t>(stream, objectiveHash);

		Write<uint8_t>(stream, value0);
		Write<uint8_t>(stream, value1);
		Write<uint8_t>(stream, value2);
		Write<uint8_t>(stream, value3);

		size_t length = std::min<size_t>(maxDescriptionLength, description.length());
		size_t padding = maxDescriptionLength - length;

		for (size_t i = 0; i < length; ++i) { Write<char>(stream, description[i]); }
		for (size_t i = 0; i < padding; ++i) { Write<char>(stream, 0x00); }
	}

	// Server
	Server::Server(Game::Instance& game) : mGame(game) {
		// Nothing?
	}

	Server::~Server() {
		stop();
	}

	void Server::start(uint16_t port) {
		mThread = std::thread([this, port] {
			mSelf = RakNetworkFactory::GetRakPeerInterface();
			mSelf->SetTimeoutTime(30000, UNASSIGNED_SYSTEM_ADDRESS);
#ifdef PACKET_LOGGING
			mSelf->AttachPlugin(&mLogger);
#endif

			auto socketDescriptor = SocketDescriptor(port, nullptr);
			if (mSelf->Startup(4, 30, &socketDescriptor, 1)) {
				mSelf->SetMaximumIncomingConnections(4);
				mSelf->SetOccasionalPing(true);
				mSelf->SetUnreliableTimeout(1000);
				while (is_running()) {
					run_one();
					RakSleep(30);
				}
			}

			mSelf->Shutdown(300);
			RakNetworkFactory::DestroyRakPeerInterface(mSelf);
		});
	}

	void Server::stop() {
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mRunning = false;
		}

		if (mThread.joinable()) {
			mThread.join();
		}
	}

	bool Server::is_running() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mRunning;
	}

	void Server::run_one() {
		const auto GetPacketIdentifier = [this]() -> MessageID {
			uint8_t message;
			if (mInStream.GetData()) {
				mInStream.Read<uint8_t>(message);
				if (message == ID_TIMESTAMP) {
					constexpr auto timestampSize = sizeof(MessageID) + sizeof(RakNetTime);
					RakAssert((mInStream.GetNumberOfUnreadBits() >> 3) > timestampSize);

					mInStream.IgnoreBytes(timestampSize - 1);
					mInStream.Read<uint8_t>(message);
				}
			} else {
				message = 0xFF;
			}
			return message;
		};

		for (Packet* packet = mSelf->Receive(); packet; mSelf->DeallocatePacket(packet), packet = mSelf->Receive()) {
			size_t packetSize = packet->bitSize / 8;
			mInStream = BitStream(packet->data, packetSize, false);

			uint8_t packetType = GetPacketIdentifier();
			std::cout << std::endl << "--- packet: "  << (int)packetType << ", length: " << packetSize << " gotten from raknet ---" << std::endl << std::endl;

			if (packetType < ID_USER_PACKET_ENUM) {
				ParseRakNetPackets(packet, packetType);
			} else {
				ParseSporeNetPackets(packet, packetType);
			}
		}

		// TODO: something to make sure things happen
		// mGame.Update();

		mInStream.Reset();
	}

	Game::Instance& Server::GetGame() {
		return mGame;
	}

	void Server::ParseRakNetPackets(Packet* packet, uint8_t packetType) {
		switch (packetType) {
			case ID_DISCONNECTION_NOTIFICATION: {
				RemoveClient(packet);
				std::cout << "ID_DISCONNECTION_NOTIFICATION from " << packet->systemAddress.ToString(true) << std::endl;
				break;
			}

			case ID_NEW_INCOMING_CONNECTION: {
				OnNewIncomingConnection(packet);
				break;
			}

			case ID_CONNECTION_REQUEST: {
				std::cout << "Trying to connect to RakNet" << std::endl;
				break;
			}

			case ID_INCOMPATIBLE_PROTOCOL_VERSION: {
				std::cout << "ID_INCOMPATIBLE_PROTOCOL_VERSION" << std::endl;
				break;
			}

			case ID_CONNECTION_LOST: {
				// TODO: try to reconnect?
				RemoveClient(packet);
				std::cout << "ID_CONNECTION_LOST from " << packet->systemAddress.ToString(true) << std::endl;
				break;
			}

			case ID_SND_RECEIPT_ACKED: {
				// Packet was successfully accepted.
				break;
			}

			case ID_SND_RECEIPT_LOSS: {
				// Packet was dropped. Add code to resend?
				break;
			}

			default: {
				std::cout << "Unhandled RakNet packet: " << (int)packetType << std::endl;
				break;
			}
		}
	}

	void Server::ParseSporeNetPackets(Packet* packet, uint8_t packetType) {
		auto client = GetClient(packet);
		if (!client) {
			// Drop connection?
			return;
		}

		switch (packetType) {
			case PacketID::HelloPlayerRequest: {
				OnHelloPlayerRequest(client);
				break;
			}

			case PacketID::PlayerStatusUpdate: {
				OnPlayerStatusUpdate(client);
				break;
			}

			case PacketID::ActionCommandMsgs: {
				OnActionCommandMsgs(client);
				break;
			}

			case PacketID::ChainPlayerMsgs: {
				OnChainPlayerMsgs(client);
				break;
			}

			case PacketID::CrystalDragMessage: {
				OnCrystalDragMessage(client);
				break;
			}

			case PacketID::DebugPing: {
				OnDebugPing(client);
				break;
			}

			default: {
				std::cout << "Unknown packet id: 0x" << std::hex << static_cast<int32_t>(packetType) << std::dec << std::endl;
				break;
			}
		}
	}

	ClientPtr Server::AddClient(Packet* packet) {
		// TODO: check for duplicated, cant bother atm
		if (packet) {
			auto it = mClients.try_emplace(packet->systemAddress);
			if (it.second) {
				auto client = std::make_shared<Client>(*this, it.first->first);
				client->mId = static_cast<uint8_t>(mClients.size() - 1);

				it.first->second = std::move(client);
			}
			return it.first->second;
		}
		return nullptr;
	}

	void Server::RemoveClient(Packet* packet) {
		if (packet) {
			mClients.erase(packet->systemAddress);
		}
	}

	ClientPtr Server::GetClient(Packet* packet) const {
		if (packet) {
			auto it = mClients.find(packet->systemAddress);
			if (it != mClients.end()) {
				return it->second;
			}
		}
		return nullptr;
	}

	void Server::Send(BitStream& stream, const ClientPtr& client) {
		mSelf->Send(&stream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, client->mSystemAddress, false);
	}

	void Server::OnNewIncomingConnection(Packet* packet) {
		const auto& client = AddClient(packet);
		if (client) {
			client->SetGameState(GameState::Spaceship);

			PrintDebugStream(mInStream);

			std::cout << "Player connected!" << std::endl;
			SendConnected(client);
		} else {
			std::cout << "OnNewIncomingConnection: Unknown client tried to connect." << std::endl;
		}
	}

	void Server::OnHelloPlayerRequest(const ClientPtr& client) {
		int64_t blazeId;
		Read(mInStream, blazeId);

		const auto& user = SporeNet::Get().GetUserManager().GetUserByAuthToken(std::to_string(blazeId));
		if (!user) {
			// Unknown user
			return;
		}

		client->SetBlazeId(blazeId);

		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		std::cout << "Hello " << user->get_name() << "! " << std::endl;

		// Initial player data (move somewhere else later)
		/*
			creature_png = string.format("%d_%d_thumb.png", character.assetID, character.version);
		*/

		// Use sporenet user data
		player->Setup(user);

		// Crystals
		player->SetCrystal(labsCrystal(labsCrystal::AoEDamage, 0, false), 0);
		player->SetCrystal(labsCrystal(labsCrystal::DamageAura, 1, false), 1);
		player->SetCrystal(labsCrystal(labsCrystal::ImmuneSleep, 2, false), 2);
		player->SetCrystal(labsCrystal(labsCrystal::AttackSpeed, 1, true), 3);

		// Game state data... I think?
		auto& gameStateData = client->GetGameStateData();
		gameStateData.var = 0.0;
		gameStateData.state = static_cast<uint32_t>(GameState::Spaceship);
		gameStateData.type = Blaze::GameType::Chain;

		SendHelloPlayer(client);
		SendDebugPing(client);

		/*
		data=
		cryos_1_v2_Audio;
		cryos_1_v2_VFX;
		cryos_1_v2_ai;
		cryos_1_v2_animation_common;
		cryos_1_v2_animators;
		cryos_1_v2_audio_ambient;
		cryos_1_v2_default;
		cryos_1_v2_design;
		cryos_1_v2_design_blocks;
		cryos_1_v2_environment;
		cryos_1_v2_levelshop;
		cryos_1_v2_lights;
		cryos_1_v2_objects;
		cryos_1_v2_plants;
		cryos_1_v2_smartobjects;
		cryos_1_v2_trees;
		*/
	}

	void Server::OnPlayerStatusUpdate(const ClientPtr& client) {
		uint32_t value;
		Read<uint32_t>(mInStream, value);

		float progress;
		Read<float>(mInStream, progress);

		const auto& player = client->GetPlayer();
		player->SetStatus(value, progress);

		std::cout << "Player Status Update: " << value << ", " << progress << std::endl;

		/*
			order in darkspore.exe
				0, 1, 2, 4
				3, 5, 6, 7, 8
		*/
		
		SendLabsPlayerUpdate(client, player);
		switch (value) {
			case 0x02: {
				ObjectiveData objectiveData;
				objectiveData.objectiveHash = utils::hash_id("DontUseHealthObelisks");
				objectiveData.value0 = 0x01;
				objectiveData.value1 = 0x02;
				objectiveData.value2 = 0x03;
				objectiveData.value3 = 0x04;
				objectiveData.description = "Do some stuff bruh";

				SendDirectorState(client);
				SendObjectivesInitForLevel(client, { objectiveData });

				for (uint32_t i = 0; i < 3; ++i) {
					const auto& characterObject = player->GetCharacterObject(i);
					if (characterObject) {
						characterObject->SetPosition(cSPVector3(i * 2, 0, 0)); // test
						SendObjectCreate(client, characterObject);
					}
				}

				break;
			}

			case 0x04: {
				// SendDebugPing(client);

				// This causes the next state to activate... but also disconnects you
				/*
				ServerEvent event;
				event.objectId = 0x0000000A;
				event.ServerEventDef = utils::hash_id("character_beam_in_plasma_flame.serverEventDef");

				SendServerEvent(client, std::move(event));
				*/
				// SendPlayerDeparted(client);

				/*
				auto& gameStateData = client->GetGameStateData();
				gameStateData.var0 = 0x31;
				gameStateData.var1 = 0x25;
				SendGameState(packet, gameStateData);
				*/
				// SendDebugPing(packet);

				/*
				// SendArenaGameMessages(packet);
				SendObjectivesInitForLevel(packet);
				SendPlayerDeparted(packet);
				*/
				break;
			}

			case 0x08: {
				// SendPlayerJoined(client);

				// At this point "StartLevel" has already run
				// SendDirectorState(client);
				SendGameStart(client);
				SendDebugPing(client);

				// SendObjectCreate(client, 0x0000000A, static_cast<uint32_t>(Game::CreatureID::BlitzAlpha));
				// SendPlayerCharacterDeploy(client, 0x0000000A);
				
				break;
			}

			case 0x20: {
				// OnBeamOut clicked.
			}
		}
	}

	void Server::OnActionCommandMsgs(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			// No player? Get lost.
			return;
		}

		std::cout << "OnActionCommandMsgs" << std::endl;

		uint8_t type;
		Read<uint8_t>(mInStream, type);

		switch (type) {
			case 4: // Cinematic?
				/*
				0x0, 0xbe, 0x11, 0x0, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0xe8, 0x62, 0x21, 0x12, 0x60, 0x62, 0x21, 0x12, 0x53, 0x2b, 0x9c, 0x0, 0x70, 0xfc, 0x19, 0x0, 0x80, 0xfc, 0x19, 0x0
				*/
				std::cout << "Cinematic?" << std::endl;
				break;


			// Switch hero
			case 5: {
				std::cout << "Switch hero" << std::endl;

				uint32_t tmp32;
				uint16_t tmp16;

				uint8_t swapCount;
				Read<uint8_t>(mInStream, swapCount);

				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint16_t>(mInStream, tmp16);

				uint32_t creatureIndex;
				Read<uint32_t>(mInStream, creatureIndex);

				SendPlayerCharacterDeploy(client, player, creatureIndex);
				// SendActionCommandResponse(client);

				/*
				0xf,
				0xb8, 0x40, 0x3c,
				0x1,
				0xb7, 0x40, 0x0,
				0x0, 0x0, 0x0, 0x0,
				0x40, 0xfe, 0x7f, 0xb8,
				0xec, 0x19, 0x0, 0xa7,
				0x67, 0xe, 0x77, 0xcc,
				0x7a, 0x2a, 0x12, 0x1,
				0x0, 0x0, 0x0, 0x88,
				0x7b, 0x2a, 0x12, 0x0,
				0x7b, 0x2a, 0x12,
				0x1, 0x0, 0x0, 0x0
				*/

				break;
			}

			case 10: {
				uint32_t tmp32;
				uint16_t tmp16;
				uint8_t tmp8;

				Read<uint16_t>(mInStream, tmp16);
				Read<uint8_t>(mInStream, tmp8);

				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);
				Read<uint32_t>(mInStream, tmp32);

				cSPVector3 position;
				position.ReadFrom(mInStream);

				std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
				std::cout << bswap(position.x) << ", " << bswap(position.y) << ", " << bswap(position.z) << std::endl;

				/*
				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xdc, 0x2f, 0x1a, 0x77, 0x4c, 0xae, 0xf3, 0x76, 0x98, 0xfe, 0x19, 0x0

				Same pos?
				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x68, 0xfc, 0x19, 0x0, 0x78, 0xfc, 0x19, 0x0, 0x98, 0xfe, 0x19, 0x0

				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x68, 0xfc, 0x19, 0x0, 0x78, 0xfc, 0x19, 0x0, 0x98, 0xfe, 0x19, 0x0
				*/
				break;
			}

			default:
				break;
		}

		/*

		Hero "2"
		0x2, 0xd, 0x15, 0x80,
		0x45, 0xd, 0x15, 0x0,
		0x0, 0x0, 0x0, 0x69,
		0x78, 0xdf, 0xc4, 0x0,
		0xd0, 0xc7, 0x17, 0x0,
		0x0, 0x0, 0x0, 0x7b,
		0x52, 0x34, 0x3f, 0xc,
		0xaa, 0x3b, 0xbf, 0xcc,
		0x89, 0x30, 0xc2, 0xc,
		0x9a, 0x1b, 0xc4, 0x1,
		0x0, 0x0, 0x0

		0x2, 0x2, 0x15, 0x80, 0x45, 0x2, 0x15, 0x0, 0x0, 0x0, 0x0, 0x33, 0x75, 0xdf, 0xc4, 0x0, 0xd0, 0xc7, 0x17, 0x0, 0x0, 0x0, 0x0, 0xdc, 0x54, 0x34, 0x3f, 0x12, 0xc7, 0x3b, 0xbf, 0x42, 0xb2, 0x30, 0xc2, 0xad, 0x79, 0x1b, 0xc4, 0x1, 0x0, 0x0, 0x0

		Hero "3"
		0x3, 0xd, 0x15, 0x80, 0x45, 0xd, 0x15, 0x0, 0x0, 0x0, 0x0, 0x69, 0x78, 0xdf, 0xc4, 0x0, 0xd0, 0xc7, 0x17, 0x0, 0x0, 0x0, 0x0, 0x7b, 0x52, 0x34, 0x3f, 0xc, 0xaa, 0x3b, 0xbf, 0xcc, 0x89, 0x30, 0xc2, 0xc, 0x9a, 0x1b, 0xc4, 0x2, 0x0, 0x0, 0x0

		0x3, 0x2, 0x15, 0x80, 0x45, 0x2, 0x15, 0x0, 0x0, 0x0, 0x0, 0x69, 0x78, 0xdf, 0xc4, 0x0, 0xd0, 0xc7, 0x17, 0x0, 0x0, 0x0, 0x0, 0x7b, 0x52, 0x34, 0x3f, 0xc, 0xaa, 0x3b, 0xbf, 0xcc, 0x89, 0x30, 0xc2, 0xc, 0x9a, 0x1b, 0xc4, 0x2, 0x0, 0x0, 0x0

		*/

		std::cout << "OnActionCommandMsgs(" << static_cast<int>(type) << ")" << std::endl;
		PrintDebugStream(mInStream);

		/*
		uint32_t value;
		for (size_t i = 0; i < 16; ++i) {
			mInStream.Read<uint32_t>(value);
			for (size_t j = 0; j < 4; ++j) {
				std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(reinterpret_cast<uint8_t*>(&value)[j]) << " ";
			}
		}
		std::cout << std::resetiosflags(0) << std::endl;
		*/
	}

	void Server::OnChainPlayerMsgs(const ClientPtr& client) {
		auto bytesToRead = mInStream.GetNumberOfUnreadBits() / 8;

		uint8_t value;
		Read<uint8_t>(mInStream, value);

		std::cout << "OnChainPlayerMsgs(" << bytesToRead << "): " << (int)value << std::endl;

		switch (bytesToRead) {
			case 1: {
				if (value == 0) {
					SendChainVoteMessages(client, value);
					SendChainVoteMessages(client, 1);
				} else if (value == 2) {
					// Cashout
					SendChainVoteMessages(client, value);
				}
				break;
			}

			case 2: {
				// unknown so far
				break;
			}

			case 6: {
				uint8_t unknown;
				Read<uint8_t>(mInStream, unknown);

				uint32_t squadId;
				Read<uint32_t>(mInStream, squadId);

				PrepareGameStart(client, unknown, squadId);
				break;
			}
		}
	}

	void Server::OnCrystalDragMessage(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		uint32_t crystalSlot;
		Read<uint32_t>(mInStream, crystalSlot);

		uint32_t unk;
		Read<uint32_t>(mInStream, unk);

		float x, y, z, w;
		Read<float>(mInStream, x);
		Read<float>(mInStream, y);
		Read<float>(mInStream, z);
		Read<float>(mInStream, w);

		std::cout << "-- CrystalDragMessage --" << std::endl;
		std::cout << crystalSlot << "(" << unk << ") : " << x << ", " << y << ", " << z << ", " << w << std::endl;
		std::cout << std::endl << "------------------------" << std::endl;

		// 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x20, 0x41, 0xbd, 0x17, 0x90, 0x42, 0xbd, 0x17, 0xc4, 0xa, 0xa2, 0x17, 0xc8, 0x1a, 0xc8, 0x11
	}
	
	void Server::OnDebugPing(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		uint64_t time;
		Read<uint64_t>(mInStream, time);

		std::string timeString(0x20, '\0');
		if (const auto* timePtr = _gmtime64(reinterpret_cast<const __time64_t*>(&time))) {
			strftime(&timeString[0], 0x20, "%H:%M:%S", timePtr);
		} else {
			timeString = std::to_string(time);
		}

		std::cout << "Debug ping: " << timeString << std::endl;

		// Schedule other packets?
		switch (client->GetGameState()) {
			case GameState::Spaceship: {
				// SendLabsPlayerUpdate(client, client->GetPlayer());
				SendGameState(client, client->GetGameStateData());
				SendReconnectPlayer(client, GameState::ChainVoting);
				break;
			}

			case GameState::ChainVoting: {
				SendGamePrepareForStart(client);
				SendDebugPing(client);
				break;
			}

			case GameState::PreDungeon:
				SendLabsPlayerUpdate(client, player);
				// SendArenaGameMessages(client);
				// SendReconnectPlayer(client, GameState::Dungeon);
				// SendDebugPing(client);
				break;

			case GameState::Dungeon:
				for (uint32_t i = 0; i < 3; ++i) {
					const auto& characterObject = player->GetCharacterObject(i);
					if (characterObject) {
						characterObject->SetPosition(cSPVector3(i * 2, 0, 0));
						SendObjectCreate(client, characterObject);
					}
				}

				SendDirectorState(client);
				SendPlayerCharacterDeploy(client, player, 0);

				// SendTutorial(client);
				// SendObjectTeleport(client);
				// SendArenaGameMessages(client);
				break;
		}
	}

	void Server::PrepareGameStart(const ClientPtr& client, uint8_t unknown, uint32_t squadId) {
		auto blazeId = client->GetBlazeId();

		const auto& player = mGame.GetPlayer(blazeId);
		if (!player) {
			// Should never happen.
			return;
		}

		const auto& user = SporeNet::Get().GetUserManager().GetUserById(blazeId);
		if (!user) {
			// Nope, go away.
			return;
		}

		const auto& squad = user->GetSquadById(squadId);
		if (!squad) {
			// Should not happen.
			return;
		}

		auto& playerData = player->GetData();
		player->SetStatus(0, 0.f);

		uint32_t creatureIndex = 0;
		for (uint32_t creatureId : squad->GetCreatureIds()) {
			labsCharacter character;

			// TODO: save/calculate health and "power"
			character.mHealthPoints = 100.0f;
			character.mMaxHealthPoints = 200.0f;
			character.mManaPoints = 100.0f;
			character.mMaxManaPoints = 200.0f;

			const auto& creature = user->GetCreatureById(creatureId);
			if (creature) {
				character.nounDef = creature->GetNoun();
				character.version = creature->GetVersion();
				character.mCreatureType = static_cast<uint32_t>(creature->GetType());
				character.mGearScore = creature->GetGearScore();
				character.mGearScoreFlattened = creature->GetGearScoreFlattened();
			} else {
				character.nounDef = 0;
				character.version = 0;
				character.mCreatureType = static_cast<uint32_t>(SporeNet::CreatureType::Unknown);
				character.mGearScore = 0.f;
				character.mGearScoreFlattened = 0.f;
			}

			player->SetCharacter(std::move(character), creatureIndex);
			creatureIndex++;
		}

		SendDebugPing(client);
	}

	void Server::SendHelloPlayer(const ClientPtr& client) {
		// Packet size: 0x08
		const auto& guid = mSelf->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);

		auto addr = mSelf->GetSystemAddressFromGuid(guid);
		if (1) { // add check for local
			addr = SystemAddress("127.0.0.1", addr.port);
		}

		/*
			u8: type
			u8: gameIndex (player id in game)
			u32: ip
			u16: port
		*/

		// uint8_t type = static_cast<uint8_t>(Blaze::GameType::Tutorial);

		// Does this value determine if you are the host?
		uint8_t type = 0; // This makes darkspore log "I AM PLAYER client->mId" internally
		
		BitStream outStream(8);
		outStream.Write(PacketID::HelloPlayer);
		outStream.Write<uint8_t>(type);
		outStream.Write<uint8_t>(client->mId);
		outStream.WriteBits(reinterpret_cast<const uint8_t*>(&addr.binaryAddress), sizeof(addr.binaryAddress) * 8, true);
		outStream.Write(addr.port);

		Send(outStream, client);
	}

	void Server::SendReconnectPlayer(const ClientPtr& client, GameState gameState) {
		if (client->SetGameState(gameState)) {
			BitStream outStream(8);
			outStream.Write(PacketID::ReconnectPlayer);

			Write<uint32_t>(outStream, static_cast<uint32_t>(gameState));

			Send(outStream, client);
		}
	}

	void Server::SendConnected(const ClientPtr& client) {
		// TODO: verify incoming connection

		BitStream outStream(8);
		outStream.Write(PacketID::Connected);

		Send(outStream, client);
	}

	void Server::SendPlayerJoined(const ClientPtr& client) {
		// Packet size: 0x01
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerJoined);
		outStream.Write<uint8_t>(client->mId);

		Send(outStream, client);
	}

	void Server::SendPlayerDeparted(const ClientPtr& client) {
		// Packet size: 0x01
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerDeparted);
		outStream.Write<uint8_t>(client->mId);

		Send(outStream, client);
	}

	void Server::SendPlayerStatusUpdate(const ClientPtr& client, uint8_t playerState) {
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerStatusUpdate);
		outStream.Write<uint8_t>(playerState);

		Send(outStream, client);
	}

	void Server::SendGameState(const ClientPtr& client, const GameStateData& data) {
		// data in here seems to only be used in spectator, dungeon, cinematic, tutorial and some other state

		// arg0 and arg1 is moved to other values in sub_9D79C0

		// Packet size: 0x19
		BitStream outStream(8);
		outStream.Write(PacketID::GameState);

		// sub_9D7830
		// this is probably double
		Write<double>(outStream, data.var);
		// Write<uint32_t>(outStream, data.var0);		// mov [eax+30h], value0, an address? used in sub_9D7A70
		// Write<uint32_t>(outStream, data.var1);	// mov [eax+34h], value1, an address?, cmp'd against
		// sub_9D79C0 sets these 2 values to +38h and +3Ch

		// sub_9C1B30(GET), sub_9C1B50(SET) - game objective completion time
		Write<uint64_t>(outStream, utils::get_unix_time() + (15 * 60 * 1000));

		Write<uint8_t>(outStream, static_cast<uint8_t>(data.state)); // mov [simulator+18h], value2 (default: 0)
		
		// sub_9C16F0
		Write<uint32_t>(outStream, static_cast<uint32_t>(data.type));		// mov [simulator+3B420h], value (default: 0xFFFFFFFF)

		// sub_9C16D0 - unused?
		Write<uint32_t>(outStream, 1);			// mov [simulator+3B41Ch], value (default: 0)

		Send(outStream, client);
	}

	void Server::SendLabsPlayerUpdate(const ClientPtr& client, const Game::PlayerPtr& player) {
		if (!player) {
			return;
		}

		uint16_t updateBits = player->GetUpdateBits();
		if (updateBits == 0) {
			return;
		}

		player->ResetUpdateBits();

		BitStream outStream(8);
		outStream.Write(PacketID::LabsPlayerUpdate);

		// Player ID?
		Write<uint8_t>(outStream, player->GetId());
		Write<uint16_t>(outStream, updateBits);

		// write player
		auto& playerData = player->GetData();
		if (updateBits & labsPlayerBits::PlayerBits) {
			playerData.WriteReflection(outStream);
		}

		// write characters
		if (updateBits & labsPlayerBits::CharacterMask) {
			// possibly send objectCreate or objectUpdate aswell

			const auto& characters = playerData.mCharacters;
			for (size_t i = 0; i < characters.size(); ++i) {
				if (updateBits & (labsPlayerBits::CharacterBits << i)) {
					characters[i].WriteReflection(outStream);
				}
			}
		}

		// write crystals
		if (updateBits & labsPlayerBits::CrystalMask) {
			// same as for characters

			const auto& crystals = playerData.mCrystals;
			for (size_t i = 0; i < crystals.size(); ++i) {
				if (updateBits & (labsPlayerBits::CrystalBits << i)) {
					crystals[i].WriteReflection(outStream);
				}
			}
		}

		playerData.ResetUpdateBits();

		Send(outStream, client);
	}

	void Server::SendDirectorState(const ClientPtr& client) {
		// Packet size: reflection(variable)
		BitStream outStream(8);
		outStream.Write(PacketID::DirectorState);

		cAIDirector director;
		director.mBossId = 0;
		director.mActiveHordeWaves = 0;
		director.mbBossSpawned = false;
		director.mbBossHorde = false;
		director.mbCaptainSpawned = false;
		director.mbBossComplete = false;
		director.mbHordeSpawned = false;
		director.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendPlayerCharacterDeploy(const ClientPtr& client, const Game::PlayerPtr& player, uint32_t creatureIndex) {
		if (!player) {
			return;
		}

		const auto& characterObject = player->GetCharacterObject(creatureIndex);
		if (!characterObject) {
			return;
		}

		// Packet size: 0x09
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerCharacterDeploy);

		Write<uint8_t>(outStream, player->GetId());
		Write<uint32_t>(outStream, creatureIndex);
		Write<uint32_t>(outStream, characterObject->GetId()); // tObjID, object id

		Send(outStream, client);
	}

	void Server::SendObjectCreate(const ClientPtr& client, const Game::ObjectPtr& object) {
		if (!object) {
			return;
		}

		auto id = object->GetId();

		const auto& data = object->GetData();

		// Packet data
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectCreate);

		// set object id to 1?
		Write<uint32_t>(outStream, id); // is this correct?

		// object creation data
		cGameObjectCreateData createData;
		createData.noun = object->GetNoun();
		createData.position = data.mPosition;
		createData.rotXDegrees = 0; // TODO: quaternion -> euler
		createData.rotYDegrees = 0;
		createData.rotZDegrees = 0;
		createData.assetId = id;
		createData.scale = data.mScale;
		createData.team = data.mTeam;
		createData.hasCollision = data.mbHasCollision;
		createData.playerControlled = data.mbPlayerControlled;

		// write reflection of creation and the object
		createData.WriteReflection(outStream);
		data.WriteReflection(outStream);

		// write update data (same as in ObjectUpdate
		// is the u8 value here "team"?
		/*
#if 1
		Write<uint8_t>(outStream, 0x00);
		Write<uint32_t>(outStream, createData.noun);

		// at u8 (0x19) this makes it crash because it assumes a memory address in the client
		Write<uint32_t>(outStream, 0x00);

		// position?
		createData.position.WriteTo(outStream);
#else
		for (uint8_t id = 0; id <= 0; ++id) {
			Write<uint8_t>(outStream, id);
			Write<uint32_t>(outStream, createData.noun);
			Write<uint32_t>(outStream, 0x00);

			// position?
			createData.position.WriteTo(outStream);
		}
#endif
		*/
		Send(outStream, client);
	}

	void Server::SendObjectUpdate(const ClientPtr& client, const Game::ObjectPtr& object) {
		if (!object) {
			return;
		}

		auto id = object->GetId();

		// Packet data
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectUpdate);

		Write<uint32_t>(outStream, id);

		// 0x15 data in loop

		Send(outStream, client);
	}

	void Server::SendObjectDelete(const ClientPtr& client, const Game::ObjectPtr& object) {
		if (!object) {
			return;
		}

		auto id = object->GetId();

		// Packet data
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectDelete);

		// List of u32? deletes many objects?

		Send(outStream, client);
	}

	void Server::SendActionCommandResponse(const ClientPtr& client) {
		// Packet size: 0x2C
		// 0x56 bytes
		BitStream outStream(8);
		outStream.Write(PacketID::ActionCommandResponse);

		// seems like bitflags but only one is accepted at a time.
		uint8_t value = 8;
		Write<uint32_t>(outStream, value);

		WriteDebugData(outStream, 0x34);

		switch (value) {
			// ability data?
			case 0x01: {
				Write<uint8_t>(outStream, 0x02); // used in switch case 1
				Write<uint8_t>(outStream, 0x01); // 0x01-0x10 (switch in sub_4D9BA0)
				Write<uint8_t>(outStream, 0x00);
				Write<uint8_t>(outStream, 0x0F);
				Write<uint32_t>(outStream, utils::hash_id("none")); // default value

				Write<uint32_t>(outStream, 0xDEADBEEF);

				Write<uint32_t>(outStream, 0x54321);
				Write<uint32_t>(outStream, 0x12345);

				Write<uint32_t>(outStream, 0xABCD);
				Write<uint32_t>(outStream, 0xDCBA);

				WriteDebugData(outStream, 0x18);

				Write<uint32_t>(outStream, 0x123456);
				break;
			}

			case 0x02: {
				break;
			}

			case 0x04: {
				break;
			}

			case 0x08: {
				break;
			}

			case 0x10:
				// reset
				break;
		}

		Write<uint8_t>(outStream, 0x01); // used in switch case 1

		/*
			0 = ability data?

		*/
		Write<uint8_t>(outStream, 0x01); // 0x01-0x10 (switch in sub_4D9BA0)

		WriteDebugData(outStream, 0x32);

		Write<uint32_t>(outStream, 123456);

		Send(outStream, client);
	}

	void Server::SendObjectPlayerMove(const ClientPtr& client) {
		// Packet size: 0x50
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectPlayerMove);

		WriteDebugData(outStream, 0x50);

		Send(outStream, client);
	}

	void Server::SendObjectTeleport(const ClientPtr& client) {
		// Packet size: 0x20
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectTeleport);

		Write<uint32_t>(outStream, 0);

		Send(outStream, client);
	}

	void Server::SendForcePhysicsUpdate(const ClientPtr& client) {
		// Packet size: 0x28
		BitStream outStream(8);
		outStream.Write(PacketID::ForcePhysicsUpdate);

		Send(outStream, client);
	}

	void Server::SendPhysicsChanged(const ClientPtr& client) {
		// Packet size: 0x05
		BitStream outStream(8);
		outStream.Write(PacketID::PhysicsChanged);

		Send(outStream, client);
	}

	void Server::SendLocomotionDataUpdate(const ClientPtr& client) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::LocomotionDataUpdate);

		Send(outStream, client);
	}

	void Server::SendLocomotionDataUnreliableUpdate(const ClientPtr& client) {
		// Packet size: 0x10
		BitStream outStream(8);
		outStream.Write(PacketID::LocomotionDataUnreliableUpdate);

		Send(outStream, client);
	}

	void Server::SendAttributeDataUpdate(const ClientPtr& client) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::AttributeDataUpdate);

		Send(outStream, client);
	}

	void Server::SendCombatantDataUpdate(const ClientPtr& client) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::CombatantDataUpdate);

		Send(outStream, client);
	}

	void Server::SendInteractableDataUpdate(const ClientPtr& client) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::InteractableDataUpdate);

		Send(outStream, client);
	}

	void Server::SendAgentBlackboardUpdate(const ClientPtr& client, const Game::ObjectPtr& object, cAgentBlackboard&& agentBlackboard) {
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::AgentBlackboardUpdate);

		Write<uint32_t>(outStream, object->GetId());
		agentBlackboard.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendLootDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, cLootData&& lootData) {
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::LootDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		lootData.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendServerEvent(const ClientPtr& client, ServerEvent&& serverEvent) {
		// Packet size: variable
		BitStream outStream(8);
		outStream.Write(PacketID::ServerEvent);

		serverEvent.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendCombatEvent(const ClientPtr& client, CombatEvent&& combatEvent) {
		BitStream outStream(8);
		outStream.Write(PacketID::CombatEvent);

		combatEvent.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendModifierCreated(const ClientPtr& client) {
		// Packet size: 0x15
		BitStream outStream(8);
		outStream.Write(PacketID::ModifierCreated);

		Send(outStream, client);
	}

	void Server::SendModifierUpdated(const ClientPtr& client) {
		// Packet size: 0x08
		BitStream outStream(8);
		outStream.Write(PacketID::ModifierUpdated);

		Send(outStream, client);
	}

	void Server::SendModifierDeleted(const ClientPtr& client) {
		// Packet size: 0x19
		BitStream outStream(8);
		outStream.Write(PacketID::ModifierDeleted);

		Send(outStream, client);
	}

	void Server::SendGamePrepareForStart(const ClientPtr& client) {
		if (!client->SetGameState(GameState::PreDungeon)) {
			return;
		}

		/*
			markerSets
				default
				levelshop
				design
				objects
				environment
				plants
				trees
				lights
				VFX
				design_blocks
				ai
				smartobjects
				Audio
				animators
				audio_ambient
				animation_common
				boss_arena_layer

				heat_map_camera
				Affix
				treesA
				treesB
				grass
				animation
				canopy
				Decal
				blobshadows
				Obelisks_1
				Obelisks_2
				Obelisks_3
				smart_objects_1
				smart_objects_2
				smart_objects_3
				AI_Wanderer
				AI_spike_(1-N)(a-c)
				AI_Horde_(1-N)
				PrettyGrass
				AI_Wanderers(A-C)
				designblocks
		*/

		const auto& chainData = mGame.GetChainData();

		// Packet size: 0x10
		BitStream outStream(8);
		outStream.Write(PacketID::GamePrepareForStart);

		bool start = true;
		if (start) {
			Write<uint32_t>(outStream, chainData.GetLevel()); // level file
			Write<uint32_t>(outStream, chainData.GetMarkerSet()); // markerset file, 0 = set to a default
			Write<uint32_t>(outStream, 0b1111); // before this updates its set to 8... unknown use. (arg8 in sub_9C2590)
			// marker set for tutorial: 0xe6335cf5

			// level index? must be (>= 0 && <= 72)
			Write<uint32_t>(outStream, chainData.GetLevelIndex());
		} else {
			Write<uint32_t>(outStream, 0);
			Write<uint32_t>(outStream, 0);
			Write<uint32_t>(outStream, 0);
			Write<uint32_t>(outStream, chainData.GetLevelIndex());
		}

		Send(outStream, client);
	}

	void Server::SendGameStart(const ClientPtr& client) {
		if (client->GetGameState() != GameState::PreDungeon || !client->SetGameState(GameState::Dungeon)) {
			std::cout << "Could not start game, wrong state." << std::endl;
			return;
		}

		const auto& chainData = mGame.GetChainData();

		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::GameStart);

		// level index? must be (>= 0 && <= 72)
		Write<uint32_t>(outStream, chainData.GetLevelIndex());

		Send(outStream, client);
	}

	void Server::SendArenaGameMessages(const ClientPtr& client) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::ArenaGameMsgs);

		GameState gameState = client->GetGameState();
		if (gameState != GameState::PreDungeon) {
			Write<uint8_t>(outStream, static_cast<uint8_t>(gameState));
			switch (gameState) {
				case GameState::PreDungeon:
					// if true: [set state ArenaLobby]
					// else: [set state ArenaRoundResults]
					Write<bool>(outStream, true);
					break;

				case GameState::Observer:
					// same function as SendChainGame with value 2, nothing
					break;

				case GameState::Spectator:
				case GameState::Replay:
					Write<float>(outStream, 1.25f);
					Write<uint32_t>(outStream, 0x00FA); // team score A?
					Write<uint32_t>(outStream, 0x00FB); // team score B?
					break;

				case GameState::Dungeon:
					WriteDebugData(outStream, 0x18);
					break;
			}
		} else {
			// PreDungeon
			
			uint8_t value = 0x09;
			Write<uint8_t>(outStream, value);

			if (value == 0x09) {
				// Read 0x0C bytes

				// used in sub_41BC00, 
				Write<float>(outStream, 1.25f);


				Write<uint32_t>(outStream, 0x00FA); // team score A? +4
				Write<uint32_t>(outStream, 0x00FB); // team score B? +6
			}
		}

		Send(outStream, client);
	}

	void Server::SendChainVoteMessages(const ClientPtr& client, uint8_t value) {
		// Packet size: 0x04
		BitStream outStream(8);
		outStream.Write(PacketID::ChainVoteMsgs);

		/*
			0 = 0x151 bytes of data
			1 = 0x04 bytes of data (show planet info + countdown)
			2 = 0x01 bytes of data (StayInParty + go to CashOut)
		*/

		Write<uint8_t>(outStream, value);
		switch (value) {
			case 0: {
				// No fucking clue what data this is...
				auto& data = mGame.GetChainData();
				data.SetLevelByIndex(4);
				data.SetStarLevel(0);
				data.SetCompleted(false);
				// data.mPlayerAsset = 0x0000000A;

				// data.mEnemyNouns[1] = utils::hash_id("nct_lieu_su_stealther.noun");
				// data.mEnemyNouns[2] = utils::hash_id("nct_lieu_su_stealther_Captain.noun");

				data.WriteTo(outStream);
				break;
			}

			case 1: {
				// Some ResetTimer thing?
				float secondsUntilDeployment = 30.0f;
				Write<float>(outStream, secondsUntilDeployment);
				break;
			}

			case 2: {
				// Go to cash out.
				bool stayInParty = false;
				Write<bool>(outStream, stayInParty);
				break;
			}
		}

		Send(outStream, client);
	}

	void Server::SendObjectivesInitForLevel(const ClientPtr& client, const std::vector<ObjectiveData>& objectives) {
		// Packet size: variable
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectivesInitForLevel);

		uint8_t count = static_cast<uint8_t>(objectives.size());
		Write<uint8_t>(outStream, count);

		for (uint8_t i = 0; i < count; ++i) {
			objectives[i].WriteTo(outStream);
		}

		Send(outStream, client);
	}

	void Server::SendPartyMergeComplete(const ClientPtr& client) {
		// This packet also "sends" debug ping automatically
		BitStream outStream(8);
		outStream.Write(PacketID::PartyMergeComplete);
		outStream.Write<uint64_t>(utils::get_unix_time());

		Send(outStream, client);
	}

	void Server::SendReloadLevel(const ClientPtr& client) {
		// Packet size: 0x08
		BitStream outStream(8);
		outStream.Write(PacketID::ReloadLevel);

		Send(outStream, client);
	}

	void Server::SendQuickGame(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::QuickGameMsgs);

		// if true: [set state Spaceship]
		bool reset = false;
		Write<bool>(outStream, reset);

		Send(outStream, client);
	}

	void Server::SendChainGame(const ClientPtr& client, uint8_t state) {
		BitStream outStream(8);
		outStream.Write(PacketID::ChainGameMsgs);

		// 9500
		/*
			state:
				0 = unk (fade to black) // [set state ChainVote]
				1 = mission failed // [set state GameOver]
				2 = observer? (seems to do nothing)
		*/
		Write<uint8_t>(outStream, state);

		Send(outStream, client);
	}

	void Server::SendJuggernautGame(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::JuggernautGameMsgs);

		uint8_t value = 3;
		Write<uint8_t>(outStream, value);

		if (value == 3) {
			// Values set to cGameUI
			Write<uint8_t>(outStream, 0x00); // ui+2
			Write<uint16_t>(outStream, 0x0001); // ui+4
			Write<uint16_t>(outStream, 0x0002); // ui+6
			Write<uint16_t>(outStream, 0x0003); // ui+8
		} else if (value == 5) {
			// [set state JuggernautResults]
			// client internal
		}

		Send(outStream, client);
	}

	void Server::SendKillRaceGame(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::KillRaceGameMsgs);

		uint8_t value = 7;
		Write<uint8_t>(outStream, value);

		if (value == 7) {
			// Values seem completely unused?
			Write<uint32_t>(outStream, 0x00000001); // [eax]
			Write<uint16_t>(outStream, 0x0002); // [eax+4]
			Write<uint8_t>(outStream, 0x03); // [eax+6]
		} else if (value == 9) {
			// [set state KillRaceResults]
			// client internal
		}

		Send(outStream, client);
	}

	void Server::SendCinematic(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::CinematicMsgs);

		// 0x08, unknown so far
		Write<uint32_t>(outStream, 9500); // dword_11C61B8, this might be player progress
		Write<uint32_t>(outStream, 10000); // dword_11C61BC

		// 0x0C, object position
		Write<float>(outStream, 100.f); // dword_11C61C0
		Write<float>(outStream, 100.f); // dword_11C61C4
		Write<float>(outStream, 100.f); // dword_11C61C8

		// 0x04, multiplies object position?
		Write<float>(outStream, 1.52f);

		Send(outStream, client);
	}

	void Server::SendTutorial(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::TutorialGameMsgs);

		bool unk = true;
		Write<bool>(outStream, unk);

		if (!unk) {
			// send xp
			Write<uint32_t>(outStream, 0);
		}

		Send(outStream, client);
	}

	void Server::SendDebugPing(const ClientPtr& client) {
		// Packet size: 0x08
		BitStream outStream(8);
		outStream.Write(PacketID::DebugPing);

		Write<uint64_t>(outStream, utils::get_unix_time());

		Send(outStream, client);
	}

	void Server::SendTestPacket(const ClientPtr& client, MessageID id, const std::vector<uint8_t>& data) {
		BitStream outStream(8);
		outStream.Write(id);
		for (auto byte : data) {
			outStream.Write(byte);
		}
		Send(outStream, client);
	}
}
