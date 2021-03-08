
// Include
#include "objectmanager.h"
#include "instance.h"
#include "lua.h"

#include "utils/functions.h"
#include "utils/log.h"

// Game
namespace Game {
	// TriggerVolume
	TriggerVolume::TriggerVolume(ObjectManager& manager, uint32_t id, const glm::vec3& position, float radius) : Object(manager, id, utils::hash_id("SecurityTeleporter.noun")) {
		SetScale(0); // hide the object, effects will still show.
		SetHasCollision(false);
		SetPosition(position);

		mBoundingBox.mExtent = glm::vec3(radius);
		// mBoundingBox.mExtent.z = NAN;

		if (mNoun) {
			const auto& aidefinition = mNoun->GetAIDefinition();
			if (aidefinition) {
				SetPassiveAbility(mManager.GetGame().GetLua().GetAbility(aidefinition->GetPassiveAbility()));
			}
		}
	}

	void TriggerVolume::OnActivate() {
		if (mPassiveAbility) {
			mPassiveAbility->Activate(std::static_pointer_cast<TriggerVolume>(shared_from_this()));
		}
	}

	void TriggerVolume::OnDeactivate() {
		if (mPassiveAbility) {
			mPassiveAbility->Deactivate(std::static_pointer_cast<TriggerVolume>(shared_from_this()));
		}
	}

	void TriggerVolume::OnTick() {
		Object::OnTick();
		if (mObjectStates.empty()) {
			return;
		}

		auto radius = mBoundingBox.mExtent.x;
		for (auto it = mObjectStates.begin(); it != mObjectStates.end(); ++it) {
			bool isInRadius = glm::distance(it->first->GetPosition(), GetPosition()) <= radius;
			if (it->second == 0) {
				if (isInRadius) {
					OnEnter(it->first);
					it->second = 1;
				}
			} else if (isInRadius) {
				OnStay(it->first);
			} else {
				OnExit(it->first);
				it = mObjectStates.erase(it);
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

	void TriggerVolume::SetOnEnterCallback(sol::protected_function callback) {
		mOnEnter = std::move(callback);
		if (!mEnvironment && mOnEnter) {
			sol::state_view state = mOnEnter.lua_state();
			mEnvironment = sol::environment(state, sol::create, state.globals());
		}
	}

	void TriggerVolume::SetOnExitCallback(sol::protected_function callback) {
		mOnExit = std::move(callback);
		if (!mEnvironment && mOnExit) {
			sol::state_view state = mOnExit.lua_state();
			mEnvironment = sol::environment(state, sol::create, state.globals());
		}
	}

	void TriggerVolume::SetOnStayCallback(sol::protected_function callback) {
		mOnStay = std::move(callback);
		if (!mEnvironment && mOnStay) {
			sol::state_view state = mOnStay.lua_state();
			mEnvironment = sol::environment(state, sol::create, state.globals());
		}
	}

	void TriggerVolume::OnEnter(ObjectPtr object) const {
		if (mOnEnter) {
			mManager.GetGame().GetLua().CreateThread(mOnEnter, mEnvironment, shared_from_this(), std::move(object));
		}
	}

	void TriggerVolume::OnExit(ObjectPtr object) const {
		if (mOnExit) {
			mManager.GetGame().GetLua().CreateThread(mOnExit, mEnvironment, shared_from_this(), std::move(object));
		}
	}

	void TriggerVolume::OnStay(ObjectPtr object) const {
		if (mOnStay) {
			mManager.GetGame().GetLua().CreateThread(mOnStay, mEnvironment, shared_from_this(), std::move(object));
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

		const auto& [it, inserted] = mObjects.try_emplace(id);
		if (inserted) {
			auto object = ObjectPtr(new Object(*this, id, noun));
			it->second = object;

			mOctTree->Enqueue(object);
			mActiveObjects.insert(object);

			return std::move(object);
		}

		return nullptr;
	}

	ObjectPtr ObjectManager::Create(const MarkerPtr& marker) {
		if (!marker) {
			return nullptr;
		}

		ObjectPtr object;
		if (const auto& teleporterData = marker->GetTeleporterData()) {
			const auto& triggerVolumeData = teleporterData->GetTriggerVolumeData();
			if (triggerVolumeData) {
				object = CreateTrigger(marker->GetPosition(), triggerVolumeData->GetBoundingBox().mExtent.x);
			} else {
				object = CreateTrigger(marker->GetPosition(), 1.f);
			}
		} else {
			object = Create(marker->GetNoun());
		}

		if (object) {
			auto overrideAssetId = marker->GetOverrideAssetId();
			if (overrideAssetId > 0) {
				object->SetAssetId(overrideAssetId);
			}

			object->SetPosition(marker->GetPosition());
			object->SetOrientation(marker->GetRotation());
			object->SetMarkerId(marker->GetId());

			if (!object->IsTrigger()) {
				object->SetScale(marker->GetScale());
			}

			const auto& markerInteractableData = marker->GetInteractableData();
			if (markerInteractableData) {
				const auto& interactableData = object->CreateInteractableData();
				interactableData->SetAbility(utils::hash_id(markerInteractableData->GetAbility()));
				interactableData->SetUsesAllowed(markerInteractableData->GetUsesAllowed());
				interactableData->SetTimesUsed(0);
			}

			object->OnActivate();
		}

		return object;
	}

	TriggerVolumePtr ObjectManager::GetTrigger(uint32_t id) const {
		if (auto it = mObjects.find(id); it != mObjects.end()) {
			if (it->second->IsTrigger()) {
				return std::static_pointer_cast<TriggerVolume>(it->second);
			}
		}
		return nullptr;
	}

	TriggerVolumePtr ObjectManager::CreateTrigger(const glm::vec3& position, float radius) {
		uint32_t id = GetNextObjectId();
		if (id == 0) {
			return nullptr;
		}

		const auto& [it, inserted] = mObjects.try_emplace(id);
		if (inserted) {
			auto object = TriggerVolumePtr(new TriggerVolume(*this, id, position, radius));
			it->second = object;

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
				mActiveObjects.erase(object);

				auto id = object->GetId();
				mObjects.erase(id);
				mOpenObjectIds.push_back(id);
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
		static thread_local uint32_t sNextId = 1;

		uint32_t id;
		if (!mOpenObjectIds.empty()) {
			id = mOpenObjectIds.back();
			mOpenObjectIds.pop_back();
		} else {
			id = sNextId++;
		}

		if (id == 0) {
			// Out of ids?
		}

		return id;
	}
}
