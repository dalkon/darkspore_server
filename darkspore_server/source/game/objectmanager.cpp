
// Include
#include "objectmanager.h"

// Game
namespace Game {
	// ObjectManager
	ObjectPtr ObjectManager::Get(uint32_t id) const {
		auto it = mObjects.find(id);
		return (it != mObjects.end()) ? it->second.lock() : nullptr;
	}

	ObjectPtr ObjectManager::Create(uint32_t noun) {
		constexpr uint32_t START_ID = 0xF000;

		uint32_t id;
		if (!mOpenIndexes.empty()) {
			id = mOpenIndexes.back();
			mOpenIndexes.pop_back();
		} else if (mObjects.empty()) {
			id = START_ID;
		} else {
			auto end = mObjects.end();
			id = (--end)->first + 1;
		}

		auto it = mObjects.try_emplace(id);
		if (it.second) {
			auto object = ObjectPtr(new Object(*this, id, noun));
			it.first->second = object;
			return std::move(object);
		}

		return nullptr;
	}

	void ObjectManager::Update() {
		// 
	}

	void ObjectManager::Remove(Object* object) {
		if (object) {
			mObjects.erase(object->mId);
			mOpenIndexes.push_back(object->mId);
		}
	}
}
