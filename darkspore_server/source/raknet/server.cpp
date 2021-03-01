
// Include
#include "server.h"

#include "sporenet/instance.h"
#include "sporenet/user.h"
#include "sporenet/creature.h"

#include "utils/functions.h"

#include "game/instance.h"
#include "game/objectmanager.h"
#include "game/serverevent.h"

#include <glm/gtx/euler_angles.hpp>

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

// TODO: Use broadcast to send packets instead of loops

/*
	Objective list (will describe how to use later)
		FinishLevelQuickly
		DontUseHealthObelisks
		DoDamageOften
		EnemyHealthRegen
		TouchAllObelisks
		DefeatAllMonsters
		HugeDamage
		LootCrystals
		DestroyAllDestructables
		StayAlive
		PlayerHealthDrain
		NoSlowing
		DefeatSameType
*/

/*
	graphics states
		dead
		fired
		unblocked
		slide_in
		boss_1
		acunit_large_interactable_start
		fired_1
		fired_2
		fired_3

	animation states
		obelisk_activate
		cast_treeoflife
		pickup
		ctd_boss_tc_minion_drop_in
		lf_shroomtempest_basic_a
		lf_shroomtempest_basic_b
*/

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

// TEMPORARY VARIABLES, TESTING ONLY!
static bool teleportMovement = true;

