
#ifndef _BLAZE_TYPES_HEADER
#define _BLAZE_TYPES_HEADER

// Include
#include <cstdint>
#include <string>

// Blaze
namespace Blaze {
	enum class Component {
		Authentication = 0x01,
		GameManager = 0x04,
		Redirector = 0x05,
		Playgrounds = 0x06,
		Stats = 0x07,
		Util = 0x09,
		CensusData = 0x0A,
		Clubs = 0x0B,
		Messaging = 0x0F,
		Rooms = 0x15,
		AssociationLists = 0x19,
		GameReporting = 0x1C,
		RSP = 0x801,
		Teams = 0x816,
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
		IpPairAddress,
		IpAddress,
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

	enum class Slot {
		Public = 0,
		Private
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
		ReplaySetup,
	};

	enum class SessionState {
		Idle = 0,
		Connecting,
		Connected,
		Authenticated,
		Invalid
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
		uint16_t error_code = 0;
		MessageType message_type;
		uint32_t message_id = 0;
	};
}

#endif
