
#ifndef _GAME_CHARACTER_HEADER
#define _GAME_CHARACTER_HEADER

// Include
#include "creature.h"

// Game
namespace Game {
	// Character
	class Character : public Creature {
		public:
			Character();
			~Character();

		protected:
			RakNet::labsCharacter mData;
	};

	using CharacterPtr = std::shared_ptr<Character>;
}

#endif
