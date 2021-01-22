
#ifndef _GAME_OBJECT_MANAGER_HEADER
#define _GAME_OBJECT_MANAGER_HEADER

// Include
#include "object.h"
#include <map>
#include <vector>

#ifdef GetObject
#undef GetObject
#endif

// Game
namespace Game {
	// ObjectManager
	class ObjectManager {
		public:
			ObjectPtr Get(uint32_t id) const;
			ObjectPtr Create(uint32_t noun);

			void Update();

		private:
			void Remove(Object* object);

		private:
			std::map<uint32_t, ObjectPtr::weak_type> mObjects;
			std::vector<uint32_t> mOpenIndexes;

			friend class Object;
	};
}

#endif
