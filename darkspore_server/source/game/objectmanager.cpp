
// Include
#include "objectmanager.h"
#include "instance.h"
#include "lua.h"

#include "utils/functions.h"

// Game
namespace Game {
	// TriggerVolume
	TriggerVolume::TriggerVolume(ObjectManager& manager, uint32_t id, const glm::vec3& position, float radius, sol::function&& onEnter, sol::function&& onExit, sol::function&& onStay) :
		Object(manager, id, utils::hash_id("SecurityTeleporter.noun")), mOnEnter(std::move(onEnter)), mOnExit(std::move(onExit)), mOnStay(std::move(onStay))
	{
		SetScale(0); // hide the object, effects will still show.
		SetHasCollision(false);

		mBoundingBox.mCenter = position;
		mBoundingBox.mExtent = glm::vec3(radius);
		// mBoundingBox.mExtent.z = NAN;

		sol::state_view state = mOnEnter.lua_state();
		mEnvironment = sol::environment(state, sol::create, state.globals());
	}

	void TriggerVolume::OnTick() {
		Object::OnTick();
		if (!mObjectStates.empty()) {
			auto& lua = mManager.GetGame().GetLua();
			auto radius = mBoundingBox.mExtent.x;
			for (auto it = mObjectStates.begin(); it != mObjectStates.end(); ++it) {
				ObjectPtr object = it->first;

				bool isInRadius = glm::distance(object->GetPosition(), GetPosition()) <= radius;
				if (it->second == 0) {
					if (isInRadius) {
						lua.CreateThread(mOnEnter, mEnvironment, shared_from_this(), std::move(object));
						it->second = 1;
					}
				} else if (isInRadius) {
					lua.CreateThread(mOnStay, mEnvironment, shared_from_this(), std::move(object));
				} else {
					lua.CreateThread(mOnExit, mEnvironment, shared_from_this(), std::move(object));
					it = mObjectStates.erase(it);
				}
			}
		}
	}

	const ObjectPtr& TriggerVolume::GetOwnerObject() const {
		return mOwnerObject;
	}

	void TriggerVolume::Attach(const ObjectPtr& object) {
		mOwnerObject = object;
	}

	void TriggerVolume::Detach(const ObjectPtr& object) {
		if (mOwnerObject == object) {
			mOwnerObject = nullptr;
		}
	}

	void TriggerVolume::AddObject(const ObjectPtr& object) {
		if (object == mOwnerObject) {
			return;
		}

		const auto& [it, inserted] = mObjectStates.try_emplace(object);
		if (inserted) {
			it->second = 0;
		}
	}

	// ObjectManager
	ObjectManager::ObjectManager(Instance& game) : mGame(game) {
		mOctTree = std::make_unique<OctTree>();
	}

	Instance& ObjectManager::GetGame() {
		return mGame;
	}

	const Instance& ObjectManager::GetGame() const {
		return mGame;
	}

	ObjectPtr ObjectManager::Get(uint32_t id) const {
		if (auto it = mObjects.find(id); it != mObjects.end()) {
			return it->second;
		}
		return nullptr;
	}

	ObjectPtr ObjectManager::Create(uint32_t noun) {
		uint32_t id = GetNextObjectId();
		if (id == 0) {
			return nullptr;
		}

		if (auto it = mObjects.try_emplace(id); it.second) {
			auto object = ObjectPtr(new Object(*this, id, noun));
			it.first->second = object;

			mOctTree->Enqueue(object);
			mActiveObjects.insert(object);

			return std::move(object);
		}

		return nullptr;
	}

	TriggerVolumePtr ObjectManager::GetTrigger(uint32_t id) const {
		if (auto it = mObjects.find(id); it != mObjects.end()) {
			if (it->second->IsTrigger()) {
				return std::static_pointer_cast<TriggerVolume>(it->second);
			}
		}
		return nullptr;
	}

	TriggerVolumePtr ObjectManager::CreateTrigger(const glm::vec3& position, float radius, sol::function&& onEnter, sol::function&& onExit, sol::function&& onStay) {
		uint32_t id = GetNextObjectId();
		if (id == 0) {
			return nullptr;
		}

		if (auto it = mObjects.try_emplace(id); it.second) {
			auto object = TriggerVolumePtr(new TriggerVolume(*this, id, position, radius, std::move(onEnter), std::move(onExit), std::move(onStay)));
			it.first->second = object;

			mOctTree->Enqueue(object);
			mActiveObjects.insert(object);

			return std::move(object);
		}

		return nullptr;
	}

	std::vector<ObjectPtr> ObjectManager::GetObjectsInRadius(const glm::vec3& position, float radius, const std::vector<NounType>& types) const {
		return mOctTree->GetObjectsInRadius(position, radius, types);
	}

	void ObjectManager::Update() {
		mOctTree->Update();
		for (const auto& object : mActiveObjects) {
			object->OnTick();
			if (object->NeedUpdate()) {
				mGame.SendObjectUpdate(object);
			}
		}

		if (!mMarkedObjects.empty()) {
			mGame.SendObjectDelete(mMarkedObjects);
			for (const auto& object : mMarkedObjects) {
				mObjects.erase(object->GetId());
				mActiveObjects.erase(object);
			}
			mMarkedObjects.clear();
		}



		// send object updates
	}

	void ObjectManager::MarkForDeletion(const ObjectPtr& object) {
		if (!object->IsMarkedForDeletion()) {
			mMarkedObjects.push_back(object);
		}
	}

	uint32_t ObjectManager::GetNextObjectId() {
		constexpr uint32_t START_ID = 1;

		uint32_t id;
		if (!mOpenObjectIds.empty()) {
			id = mOpenObjectIds.back();
			mOpenObjectIds.pop_back();
		} else if (mObjects.empty()) {
			id = START_ID;
		} else {
			auto end = mObjects.end();
			id = (--end)->first + 1;
		}

		if (id == 0) {
			// Out of ids?
		}

		return id;
	}
}