// RakNet
namespace RakNet {
	// Debug
	void WriteDebugData(RakNet::BitStream& stream, size_t length) {
		constexpr std::array<uint8_t, 8> bytes { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
		constexpr size_t bytesMask = bytes.size() - 1;
		for (size_t i = 0; i < length; ++i) {
			Write<uint8_t>(stream, bytes[i & bytesMask]);
		}
	}

	void PrintDebugStream(RakNet::BitStream& stream) {
		auto bytes = bits_to_bytes(stream.GetNumberOfUnreadBits());
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
			// mSelf->SetTimeoutTime(30000, UNASSIGNED_SYSTEM_ADDRESS);
			mSelf->SetTimeoutTime(0xFFFFFFFF, UNASSIGNED_SYSTEM_ADDRESS);
#ifdef PACKET_LOGGING
			mSelf->AttachPlugin(&mLogger);
#endif

			auto socketDescriptor = SocketDescriptor(port, nullptr);
			if (!mSelf->Startup(4, 10, &socketDescriptor, 1)) {
				return;
			}

			mSelf->SetMaximumIncomingConnections(4);
			mSelf->SetOccasionalPing(true);
			// mSelf->SetUnreliableTimeout(1000);
			mSelf->SetUnreliableTimeout(0);

			std::unique_lock<std::mutex> taskUniqueLock(mTaskMutex, std::defer_lock);
			while (is_running()) {
				taskUniqueLock.lock();
				while (!mTasks.empty()) {
					mTasks.front()();
					mTasks.pop();
				}
				taskUniqueLock.unlock();

				if (mGame.ServerUpdate()) {
					run_one();
				}

				if (mGame.Update()) {
					// Loop clients instead of using broadcasting to get separate client data
					const auto& gameStateData = mGame.GetStateData();
					for (const auto& [_, client] : mClients) {
						auto& clientGameStateData = client->GetGameStateData();
						clientGameStateData.var = gameStateData.var;
						clientGameStateData.type = gameStateData.type;

						SendGameState(client, clientGameStateData);
					}
				}

				RakSleep(1);
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
			auto packetSize = packet->bitSize / 8;
			mInStream = BitStream(packet->data, packetSize, false);

			uint8_t packetType = GetPacketIdentifier();
			std::cout << std::endl << "--- packet: "  << (int)packetType << ", length: " << packetSize << " gotten from raknet ---" << std::endl << std::endl;

			if (packetType < ID_USER_PACKET_ENUM) {
				ParseRakNetPackets(packet, packetType);
			} else {
				ParseSporeNetPackets(packet, packetType);
			}
		}

		mInStream.Reset();
	}

	void Server::add_task(std::function<void(void)> task) {
		mTaskMutex.lock();
		mTasks.push(std::move(task));
		mTaskMutex.unlock();
	}

	void Server::add_client_task(uint8_t id, MessageID packet) {
		mTaskMutex.lock();
		mTasks.push([this, id, packet] {
			const auto& client = mClients.begin()->second;
			if (!client) {
				return;
			}

			const auto& player = client->GetPlayer();
			if (!player) {
				return;
			}

			switch (packet) {
				case PacketID::ReloadLevel:
					SendReloadLevel(client);
					break;

				case PacketID::ServerEvent: {
					/*
						Client event ids (quotes = voiceover, parenthesis = text)
							0x6a72be7d (This ability requires a target.)
							0x566c721b "Overdrive not charged"
							0x4995368f - PlayerReady
							0x502a2d7e (when rolling for loot)
							0x53b598f9 - PlayerCashOut
							0x57ccfce5 (Teleporters are deactivated in the presence of Darkspore.)
							0x615f3861 - drop loot for player at textValue index?
							0x62c30464 - PlayerDeath
							0x46c0bc39 (Switching is unavailable.)
							0x414b80b8 - PlayerFreed
							0x459e0873 - PlayerLeftGame
							0x38c1d4be (Invalid target area.)
							0x38571612 (Cannot switch to a hero that has perished.)
							0x28e9b8ec "Warning, final hero"
							0x0bedbf7f "Ally reconnected"
							0x17162dc8 (Insufficient power.)
							0x1d42121d - HordeIncoming (Horde incoming!)
							0x71e2bc36 - PlayerUnlockedSecondCreature (Enemies of the same type as your hero will deal DOUBLE damage with their attacks.)
							0x6ea4091e (when picking up a catalyst?)
							0x6f8812d2 - PlayerTrapped
							0xa717f30b something with lootInstanceId
							0x8d5ab239 - PlayerEnteredTunnel
							0x72b1fc37 - HealthLow
							0x865593d9
							0x8047eaf4 (Horde defeated!)
							0xa5f61911 (That ability is not ready.)
							0xa2bc2e34 (Hero switching is on cooldown.)
							0xe7be61b3 - PlayerExitedTunnel
							0xf7da68a5 - GameOver
							0xd55fa389 "Power reserves low"
							0xad451342 - OverdriveReady
							0xd30b856c "Ally disconnected
							0xbe855f87 (Squad ability is unavailable. That squad member has perished.)
					*/
					break;
				}

				case PacketID::LabsPlayerUpdate: {
					player->SetUpdateBits(
						labsPlayerBits::CharacterMask |
						labsPlayerBits::CrystalMask |
						labsPlayerBits::PlayerBits
					);

					SendLabsPlayerUpdate(client, player);
					player->ResetUpdateBits();
					break;
				}

				case PacketID::ObjectCreate: {
					auto playerObject = player->GetCharacterObject(0);
					if (playerObject) {
						// mGame.DropLoot();
						mGame.DropCatalyst();
					}

					break;
				}

				case PacketID::ObjectUpdate: {
					auto playerObject = player->GetCharacterObject(0);
					if (playerObject) {
						playerObject->SetScale(2);
						// objectData.mLinearVelocity = glm::vec3(5.f, 5.f, 5.f);
						SendObjectUpdate(client, playerObject);
					}

					break;
				}

				case PacketID::DirectorState: {
					cAIDirector director;
					director.mBossId = player->GetCharacterObject(0)->GetId();
					director.mbBossSpawned = true;

					SendDirectorState(client, director);
					break;
				}

				case PacketID::AgentBlackboardUpdate: {
					cAgentBlackboard data;
					data.mbInCombat = false;
					data.mNumAttackers = 0;
					data.mbTargetable = true;
					data.mStealthType = 0;
					data.targetId = 0;

					for (int i = 0; i < 3; ++i) {
						auto object = player->GetCharacterObject(i);
						SendAgentBlackboardUpdate(client, object, data);
					}
					break;
				}

				case PacketID::ModifierCreated: {
					SendModifierCreated(client, player->GetDeployedCharacterObject());
					break;
				}

				case PacketID::DebugPing: {
					SendDebugPing(client);
					break;
				}

				default:
					teleportMovement = !teleportMovement;
					break;
			}
		});
		mTaskMutex.unlock();
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

			case PacketID::LootDropMessage: {
				OnLootDropMessage(client);
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

	ClientPtr Server::GetClient(uint8_t id) const {
		for (const auto& [_, client] : mClients) {
			if (client->GetId() == id) {
				return client;
			}
		}
		return nullptr;
	}

	void Server::Send(BitStream& stream, const ClientPtr& client) {
		mSelf->Send(&stream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, client->mSystemAddress, false);
	}

	void Server::SendBroadcast(BitStream& stream) {
		mSelf->Send(&stream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	void Server::OnNewIncomingConnection(Packet* packet) {
		const auto& client = AddClient(packet);
		if (client) {
			client->SetGameState(GameState::Spaceship);

			// Read but ignore
			SystemAddress systemAddress;
			mInStream.Read(systemAddress);

			std::array<SystemAddress, MAXIMUM_NUMBER_OF_INTERNAL_IDS> mySystemAddress;
			for (decltype(auto) address : mySystemAddress) {
				mInStream.Read(address);
			}

			RakNetTime pongTime;
			mInStream.Read(pongTime);

			RakNetTime time;
			mInStream.Read(time);
			//

			std::cout << "Player connected!" << std::endl;
			std::cout << pongTime << ", " << time << std::endl << std::endl;

			SendConnected(client);
		} else {
			std::cout << "OnNewIncomingConnection: Unknown client tried to connect." << std::endl;
		}
	}

	void Server::OnHelloPlayerRequest(const ClientPtr& client) {
		// Set user
		int64_t blazeId;
		Read(mInStream, blazeId);

		client->SetUser(SporeNet::Get().GetUserManager().GetUserById(blazeId));

		// Get player (null if user is null)
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		std::cout << "Hello " << client->GetUser()->get_name() << "! " << std::endl;

		// Initial player data (move somewhere else later)
		/*
			creature_png = string.format("%d_%d_thumb.png", character.assetID, character.version);
		*/

		// Crystals
		player->SetCrystal(labsCrystal(labsCrystal::AoEDamage, 0, false), 7);
		/*
		player->SetCrystal(labsCrystal(labsCrystal::DamageAura, 1, false), 1);
		player->SetCrystal(labsCrystal(labsCrystal::ImmuneSleep, 2, false), 2);
		player->SetCrystal(labsCrystal(labsCrystal::AttackSpeed, 1, true), 3);
		*/

		// Game state data
		auto& gameStateData = client->GetGameStateData();
		gameStateData.state = static_cast<uint32_t>(GameState::Spaceship);
		gameStateData.type = Blaze::GameType::Chain;

		SendHelloPlayer(client);

		// TODO: wait for party
		SendPartyMergeComplete(client);
		// SendDebugPing(client);

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
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		uint32_t status, oldStatus;
		Read<uint32_t>(mInStream, status);

		float progress, oldProgress;
		Read<float>(mInStream, progress);

		player->GetStatus(oldStatus, oldProgress);
		player->SetStatus(status, progress);

		std::cout << "OnPlayerStatusUpdate: old(" << oldStatus << ", " << oldProgress << "); new(" << status << ", " << progress << ")" << std::endl;

		/*
			order in darkspore.exe
				0, 1, 2, 4
				3, 5, 6, 7, 8
		*/
		/*
		if (status > 0 && progress == 0) {
			player->GetData().SetUpdateBits(RakNet::labsPlayer::Characters);
			player->SetUpdateBits(RakNet::labsPlayerBits::CharacterMask);
			player->SetUpdateBits(RakNet::labsPlayerBits::PlayerBits);
		}
		*/

		switch (status) {
			case 0x02: {
				break;
			}

			case 0x04: {
				break;
			}

			case 0x08: {
				auto& gameStateData = client->GetGameStateData();
				gameStateData.state = static_cast<uint32_t>(GameState::Dungeon);
				gameStateData.type = Blaze::GameType::Chain;

				SendGameStart(client);
				SendDebugPing(client);
				break;
			}

			case 0x20: {
				// OnBeamOut clicked.
				break;
			}
		}

		SendLabsPlayerUpdate(client, player);
		player->ResetUpdateBits();
	}

	void Server::OnActionCommandMsgs(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			// No player? Get lost.
			return;
		}

		uint8_t type;
		Read<uint8_t>(mInStream, type);

		// unknown 11 bytes
		uint8_t skipByte[3];
		for (uint32_t i = 0; i < 3; ++i) {
			Read<uint8_t>(mInStream, skipByte[i]);
		}

		uint32_t timestamp;
		Read<uint32_t>(mInStream, timestamp);

		uint32_t objectId;
		Read<uint32_t>(mInStream, objectId);

		const auto& object = mGame.GetObjectManager().Get(objectId);
		if (!object) {
			// Could not find the object.
			return;
		}

		std::cout << "------------------------" << std::endl;
		std::cout << "OnActionCommandMsgs(" << static_cast<int>(type) << ", " << bits_to_bytes(mInStream.GetNumberOfUnreadBits()) << ")" << std::endl;
		std::cout << "object: " << std::hex << objectId << std::dec << std::endl;
		std::cout << "unknown: " << std::hex;
		std::cout << static_cast<int>(skipByte[0]);
		for (uint32_t i = 1; i < 3; ++i) {
			std::cout << ", " << static_cast<int>(skipByte[i]);
		}
		std::cout << std::dec << std::endl;

		switch (type) {
			// Movement
			case 3: {
				LocomotionData data;
				Read(mInStream, data.mPartialGoalPosition);

				glm::quat orientation;
				Read(mInStream, orientation);
				
				uint32_t unk;
				Read<uint32_t>(mInStream, unk);

				Read(mInStream, data.mGoalPosition);
				Read<uint32_t>(mInStream, data.mGoalFlags);

				uint32_t a0;
				Read<uint32_t>(mInStream, a0);

				/*
					Goal flags
						0x001 = move?
						0x002 = used with 0x100 in "TurnToFaceTargetObject" (stand still?)
						0x004 = used with some jump functions and slide
						0x008 = apply external velocity?
						0x020 = stop?
						0x040 = has target object (ignored if flag 0x001 is set)
						0x100 = face object when moving?
						0x400 = something
						0x800 = start rotating towards facing direction?
				*/

				// data.mGoalFlags = 0x001 | 0x800;
				if (teleportMovement) {
					data.mGoalFlags = 0x020;
				}

				data.mFacing = glm::eulerAngles(orientation);
				data.mAllowedStopDistance = 0;
				data.mDesiredStopDistance = 0;
				data.mTargetObjectId = 0;
				data.reflectedLastUpdate = 0xFFFF;

				mGame.MoveObject(object, data);
				mGame.SendObjectUpdate(object);

				// SendObjectUpdate(client, object);
				break;
			}

			// Stop moving
			case 4: {
				/*
				0x0, 0xbe, 0x11, 0x0, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0xc8, 0x42, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0xe8, 0x62, 0x21, 0x12, 0x60, 0x62, 0x21, 0x12, 0x53, 0x2b, 0x9c, 0x0, 0x70, 0xfc, 0x19, 0x0, 0x80, 0xfc, 0x19, 0x0


OnActionCommandMsgs
Cinematic?
OnActionCommandMsgs(4)
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0, 0x0, 0x0, 0x9a, 0x19, 0x72, 0x42, 0xcb, 0x4c, 0x4, 0xc2, 0x3e, 0x8d, 0xa0, 0x41, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xbe, 0xbe, 0x42, 0xbf, 0xe4, 0x29, 0x26, 0x3f, 0x1, 0x0, 0x0, 0x0, 0x28, 0xd9, 0x87, 0x13, 0xa0, 0xd8, 0x87, 0x13, 0x53, 0x2b, 0x9c, 0x0, 0x6c, 0xfc, 0x19, 0x0, 0x7c, 0xfc, 0x19, 0x0
				*/

				glm::vec3 position;
				Read(mInStream, position);

				glm::quat orientation;
				Read(mInStream, orientation);

				// Is this garbage data?
				uint32_t goalFlags;
				Read<uint32_t>(mInStream, goalFlags);

				uint32_t u0, u1, u2;
				Read(mInStream, u0);
				Read(mInStream, u1);
				Read(mInStream, u2);

				uint32_t a0, a1;
				Read<uint32_t>(mInStream, a0);
				Read<uint32_t>(mInStream, a1);

				std::cout << "Stopping movement?" << std::endl;
				std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
				std::cout << std::hex;
				std::cout << goalFlags << std::endl;
				std::cout << u0 << ", " << u1 << ", " << u2 << std::endl;
				std::cout << a0 << ", " << a1 << std::endl;
				std::cout << std::dec;

				if (!teleportMovement) {
					LocomotionData data;
					data.Stop();

					SendObjectPlayerMove(client, object, data);
				}

				break;
			}

			// Switch hero
			case 5: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				uint32_t creatureIndex;
				Read<uint32_t>(mInStream, creatureIndex);

				mGame.SwapCharacter(player, creatureIndex);
				break;
			}

			// Ability
			case 7: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				CombatData combatData;
				Read<uint32_t>(mInStream, combatData.targetId);
				Read(mInStream, combatData.cursorPosition);
				Read(mInStream, combatData.targetPosition);

				uint32_t abilityIndex;
				Read<uint32_t>(mInStream, abilityIndex);
				combatData.abilityId = player->GetAbilityId(player->GetCurrentDeckIndex(), abilityIndex);

				Read<uint32_t>(mInStream, combatData.unk[0]);
				Read<uint32_t>(mInStream, combatData.unk[1]);
				Read<uint32_t>(mInStream, combatData.valueFromActionResponse);

				AbilityCommandResponse actionResponse;
				actionResponse.abilityId = combatData.abilityId;
				actionResponse.cooldown = 5000;
				actionResponse.userData = 0x1234;

				object->SetOrientation(orientation);
				mGame.UseAbility(object, combatData);

				break;
			}

			// Squad ability
			case 8: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				CombatData combatData;
				Read<uint32_t>(mInStream, combatData.targetId);
				Read(mInStream, combatData.cursorPosition);
				Read(mInStream, combatData.targetPosition);

				uint32_t abilityIndex;
				Read<uint32_t>(mInStream, abilityIndex);
				combatData.abilityId = player->GetAbilityId(0xFF, abilityIndex);

				Read<uint32_t>(mInStream, combatData.unk[0]);
				Read<uint32_t>(mInStream, combatData.unk[1]);
				Read<uint32_t>(mInStream, combatData.valueFromActionResponse);

				AbilityCommandResponse actionResponse;
				actionResponse.abilityId = combatData.abilityId;
				actionResponse.cooldown = 2000;
				actionResponse.userData = 0x4321;

				mGame.UseAbility(object, combatData);
				SendCooldownUpdate(client, object, actionResponse.abilityId, actionResponse.cooldown);

				break;
			}

			// Catalyst pickup
			case 9: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				uint32_t catalystObjectId;
				Read<uint32_t>(mInStream, catalystObjectId);

				glm::vec3 catalystPosition;
				Read(mInStream, catalystPosition);

				uint32_t unknown;
				Read<uint32_t>(mInStream, unknown);

				mGame.InteractWithObject(player, catalystObjectId);
				break;
			}

			// Cancel current action
			case 10: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				uint32_t u0, u1, u2, u3;
				Read<uint32_t>(mInStream, u0);
				Read<uint32_t>(mInStream, u1);
				Read<uint32_t>(mInStream, u2);
				Read<uint32_t>(mInStream, u3);

				uint32_t a0, a1;
				Read<uint32_t>(mInStream, a0);
				Read<uint32_t>(mInStream, a1);

				std::cout << "Cancel current action?" << std::endl;
				std::cout << currentPosition.x << ", " << currentPosition.y << ", " << currentPosition.z << std::endl;
				std::cout << orientation.x << ", " << orientation.y << ", " << orientation.z << ", " << orientation.w << std::endl;
				std::cout << u0 << ", " << u1 << ", " << u2 << ", " << u3 << std::endl;
				std::cout << a0 << ", " << a1 << std::endl;

				/*
				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xdc, 0x2f, 0x1a, 0x77, 0x4c, 0xae, 0xf3, 0x76, 0x98, 0xfe, 0x19, 0x0

				Same pos?
				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x68, 0xfc, 0x19, 0x0, 0x78, 0xfc, 0x19, 0x0, 0x98, 0xfe, 0x19, 0x0

				0x0, 0xe8, 0x11, 0x0, 0xb0, 0xfe, 0x7f, 0x0, 0x0, 0x0, 0x0, 0xb7, 0x67, 0x23, 0x77, 0x74, 0xfc, 0x19, 0x0, 0xe5, 0x61, 0xf2, 0x76, 0x8f, 0x61, 0xf2, 0x76, 0x5d, 0x38, 0x21, 0x8e, 0x0, 0x0, 0x0, 0x0, 0xea, 0x3, 0x0, 0x0, 0xa0, 0xb5, 0x1a, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x68, 0xfc, 0x19, 0x0, 0x78, 0xfc, 0x19, 0x0, 0x98, 0xfe, 0x19, 0x0
				*/
				break;
			}

			// Interactable
			case 11: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				uint32_t interactableObjectId;
				Read<uint32_t>(mInStream, interactableObjectId);

				mGame.InteractWithObject(player, interactableObjectId);
				break;
			}

