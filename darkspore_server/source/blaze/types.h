
#ifndef _BLAZE_TYPES_HEADER
#define _BLAZE_TYPES_HEADER

// Include
#include <cstdint>
#include <string>

// Blaze
namespace Blaze {
	enum class Component {
		Authentication = 1,
		GameManager = 4,
		Redirector,
		Stats = 7,
		Util = 9,
		Clubs = 11,
		GameReporting = 0x1C,
		RSP = 0x801,
		UserSessions = 0x7802
	};

	enum class MessageType {
		Message,
		Reply,
		Notification,
		ErrorReply
	};

	enum class NetworkAddressMember {
		XboxClientAddress,
		XboxServerAddress,
		IPPAirAddress,
		IPAddress,
		HostnameAddress,
		Unset = 0x7F
	};

	enum class UpnpStatus {
		Unknown,
		Found,
		Enabled
	};

	enum class TelemetryOpt {
		OptOut,
		OptIn
	};

	enum class GameState {
		NewState,
		Initializing,
		Virtual,
		PreGame = 0x82,
		InGame = 0x83,
		PostGame = 4,
		Migrating,
		Destructing,
		Resetable,
		ReplaySetup
	};

	enum class PlayerState {
		Disconnected,
		Connected = 2
	};

	/*
	#STAS 0 = Unknown
	#STAS 1 = Pending
	#STAS 2 = Active
	#STAS 3 = Deactivated
	#STAS 4 = Disabled
	#STAS 5 = Deleted
	#STAS 6 = Banned
	*/

	enum class PresenceMode {
		None,
		Standard,
		Private
	};

	enum class VoipTopology {
		Disabled,
		DedicatedServer,
		PeerToPeer
	};

	enum class GameNetworkTopology {
		ClientServerPeerHosted,
		ClientServerDedicated,
		PeerToPeerFullMesh = 0x82,
		PeerToPeerPartialMesh,
		PeerToPeerDirtyCastFailover
	};

	enum class PlayerRemovedReason {
		PlayerJoinTimeout,
		PlayerConnLost,
		BlazeServerConnLost,
		MigrationFailed,
		GameDestroyed,
		GameEnded,
		PlayerLeft,
		GroupLeft,
		PlayerKicked,
		PlayerKickedWithBan,
		PlayerJoinFromQueueFailed,
		PlayerReservationTimeout,
		HostEjected
	};

	enum class ClientType {
		GameplayUser,
		HttpUser,
		DedicatedServer,
		Tools,
		Invalid
	};

	enum class ExternalRefType {
		Unknown,
		Xbox,
		PS3,
		Wii,
		Mobile,
		LegacyProfileID,
		Twitter,
		Facebook
	};

	enum class NatType {
		Open,
		Moderate,
		Sequential,
		Strict,
		Unknown
	};

	// Header
	struct Header {
		uint16_t length;
		Component component;
		uint16_t command;
		uint16_t error_code;
		MessageType message_type;
		uint32_t message_id;
	};
}

#endif
