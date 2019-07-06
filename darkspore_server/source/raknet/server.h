
#ifndef _RAKNET_SERVER_HEADER
#define _RAKNET_SERVER_HEADER

#define PACKET_LOGGING

// Include
#include "../blaze/types.h"

#include <RakPeerInterface.h>
#include <BitStream.h>
#ifdef PACKET_LOGGING
#	include <PacketLogger.h>
#endif

#include <cstdint>
#include <thread>
#include <mutex>
#include <vector>

// RakNet
namespace RakNet {
	// Packet
	namespace PacketID {
		constexpr MessageID HelloPlayer = 0x80;
		constexpr MessageID ReconnectPlayer = 0x81;
		constexpr MessageID Connected = 0x82;
		constexpr MessageID Goodbye = 0x83;
		constexpr MessageID PlayerJoined = 0x84;
		constexpr MessageID PartyMergeComplete = 0x85;
		constexpr MessageID PlayerDeparted = 0x86;
		constexpr MessageID VoteKickStarted = 0x87;
		constexpr MessageID PlayerStatusUpdate = 0x88;
		constexpr MessageID GameAborted = 0x89;
		constexpr MessageID GameState = 0x8A;
		constexpr MessageID DirectorState = 0x8B;
		constexpr MessageID ObjectUpdate = 0x8C;
		constexpr MessageID ObjectDelete = 0x8D;
		constexpr MessageID ObjectJump = 0x8E;
		constexpr MessageID ActionCommandResponse = 0x8F;
		constexpr MessageID ObjectPlayerMove = 0x90;
		constexpr MessageID ObjectTeleport = 0x91;
		constexpr MessageID ForcePhysicsUpdate = 0x92;
		constexpr MessageID PhysicsChanged = 0x93;
		constexpr MessageID LocomotionDataUpdate = 0x94;
		constexpr MessageID LocomotionDataUnreliableUpdate = 0x95;
		constexpr MessageID AttributeDataUpdate = 0x96;
		constexpr MessageID CombatantDataUpdate = 0x97;
		constexpr MessageID InteractableDataUpdate = 0x98;
		constexpr MessageID AgentBlackboardUpdate = 0x99;
		constexpr MessageID LootDataUpdate = 0x9A;
		constexpr MessageID ServerEvent = 0x9B;
		constexpr MessageID ActionCommandMsgs = 0x9C;
		constexpr MessageID PlayerDamage = 0x9D;
		constexpr MessageID ChainVoteMsgs = 0x9E;
		constexpr MessageID LootSpawned = 0x9F;
		constexpr MessageID LootAcquired = 0xA0;
		constexpr MessageID SystemMessage = 0xA1;
		constexpr MessageID LabsPlayerUpdate = 0xA2;
		constexpr MessageID ModifierCreated = 0xA3;
		constexpr MessageID ModifierUpdated = 0xA4;
		constexpr MessageID ModifierDeleted = 0xA5;
		constexpr MessageID SetAnimationState = 0xA6;
		constexpr MessageID SetObjectGfxState = 0xA7;
		constexpr MessageID PlayerCharacterDeploy = 0xA8;
		constexpr MessageID ChainGameMsgs = 0xA9;
		constexpr MessageID ChainLevelResultsMsgs = 0xAA;
		constexpr MessageID ChainCashOutMsgs = 0xAB;
		constexpr MessageID ChainPlayerMsgs = 0xAC;
		constexpr MessageID ChainGameOverMsgs = 0xAD;
		constexpr MessageID QuickGameMsgs = 0xAE;
		constexpr MessageID GamePrepareForStart = 0xAF;
		constexpr MessageID GameStart = 0xB0;
		constexpr MessageID CheatMessageDontUseInReleaseButDontChangeTheIndexOfTheMessagesBelowInCaseWeAreRunningOnADevServer = 0xB1;
		constexpr MessageID ArenaPlayerMsgs = 0xB2;
		constexpr MessageID ArenaLobbyMsgs = 0xB3;
		constexpr MessageID ArenaGameMsgs = 0xB4;
		constexpr MessageID ArenaResultsMsgs = 0xB5;
		constexpr MessageID ObjectivesInitForLevel = 0xB6;
		constexpr MessageID ObjectCreate = 0xB7;
		constexpr MessageID ObjectiveUpdated = 0xB8;
		constexpr MessageID CombatEvent = 0xB9;
		constexpr MessageID ObjectivesComplete = 0xBA;
		constexpr MessageID JuggernautLobbyMsgs = 0xBB;
		constexpr MessageID JuggernautGameMsgs = 0xBC;
		constexpr MessageID JuggernautResultsMsgs = 0xBD;
		constexpr MessageID ReloadLevel = 0xBE;
		constexpr MessageID JuggernautPlayerMsgs = 0xBF;
		constexpr MessageID GravityForceUpdate = 0xC0;
		constexpr MessageID CooldownUpdate = 0xC1;
		constexpr MessageID CrystalDragMessage = 0xC2;
		constexpr MessageID CrystalMessage = 0xC3;
		constexpr MessageID KillRacePlayerMsgs = 0xC4;
		constexpr MessageID KillRaceLobbyMsgs = 0xC5;
		constexpr MessageID KillRaceGameMsgs = 0xC6;
		constexpr MessageID KillRaceResultsMsgs = 0xC7;
		constexpr MessageID TutorialGameMsgs = 0xC8;
		constexpr MessageID CinematicMsgs = 0xC9;
		constexpr MessageID ObjectiveAdd = 0xCA;
		constexpr MessageID LootDropMessage = 0xCB;
		constexpr MessageID DebugPing = 0xCC;
	}

	// Server
	class Server {
		public:
			Server(uint16_t port, uint32_t gameId);
			~Server();

			void run_one();

			void stop();
			bool is_running();

		private:
			void ParsePacket(Packet* packet, MessageID packetType);

			void OnNewIncomingConnection(Packet* packet);
			void OnHelloPlayer(Packet* packet);
			void OnPlayerStatusUpdate(Packet* packet);

			void SendHelloPlayer(Packet* packet);
			void SendConnected(Packet* packet);
			void SendPlayerJoined(Packet* packet);
			void SendPlayerDeparted(Packet* packet);
			void SendPlayerStatusUpdate(Packet* packet, Blaze::PlayerState playerState);
			void SendGameState(Packet* packet, Blaze::GameState gameState);
			void SendGamePrepareForStart(Packet* packet);
			void SendGameStart(Packet* packet);
			void SendTutorial(Packet* packet);

			void SendTestPacket(Packet* packet, MessageID id, const std::vector<uint8_t>& data);

		private:
			std::thread mThread;
			std::mutex mMutex;
#ifdef PACKET_LOGGING
			PacketLogger mLogger;
#endif

			BitStream mInStream;

			RakPeerInterface* mSelf;

			uint32_t mGameId;

			bool mRunning = true;
	};
}

#endif