			// Dancing (/dance command)
			case 12: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				// TODO: proper function for dancing
				mGame.SendAnimationState(object, utils::hash_id("emote_dance_all"), false);
				break;
			}

			// Taunting (/taunt command)
			case 13: {
				glm::vec3 currentPosition;
				Read(mInStream, currentPosition);

				glm::quat orientation;
				Read(mInStream, orientation);

				// TODO: proper function for taunting
				mGame.SendAnimationState(object, utils::hash_id("emote_taunt_all"), false);
				break;
			}

			default:
				break;
		}

		PrintDebugStream(mInStream);
		std::cout << "------------------------" << std::endl;
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

		uint32_t moveType;
		Read<uint32_t>(mInStream, moveType);

		uint32_t pickupTime;
		Read<uint32_t>(mInStream, pickupTime);

		uint32_t dropTime;
		Read<uint32_t>(mInStream, dropTime);

		uint32_t unk;
		Read<uint32_t>(mInStream, unk);

		uint32_t newSlot;
		Read<uint32_t>(mInStream, newSlot);

		std::cout << "-- CrystalDragMessage --" << std::endl;
		if (moveType == 0) {
			std::cout << "Dropped crystal " << crystalSlot << " into the world." << std::endl;
		} else if (moveType == 2) {
			if (crystalSlot == 0) {
				std::cout << "Picked up crystal into slot " << newSlot << "." << std::endl;
			} else {
				std::cout << "Moved crystal from slot " << crystalSlot << " to slot " << newSlot << "." << std::endl;
			}
		}

		std::cout << std::hex << pickupTime << ", " << dropTime << ", " << unk << std::dec << std::endl;
		std::cout << std::endl << "------------------------" << std::endl;

		// 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x20, 0x41, 0xbd, 0x17, 0x90, 0x42, 0xbd, 0x17, 0xc4, 0xa, 0xa2, 0x17, 0xc8, 0x1a, 0xc8, 0x11
		// 0xc0, 0x26, 0x89, 0x18, 0x30, 0x28, 0x89, 0x18, 0xc4, 0xa, 0xa2, 0x17

		const auto& crystal = player->GetCrystal(crystalSlot);

		CrystalData crystalData;
		/*
		crystalData.position.x = 1.f;
		crystalData.position.y = 2.f;
		crystalData.position.z = 3.f;
		*/
		// crystalData.position = position;

		crystalData.unk32[0] = 0x12345678;
		crystalData.unk32[1] = 0x87654321;
		crystalData.unk32[2] = 0xAABBCCDD;
		crystalData.unk32[3] = 0xDDCCBBAA;

		crystalData.unk8 = 5;

		SendCrystalMessage(client, crystalData);
	}

	void Server::OnLootDropMessage(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}
		/*
		uint32_t crystalSlot;
		Read<uint32_t>(mInStream, crystalSlot);

		uint32_t moveType;
		Read<uint32_t>(mInStream, moveType);

		uint32_t pickupTime;
		Read<uint32_t>(mInStream, pickupTime);

		uint32_t dropTime;
		Read<uint32_t>(mInStream, dropTime);

		uint32_t unk;
		Read<uint32_t>(mInStream, unk);

		uint32_t newSlot;
		Read<uint32_t>(mInStream, newSlot);
		*/

		std::cout << "-- LootDropMessage --" << std::endl;
		PrintDebugStream(mInStream);
		std::cout << std::endl << "------------------------" << std::endl;

		// 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x20, 0x41, 0xbd, 0x17, 0x90, 0x42, 0xbd, 0x17, 0xc4, 0xa, 0xa2, 0x17, 0xc8, 0x1a, 0xc8, 0x11
		// 0xc0, 0x26, 0x89, 0x18, 0x30, 0x28, 0x89, 0x18, 0xc4, 0xa, 0xa2, 0x17

		// SendCrystalMessage(client, crystalData);
		/*
		constexpr MessageID LootDataUpdate = 0x9A;
		constexpr MessageID ServerEvent = 0x9B;
		constexpr MessageID ActionCommandMsgs = 0x9C;
		constexpr MessageID PlayerDamage = 0x9E;
		constexpr MessageID LootSpawned = 0x9F;
		constexpr MessageID LootAcquired = 0xA0;
		*/
	}
	
	void Server::OnDebugPing(const ClientPtr& client) {
		const auto& player = client->GetPlayer();
		if (!player) {
			return;
		}

		uint64_t time;
		Read<uint64_t>(mInStream, time);

		/*
		std::string timeString(0x20, '\0');
		if (const auto* timePtr = _gmtime64(reinterpret_cast<const __time64_t*>(&time))) {
			strftime(&timeString[0], 0x20, "%H:%M:%S", timePtr);
		} else {
			timeString = std::to_string(time);
		}

		std::cout << "-- DebugPing --" << std::endl;
		std::cout << timeString << ", 0x" << std::hex << time << std::dec << std::endl;
		std::cout << "---------------" << std::endl;
		*/

		// Schedule other packets?
		switch (client->GetGameState()) {
			case GameState::Spaceship: {
				auto& gameStateData = client->GetGameStateData();
				gameStateData.state = static_cast<uint32_t>(GameState::ChainVoting);

				SendReconnectPlayer(client, GameState::Spaceship);
				// SendChainGame(client, 0);
				break;
			}

			case GameState::ChainVoting: {
				//
				break;
			}

			case GameState::PreDungeon: {
				// SendLabsPlayerUpdate(client, player);
				// SendArenaGameMessages(client);
				// SendReconnectPlayer(client, GameState::Dungeon);
				// SendDebugPing(client);
				break;
			}

			case GameState::Dungeon: {
				mGame.OnPlayerStart(player);

				// SendChainGame(client, 2);

				// Other stuff
				cAIDirector director;
				director.mBossId = 0;
				director.mbBossSpawned = false;

				SendObjectivesInitForLevel(client);
				SendDirectorState(client, director);

				SendQuickGame(client);
				// SendTutorial(client);
				// SendChainGame(client, 2);

				mGame.SwapCharacter(player, 1);
				break;
			}
		}
	}

	void Server::PrepareGameStart(const ClientPtr& client, uint8_t unknown, uint32_t squadId) {
		const auto& player = client->GetPlayer();
		if (!player) {
			// Should never happen.
			return;
		}

		const auto& user = client->GetUser();
		if (!user) {
			// Nope, go away.
			return;
		}

		const auto& squad = user->GetSquadById(squadId);
		if (!squad) {
			// Should not happen.
			return;
		}

		auto& gameStateData = client->GetGameStateData();
		gameStateData.state = static_cast<uint32_t>(GameState::PreDungeon);

		player->SetSquad(squad);

		SendGamePrepareForStart(client);
	}

	void Server::SendHelloPlayer(const ClientPtr& client) {
		// Packet size: 0x08
		const auto& guid = mSelf->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);

		auto addr = mSelf->GetSystemAddressFromGuid(guid);
		/*
		if (1) { // add check for local
			addr = SystemAddress("127.0.0.1", addr.port);
		}
		*/

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

		BitStream outStream(8);
		outStream.Write(PacketID::GameState);

		// Game time
		Write<uint64_t>(outStream, mGame.GetTime());
		// sub_9D79C0 sets these 2 values to +38h and +3Ch

		// Game objective completion time
		Write<uint64_t>(outStream, mGame.GetTimeElapsed());
		Write<uint8_t>(outStream, static_cast<uint8_t>(data.state)); // mov [simulator+18h], value2 (default: 0) is this a boolean?
		
		// Game type
		Write<uint32_t>(outStream, static_cast<uint32_t>(data.type));

		// sub_9C16D0 - unused?
		Write<uint32_t>(outStream, 1);			// mov [simulator+3B41Ch], value (default: 0)

		Send(outStream, client);
	}

	void Server::SendGameState() {
		// data in here seems to only be used in spectator, dungeon, cinematic, tutorial and some other state

		// arg0 and arg1 is moved to other values in sub_9D79C0

		// Packet size: 0x19
		BitStream outStream(8);
		outStream.Write(PacketID::GameState);

		// sub_9D7830
		// this is probably double
		Write<double>(outStream, 0.0);
		// Write<uint32_t>(outStream, data.var0);		// mov [eax+30h], value0, an address? used in sub_9D7A70
		// Write<uint32_t>(outStream, data.var1);	// mov [eax+34h], value1, an address?, cmp'd against
		// sub_9D79C0 sets these 2 values to +38h and +3Ch

		// sub_9C1B30(GET), sub_9C1B50(SET) - game objective completion time
		Write<uint64_t>(outStream, mGame.GetTimeElapsed());

		Write<uint8_t>(outStream, static_cast<uint8_t>(GameState::Dungeon)); // mov [simulator+18h], value2 (default: 0)

		// sub_9C16F0
		Write<uint32_t>(outStream, static_cast<uint32_t>(Blaze::GameType::Chain));		// mov [simulator+3B420h], value (default: 0xFFFFFFFF)

		// sub_9C16D0 - unused?
		Write<uint32_t>(outStream, 1);			// mov [simulator+3B41Ch], value (default: 0)

		SendBroadcast(outStream);
	}

	void Server::SendLabsPlayerUpdate(const ClientPtr& client, const Game::PlayerPtr& player) {
		if (!player) {
			return;
		}

		uint16_t updateBits = player->GetUpdateBits();
		if (updateBits == 0) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::LabsPlayerUpdate);

		Write<uint8_t>(outStream, player->GetId());
		Write<uint16_t>(outStream, updateBits);

		// write player
		if (updateBits & labsPlayerBits::PlayerBits) {
			player->WriteReflection(outStream);
		}

		// write characters
		if (updateBits & labsPlayerBits::CharacterMask) {
			for (uint32_t i = 0; i < 3; ++i) {
				if (updateBits & (labsPlayerBits::CharacterBits << i)) {
					player->GetCharacter(i).WriteReflection(outStream);
				}
			}
		}

		// write crystals
		if (updateBits & labsPlayerBits::CrystalMask) {
			for (uint32_t i = 0; i < 9; ++i) {
				if (updateBits & (labsPlayerBits::CrystalBits << i)) {
					player->GetCrystal(i).WriteReflection(outStream);
				}
			}
		}

		Send(outStream, client);
	}

	void Server::SendDirectorState(const ClientPtr& client, const cAIDirector& director) {
		BitStream outStream(8);
		outStream.Write(PacketID::DirectorState);

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

		const auto& euler = glm::eulerAngles(object->GetOrientation());

		// Packet data
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectCreate);

		// set object id to 1?
		Write<uint32_t>(outStream, object->GetId());

		// object creation data
		cGameObjectCreateData createData;
		createData.noun = object->GetNounId();
		createData.position = object->GetPosition();
		createData.rotXDegrees = euler.x;
		createData.rotYDegrees = euler.y;
		createData.rotZDegrees = euler.z;
		createData.assetId = object->GetAssetId();
		createData.scale = object->GetScale();
		createData.team = object->GetTeam();
		createData.hasCollision = object->HasCollision();
		createData.playerControlled = object->IsPlayerControlled();

		// write reflection of creation and the object
		createData.WriteReflection(outStream);
		object->WriteReflection(outStream);

		// write update data (same as in ObjectUpdate
		// is the u8 value here "team"?
