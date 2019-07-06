
// Include
#include "server.h"

#include <MessageIdentifiers.h>
#include <RakSleep.h>
#include <RakNetTypes.h>
#include <RakNetworkFactory.h>
#include <BitStream.h>
#include <GetTime.h>

#include <iostream>

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


	8A GameState
	B6 ObjectivesInitForLevel
	8B DirectorState
	8C ObjectUpdate
	8D ObjectDelete
	90 ObjectPlayerMove
	8F ActionCommandResponse
	91 ObjectTeleport
	92 ForcePhysicsUpdate
	93 PhysicsChanged
	94 LocomotionDataUpdate
	95 LocomotionDataUnreliableUpdate
	96 AttributeDataUpdate
	97 CombatantDataUpdate
	98 InteractableDataUpdate
	99 AgentBlackboardUpdate
	9A LootDataUpdate
	A2 LabsPlayerUpdate
	9B ServerEvent
	A3 ModifierCreated
	A4 ModifierUpdated
	A5 ModifierDeleted
	AF GamePrepareForStart (disabled)
	B0 GameStart
	A6 SetAnimationState
	A7 SetObjectGfxState
	8E ObjectJump
	9D PlayerDamage
	B7 ObjectCreate
	B9 CombatEvent
	B8 ObjectiveUpdated
	CA ObjectiveAdd
	BE ReloadLevel
	BF JuggernautPlayerMsgs
	C0 GravityForceUpdate
	C1 CooldownUpdate
	C3 CrystalMessage
	87 VoteKickStarted
	85 PartyMergeComplete
	CC DebugPing

	or +1 all ids
*/

