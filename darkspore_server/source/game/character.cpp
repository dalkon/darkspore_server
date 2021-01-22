
// Include
#include "character.h"

// Game
namespace Game {
	// Character
	Character::Character() : Creature() {
		decltype(auto) data = mObject->GetData();
		data.mbPlayerControlled = true;
	}

	Character::~Character() {

	}
}
