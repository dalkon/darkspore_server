
#ifndef _PREDEFINED_TYPES_HEADER
#define _PREDEFINED_TYPES_HEADER

// Include
#include <memory>

// Game
namespace Game {
	// classes
	class Instance;
	class Player;
	class Object;
	class TriggerVolume;
	class ServerEventBase;
	class CombatEvent;
	class Attributes;

	// shared_ptrs
	using InstancePtr = std::shared_ptr<Instance>;
	using PlayerPtr = std::shared_ptr<Player>;
	using ObjectPtr = std::shared_ptr<Object>;
	using TriggerVolumePtr = std::shared_ptr<TriggerVolume>;
}

// RakNet
namespace RakNet {
	// classes
	class Server;
	class Client;

	// shared_ptrs
	using ClientPtr = std::shared_ptr<Client>;
}

// SporeNet
namespace SporeNet {
	// classes
	class Instance;
	class Vendor;

	class TemplateCreature;
	class TemplateDatabase;
	class Creature;

	class Room;

	class Squad;

	class User;
	class UserManager;

	// shared_ptrs
	using TemplateCreaturePtr = std::shared_ptr<TemplateCreature>;
	using CreaturePtr = std::shared_ptr<Creature>;

	using RoomPtr = std::shared_ptr<Room>;

	using SquadPtr = std::shared_ptr<Squad>;

	using UserPtr = std::shared_ptr<User>;
}

#endif