// RakNet
namespace RakNet {
	// Server
	Server::Server(uint16_t port, uint32_t gameId) : mGameId(gameId) {
		mThread = std::thread([this, port] {
			mSelf = RakNetworkFactory::GetRakPeerInterface();
			mSelf->SetTimeoutTime(30000, UNASSIGNED_SYSTEM_ADDRESS);
#ifdef PACKET_LOGGING
			mSelf->AttachPlugin(&mLogger);
#endif
			if (mSelf->Startup(4, 30, &SocketDescriptor(port, nullptr), 1)) {
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

	Server::~Server() {
		stop();
	}

	void Server::stop() {
		mMutex.lock();
		mRunning = false;
		mMutex.unlock();
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
			mInStream = BitStream(packet->data, packet->bitSize * 8, false);

			uint8_t packetType = GetPacketIdentifier();
			std::cout << std::endl << "--- "  << (int)packetType << " gotten from raknet ---" << std::endl << std::endl;
			switch (packetType) {
				case ID_DISCONNECTION_NOTIFICATION: {
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
					std::cout << "ID_CONNECTION_LOST from " << packet->systemAddress.ToString(true) << std::endl;
					break;
				}

				case ID_USER_PACKET_ENUM: {
					OnHelloPlayer(packet);
					break;
				}

				default: {
					ParsePacket(packet, packetType);
					break;
				}
			}
		}

		mInStream.Reset();
	}

	void Server::ParsePacket(Packet* packet, MessageID packetType) {
		switch (packetType) {
			case PacketID::HelloPlayer: {
				OnHelloPlayer(packet);
				break;
			}

			case PacketID::PlayerStatusUpdate: {
				OnPlayerStatusUpdate(packet);
				break;
			}

			default: {
				std::cout << "Unknown packet id: 0x" << std::hex << static_cast<int32_t>(packetType) << std::dec << std::endl;
				break;
			}
		}
	}

	void Server::OnNewIncomingConnection(Packet* packet) {
		SendConnected(packet);
		// SendHelloPlayer(packet);
		// SendPlayerJoined(packet);
		// SendPlayerDeparted(packet);
	}

	void Server::OnHelloPlayer(Packet* packet) {
		SendHelloPlayer(packet);
		// if ok
		SendPlayerJoined(packet);
		// else
		// SendPlayerDeparted(packet);

		SendGameStart(packet);
		// SendTestPacket(packet, PacketID::DirectorState, { 0x01, 0x02 });

		/*
		SendTestPacket(packet, PacketID::GamePrepareForStart + 1, { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA });
		SendGameState(packet, Blaze::GameState::PreGame);
		SendTestPacket(packet, PacketID::GameStart + 1, { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA });
		*/
		// SendGameStart(packet);
	}

	void Server::OnPlayerStatusUpdate(Packet* packet) {
		uint8_t value;
		mInStream.Read<uint8_t>(value);
		switch (static_cast<Blaze::PlayerState>(value)) {
			case Blaze::PlayerState::Reserved: {
				// Unknown
				break;
			}

			case Blaze::PlayerState::Connecting: {
				// SendPlayerStatusUpdate(packet, Blaze::PlayerState::Connected);
				// SendGamePrepareForStart(packet);
				break;
			}

			case Blaze::PlayerState::Connected: {

				break;
			}
		}
	}

	void Server::SendHelloPlayer(Packet* packet) {
		const auto& guid = mSelf->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);

		auto addr = mSelf->GetSystemAddressFromGuid(guid);
		/*
		if (1) { // add check for local
			addr = SystemAddress("127.0.0.1", addr.port);
		}
		*/

		/*
			u8: type
			u8: gameIndex
			u32: ip
			u16: port
		*/

		BitStream outStream(8);
		outStream.Write(PacketID::HelloPlayer);
		outStream.Write<uint8_t>(static_cast<uint8_t>(Blaze::GameType::Tutorial));
		outStream.Write<uint8_t>(mGameId);
		outStream.WriteBits(reinterpret_cast<const uint8_t*>(&addr.binaryAddress), sizeof(addr.binaryAddress) * 8, true);
		outStream.Write(addr.port);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendConnected(Packet* packet) {
		// TODO: verify incoming connection

		BitStream outStream(8);
		outStream.Write(PacketID::Connected);
		outStream.Write<uint8_t>(0x12);
		outStream.Write<uint8_t>(0x34);
		outStream.Write<uint8_t>(0x56);
		outStream.Write<uint8_t>(0x78);
		outStream.Write<uint8_t>(0x90);
		outStream.Write<uint8_t>(0xAB);
		outStream.Write<uint8_t>(0xCD);
		outStream.Write<uint8_t>(0xEF);
		outStream.Write<uint8_t>(0xFE);
		outStream.Write<uint8_t>(0xDC);
		outStream.Write<uint8_t>(0xBA);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendPlayerJoined(Packet* packet) {
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerJoined);
		outStream.Write<uint8_t>(mGameId);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendPlayerDeparted(Packet* packet) {
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerDeparted);
		outStream.Write<uint8_t>(mGameId);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendPlayerStatusUpdate(Packet* packet, Blaze::PlayerState playerState) {
		BitStream outStream(8);
		outStream.Write(PacketID::PlayerStatusUpdate);
		outStream.Write<uint8_t>(static_cast<uint8_t>(playerState));

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendGameState(Packet* packet, Blaze::GameState gameState) {
		BitStream outStream(8);
		outStream.Write(PacketID::GameState);
		// outStream.Write<uint8_t>(static_cast<uint8_t>(gameState));
		outStream.Write<uint8_t>(0x12);
		outStream.Write<uint8_t>(0x34);
		outStream.Write<uint8_t>(0x56);
		outStream.Write<uint8_t>(0x78);
		outStream.Write<uint8_t>(0x90);
		outStream.Write<uint8_t>(0xAB);
		outStream.Write<uint8_t>(0xCD);
		outStream.Write<uint8_t>(0xEF);
		outStream.Write<uint8_t>(0xFE);
		outStream.Write<uint8_t>(0xDC);
		outStream.Write<uint8_t>(0xBA);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendGameStart(Packet* packet) {
		BitStream outStream(8);
		outStream.Write(PacketID::GameStart);
		// outStream.Write<uint8_t>(0xFA);
		outStream.Write<uint8_t>(0x12);
		outStream.Write<uint8_t>(0x34);
		outStream.Write<uint8_t>(0x56);
		outStream.Write<uint8_t>(0x78);
		outStream.Write<uint8_t>(0x90);
		outStream.Write<uint8_t>(0xAB);
		outStream.Write<uint8_t>(0xCD);
		outStream.Write<uint8_t>(0xEF);
		outStream.Write<uint8_t>(0xFE);
		outStream.Write<uint8_t>(0xDC);
		outStream.Write<uint8_t>(0xBA);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendGamePrepareForStart(Packet* packet) {
		BitStream outStream(8);
		outStream.Write(PacketID::GamePrepareForStart);
		// outStream.Write<uint8_t>(0xBA);
		outStream.Write<uint8_t>(0x12);
		outStream.Write<uint8_t>(0x34);
		outStream.Write<uint8_t>(0x56);
		outStream.Write<uint8_t>(0x78);
		outStream.Write<uint8_t>(0x90);
		outStream.Write<uint8_t>(0xAB);
		outStream.Write<uint8_t>(0xCD);
		outStream.Write<uint8_t>(0xEF);
		outStream.Write<uint8_t>(0xFE);
		outStream.Write<uint8_t>(0xDC);
		outStream.Write<uint8_t>(0xBA);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendTutorial(Packet* packet) {
		BitStream outStream(8);
		outStream.Write(PacketID::TutorialGameMsgs);

		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}

	void Server::SendTestPacket(Packet* packet, MessageID id, const std::vector<uint8_t>& data) {
		BitStream outStream(8);
		outStream.Write(id);
		for (auto byte : data) {
			outStream.Write(byte);
		}
		mSelf->Send(&outStream, HIGH_PRIORITY, UNRELIABLE_WITH_ACK_RECEIPT, 0, packet->systemAddress, false);
	}
}