#if 1
		/*
		Write<uint8_t>(outStream, 0x00);
		Write<uint32_t>(outStream, createData.noun);

		// at u8 (0x19) this makes it crash because it assumes a memory address in the client
		Write<uint32_t>(outStream, 0x00);

		// position?
		Write(outStream, createData.position);
		*/
#else
		for (uint8_t id = 0; id <= 0; ++id) {
			Write<uint8_t>(outStream, id);
			Write<uint32_t>(outStream, createData.noun);
			Write<uint32_t>(outStream, 0x00);

			// position?
			createData.position.WriteTo(outStream);
		}
#endif
		Send(outStream, client);
	}

	void Server::SendObjectUpdate(const ClientPtr& client, const Game::ObjectPtr& object) {
		if (!object) {
			return;
		}

		// Packet data
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectUpdate);

		Write<uint32_t>(outStream, object->GetId());
		object->WriteReflection(outStream);

		// 0x15 data in loop

		Send(outStream, client);
	}

	void Server::SendObjectDelete(const ClientPtr& client, const Game::ObjectPtr& object) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::ObjectDelete);

		Write<uint32_t>(outStream, object->GetId());

		Send(outStream, client);
	}

	void Server::SendObjectDelete(const ClientPtr& client, const std::vector<Game::ObjectPtr>& objects) {
		// 100%
		if (objects.empty()) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::ObjectDelete);
		for (auto object : objects) {
			if (object) {
				Write<uint32_t>(outStream, object->GetId());
			}
		}

		Send(outStream, client);
	}

	void Server::SendActionCommandResponse(const ClientPtr& client, uint8_t type) {
		BitStream outStream(8);
		outStream.Write(PacketID::ActionCommandResponse);

		// seems like bitflags but only one is accepted at a time.
		Write<uint8_t>(outStream, 0xFF);
		Write<uint8_t>(outStream, type);
		Write<uint8_t>(outStream, 0xAA);
		Write<uint8_t>(outStream, 0xBB);

		// WriteDebugData(outStream, 0x34);

		switch (type) {
			// ability data?
			case 0x01: {
				/*
				byte 0 = arg_0 -> sub_4E21D0 | esi+1Ch | 
				byte 1 = unk
				byte 2 = unk
				byte 3 = unk
				*/

				/*
					Sage abilities
						SupportHealerBasic
						TreeOfLife
						CastEnrage
						SupportHealerSupport
						SupportHealerPassiveModifier
				*/

				Write<uint32_t>(outStream, utils::hash_id("TreeOfLife")); // arg_0 -> sub_9DFB40 (must be a valid object)
				Write<uint32_t>(outStream, 1); // compared to 0
				Write<uint32_t>(outStream, 0);

				auto time = mGame.GetTimeElapsed();
				Write<uint64_t>(outStream, time + 1000); // arg_0 -> sub_9D7A00 (packed as 64 bit)
				Write<uint64_t>(outStream, time + 2000); // arg_0 -> sub_9D7A00_2 (packed as 64 bit)
				Write<uint64_t>(outStream, time + 3000); // arg_0 -> sub_9D7A00_3 (packed as 64 bit)
				Write<uint64_t>(outStream, time + 4000); // arg_0 -> sub_9D7A00_4 (packed as 64 bit)

				Write<uint32_t>(outStream, 0);
				Write<uint32_t>(outStream, 0x123456); // arg_4 -> sub_4E21D0 (sets dword_143FE3C)
				break;
			}

			case 0x02: {
				/*
					if byte_143FD6E == stream[1] {
						byte_143FD6C = 0;
					}

					if byte_143FDE2 == stream[1] {
						byte_143FDE0 = 0;
					}
				*/
				WriteDebugData(outStream, 0x30);
				Write<uint32_t>(outStream, 0x11112222); // dword_143FE3C
				break;
			}

			case 0x04: {
				break;
			}

			case 0x08: {
				// move to position with or without stopping distance
				WriteDebugData(outStream, 0x34);
				break;
			}

			case 0x10:
				// reset value sent to 0x02
				break;
		}

		Send(outStream, client);
	}

	void Server::SendActionCommandResponse(const ClientPtr& client, const AbilityCommandResponse& data) {
		BitStream outStream(8);
		outStream.Write(PacketID::ActionCommandResponse);

		Write<uint8_t>(outStream, 0x00); // arg_0 -> sub_4E21D0 | esi+1Ch | compared with byte_143FD6E
		Write<uint8_t>(outStream, 0x01);
		Write<uint8_t>(outStream, 0x00); // unused
		Write<uint8_t>(outStream, 0x00); // unused

		Write<uint32_t>(outStream, data.abilityId);
		Write<uint32_t>(outStream, 1);
		Write<uint32_t>(outStream, 0); // unused

		auto time = mGame.GetTime();
		Write<uint64_t>(outStream, 0); // ([esi+20h], [esi+24h]) sub_4E1FE0
		Write<uint64_t>(outStream, time + data.timeImmobilized); // ([esi+28h], [esi+2Ch])
		Write<uint64_t>(outStream, time + data.cooldown); // arg_0 -> sub_9D7A00_3 (packed as 64 bit)
		Write<uint64_t>(outStream, time); // arg_0 -> sub_9D7A00_4 (packed as 64 bit)

		Write<uint32_t>(outStream, 0); // unused
		Write<uint32_t>(outStream, data.userData);

		Send(outStream, client);
	}

	void Server::SendObjectJump(const ClientPtr& client, const Game::ObjectPtr& object, const glm::vec3& position, const glm::vec3& destination, const glm::quat& rotation) {
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectJump);

		Write<uint32_t>(outStream, object->GetId());
		Write(outStream, position);
		Write(outStream, destination);
		Write(outStream, rotation);

		Send(outStream, client);
	}

	void Server::SendObjectTeleport(const ClientPtr& client, const Game::ObjectPtr& object, const glm::vec3& position, const glm::vec3& direction) {
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectTeleport);

		Write<uint32_t>(outStream, object->GetId());
		Write(outStream, position);
		Write(outStream, direction);

		Send(outStream, client);
	}

	void Server::SendObjectPlayerMove(const ClientPtr& client, const Game::ObjectPtr& object, const LocomotionData& locomotionData) {
		// 100%
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectPlayerMove);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, locomotionData.mGoalFlags);
		Write(outStream, locomotionData.mGoalPosition);
		Write(outStream, locomotionData.mFacing);
		Write(outStream, locomotionData.mExternalLinearVelocity);
		Write(outStream, locomotionData.mExternalForce);
		Write<float>(outStream, locomotionData.mAllowedStopDistance);
		Write<float>(outStream, locomotionData.mDesiredStopDistance);
		Write(outStream, locomotionData.mTargetPosition);
		Write<uint32_t>(outStream, locomotionData.mTargetObjectId);

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

	void Server::SendLocomotionDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const LocomotionData& locomotionData) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::LocomotionDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		locomotionData.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendLocomotionDataUnreliableUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const glm::vec3& position) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(0x10);
		outStream.Write(PacketID::LocomotionDataUnreliableUpdate);

		Write<uint32_t>(outStream, object->GetId());
		Write(outStream, position);

		Send(outStream, client);
	}

	void Server::SendAttributeDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const Game::Attributes& attributes) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::AttributeDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		attributes.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendCombatantDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const cCombatantData& combatantData) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::CombatantDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		combatantData.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendInteractableDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const cInteractableData& interactableData) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::InteractableDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		interactableData.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendAgentBlackboardUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const cAgentBlackboard& agentBlackboard) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::AgentBlackboardUpdate);

		Write<uint32_t>(outStream, object->GetId());
		agentBlackboard.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendLootDataUpdate(const ClientPtr& client, const Game::ObjectPtr& object, const cLootData& lootData) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::LootDataUpdate);

		Write<uint32_t>(outStream, object->GetId());
		lootData.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendCooldownUpdate(const ClientPtr& client, const Game::ObjectPtr& object, uint32_t id, int64_t milliseconds) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::CooldownUpdate);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, id);
		Write<uint32_t>(outStream, 0); // unused
		Write<uint64_t>(outStream, milliseconds);
		Write<uint64_t>(outStream, mGame.GetTime());
		Write<uint64_t>(outStream, 0); // not quite sure how this one works yet
		
		Send(outStream, client);
	}

	void Server::SendServerEvent(const ClientPtr& client, const Game::ServerEventBase& serverEvent) {
		// 100%
		BitStream outStream(8);
		outStream.Write(PacketID::ServerEvent);

		serverEvent.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendCombatEvent(const ClientPtr& client, const CombatEvent& combatEvent) {
		// 100%
		BitStream outStream(8);
		outStream.Write(PacketID::CombatEvent);

		combatEvent.WriteReflection(outStream);

		Send(outStream, client);
	}

	void Server::SendModifierCreated(const ClientPtr& client, const Game::ObjectPtr& object) {
		// Packet size: 0x15
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::ModifierCreated);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, utils::hash_id("SecurityTeleporterPassive"));
		Write<uint32_t>(outStream, utils::hash_id("SecurityTeleporterPassive"));
		Write<uint32_t>(outStream, 0x89ABCDEF);
		Write<uint32_t>(outStream, utils::hash_id("BossSecurityTeleporterPassive"));
		Write<uint32_t>(outStream, 0x87651234);
		Write<uint64_t>(outStream, mGame.GetTime()); // some timestamp
		Write<uint32_t>(outStream, 0x1234); // some noun? because TestLevelData
		Write<uint8_t>(outStream, 0xFF);

		Send(outStream, client);
	}

	void Server::SendModifierUpdated(const ClientPtr& client, const Game::ObjectPtr& object) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::ModifierUpdated);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, 0); // modifier id
		Write<uint64_t>(outStream, mGame.GetTime()); // some timestamp
		Write<uint32_t>(outStream, 0x1234); // esi+24h
		Write<uint8_t>(outStream, 0xFF); // esi+2Ch

		Send(outStream, client);
	}

	void Server::SendModifierDeleted(const ClientPtr& client, const Game::ObjectPtr& object, uint32_t modifierId) {
		// 100%
		if (!object) {
			return;
		}

		BitStream outStream(8);
		outStream.Write(PacketID::ModifierDeleted);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, modifierId); // modifier id

		Send(outStream, client);
	}

	void Server::SendAnimationState(const ClientPtr& client, const Game::ObjectPtr& object, uint32_t state, uint64_t timestamp, bool overlay) {
		BitStream outStream(8);
		outStream.Write(PacketID::SetAnimationState);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, state);
		Write<uint64_t>(outStream, timestamp);
		Write<bool>(outStream, overlay);
		Write<float>(outStream, 1.f);
		Write<uint32_t>(outStream, state); // some client data?

		Send(outStream, client);
	}

	void Server::SendObjectGfxState(const ClientPtr& client, const Game::ObjectPtr& object, uint32_t state, uint64_t timestamp) {
		BitStream outStream(8);
		outStream.Write(PacketID::SetObjectGfxState);

		Write<uint32_t>(outStream, object->GetId());
		Write<uint32_t>(outStream, state);
		Write<uint64_t>(outStream, timestamp);

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
			// sub_9DB270(_, _, markerset, last_value);

			Write<uint32_t>(outStream, chainData.GetLevel()); // level file
			Write<uint32_t>(outStream, chainData.GetMarkerSet()); // markerset file? (sub_AEAAE0?) // chainData.GetMarkerSet()
			Write<uint32_t>(outStream, 1); // before this updates its set to 8... unknown use. (argC in sub_9C2590, arg8 in sub_9DB650) 0b1011
			// 0bABCD | 1 bit per player?

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
				// data.mPlayerAsset = 0x0000000A;

				// data.mEnemyNouns[1] = utils::hash_id("nct_lieu_su_stealther.noun");
				// data.mEnemyNouns[2] = utils::hash_id("nct_lieu_su_stealther_Captain.noun");

				data.WriteTo(outStream);
				break;
			}

			case 1: {
				float secondsUntilDeployment = 30.0f;
				// add some timer to force start game

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

	void Server::SendObjectivesInitForLevel(const ClientPtr& client) {
		// 100%
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectivesInitForLevel);

		const auto& objectives = mGame.GetObjectives();
		uint8_t count = static_cast<uint8_t>(objectives.size());

		Write<uint8_t>(outStream, count);
		for (uint8_t i = 0; i < count; ++i) {
			objectives[i].WriteTo(outStream);
		}

		Send(outStream, client);
	}

	void Server::SendObjectiveUpdate(const ClientPtr& client, uint8_t id) {
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectiveUpdated);

		const auto& objectives = mGame.GetObjectives();
		const auto& objective = objectives[id];

		Write<uint32_t>(outStream, objective.id);
		Write<uint8_t>(outStream, client->GetId()); // player/client id (they are the same)
		Write<uint8_t>(outStream, static_cast<uint8_t>(objective.medal));
		Write<uint32_t>(outStream, 0); // unknown
		Write<bool>(outStream, false); // show notification
		Write<uint32_t>(outStream, objective.value);
		Write<uint32_t>(outStream, 2); // unknown
		Write<uint32_t>(outStream, 3); // unknown

		Send(outStream, client);
	}

	void Server::SendObjectivesComplete(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectivesComplete);

		const auto& objectives = mGame.GetObjectives();

		uint8_t count = static_cast<uint8_t>(objectives.size());
		uint32_t medals = 0;

		Write<uint8_t>(outStream, count);
		for (uint8_t i = 0; i < count; ++i) {
			const auto& objective = objectives[i];
			objective.WriteTo(outStream);

			medals |= (static_cast<uint32_t>(objective.medal) << (8 * i));
		}

		Write<uint32_t>(outStream, medals);

		Send(outStream, client);
	}

	void Server::SendObjectiveAdd(const ClientPtr& client, const Objective& objective) {
		BitStream outStream(8);
		outStream.Write(PacketID::ObjectiveAdd);
		
		objective.WriteTo(outStream);

		Send(outStream, client);
	}

	void Server::SendPartyMergeComplete(const ClientPtr& client) {
		// This packet also "sends" debug ping automatically
		BitStream outStream(8);
		outStream.Write(PacketID::PartyMergeComplete);

		Write<uint64_t>(outStream, utils::get_unix_time());

		Send(outStream, client);
	}

	void Server::SendReloadLevel(const ClientPtr& client) {
		// 100%
		BitStream outStream(8);
		outStream.Write(PacketID::ReloadLevel);

		// add some sort of callback to reload all objects etc

		Send(outStream, client);
	}

	void Server::SendCrystalMessage(const ClientPtr& client, const CrystalData& crystalData) {
		BitStream outStream(8);
		outStream.Write(PacketID::CrystalMessage);

		Write<uint8_t>(outStream, crystalData.unk8);
		Write<uint32_t>(outStream, crystalData.unk32[0]);
		Write<uint32_t>(outStream, crystalData.unk32[1]);
		Write<uint32_t>(outStream, crystalData.unk32[2]);
		Write<uint32_t>(outStream, crystalData.unk32[3]);
		Write<uint32_t>(outStream, crystalData.unk32[4]);
		Write<uint32_t>(outStream, crystalData.unk32[5]);
		Write<uint32_t>(outStream, crystalData.unk32[6]);


		Send(outStream, client);
	}

	void Server::SendQuickGame(const ClientPtr& client) {
		BitStream outStream(8);
		outStream.Write(PacketID::QuickGameMsgs);

		// honestly im not 100% sure yet, ignore all
		// if true: [set state Spaceship]
		bool reset = true;
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
		// 100%
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
