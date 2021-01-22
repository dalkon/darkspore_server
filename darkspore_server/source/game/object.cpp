
// Include
#include "object.h"
#include "objectmanager.h"

// Game
namespace Game {
	// TODO: a bunch of more functions for this + automatic updating/deleting in batches (Game::Instance::Update() -> ObjectManager::Update())
	// Object
	Object::Object(ObjectManager& manager, uint32_t id, uint32_t noun) : mManager(manager), mId(id), mNoun(noun) {
		mData.mTeam = 0;
		mData.mbPlayerControlled = false;
		mData.mInputSyncStamp = 0;
		mData.mPlayerIdx = 0xFF;
		mData.mScale = 1.f;
		mData.mMarkerScale = 1.f;
		mData.mLastAnimationState = 0;
		mData.mLastAnimationPlayTimeMs = 0;
		mData.mOverrideMoveIdleAnimationState = 0;
		mData.mGraphicsState = 0;
		mData.mGraphicsStateStartTimeMs = 0;
		mData.mNewGraphicsStateStartTimeMs = 0;
		mData.mVisible = true;
		mData.mbHasCollision = true;
		mData.mOwnerID = 0;
		mData.mMovementType = 0; // ?
		mData.mDisableRepulsion = false;
		mData.mInteractableState = 0;
		mData.sourceMarkerKey_markerId = 0; // ?
	}

	Object::~Object() {
		mManager.Remove(this);
	}

	RakNet::sporelabsObject& Object::GetData() {
		return mData;
	}

	const RakNet::sporelabsObject& Object::GetData() const {
		return mData;
	}

	bool Object::HasAttributeData() const {
		return static_cast<bool>(mAttributes);
	}

	const RakNet::AttributeData& Object::GetAttributeData() const {
		if (!HasAttributeData()) {
			static RakNet::AttributeData _data;
			return _data;
		}
		return *mAttributes;
	}

	bool Object::HasLootData() const {
		return static_cast<bool>(mLootData);
	}

	const RakNet::cLootData& Object::GetLootData() const {
		if (!HasLootData()) {
			static RakNet::cLootData _data;
			return _data;
		}
		return *mLootData;
	}

	void Object::SetLootData(RakNet::cLootData&& lootData) {
		mLootData = std::make_unique<RakNet::cLootData>(std::move(lootData));
	}

	bool Object::HasLocomotionData() const {
		return static_cast<bool>(mLocomotionData);
	}

	const RakNet::cLocomotionData& Object::GetLocomotionData() const {
		if (!HasLocomotionData()) {
			static RakNet::cLocomotionData _data;
			return _data;
		}
		return *mLocomotionData;
	}

	uint32_t Object::GetId() const {
		return mId;
	}

	uint32_t Object::GetNoun() const {
		return mNoun;
	}

	const RakNet::cSPVector3& Object::GetPosition() const { return mData.mPosition; }
	void Object::SetPosition(const RakNet::cSPVector3& position) { mData.mPosition = position; }
	void Object::SetPosition(RakNet::cSPVector3&& position) { mData.mPosition = std::move(position); }

	const RakNet::cSPQuaternion& Object::GetOrientation() const { return mData.mOrientation; }
	void Object::SetOrientation(const RakNet::cSPQuaternion& orientation) { mData.mOrientation = orientation; }
	void Object::SetOrientation(RakNet::cSPQuaternion&& orientation) { mData.mOrientation = std::move(orientation); }

	// Properties
	float Object::GetValue(uint8_t idx) const {
		if (mAttributes) {
			return mAttributes->GetValue(idx);
		}
		return 0.f;
	}

	void Object::SetValue(uint8_t idx, float value) {
		if (!mAttributes) {
			mAttributes = std::make_unique<RakNet::AttributeData>();
		}
		mAttributes->SetValue(idx, value);
	}

	float Object::GetHealth() const {
		return mCombatantData.mHitPoints;
	}

	float Object::GetMaxHealth() const {
		if (mAttributes) {
			return mAttributes->GetValue(Ability::MaxHealth);
		}
		return 100.f;
	}

	void Object::SetHealth(float newHealth) {
		mCombatantData.mHitPoints = std::max<float>(0, std::min<float>(newHealth, GetMaxHealth()));
	}

	float Object::GetMana() const {
		return mCombatantData.mManaPoints;
	}

	float Object::GetMaxMana() const {
		if (mAttributes) {
			return mAttributes->GetValue(Ability::MaxMana);
		}
		return 100.f;
	}

	void Object::SetMana(float newMana) {
		mCombatantData.mManaPoints = std::max<float>(0, std::min<float>(newMana, GetMaxMana()));
	}

	// Combatant functions
	void Object::OnChangeHealth(float healthChange) {
		// TODO: Add scripting event (lua)
		SetHealth(GetHealth() + healthChange);
	}

	void Object::OnChangeMana(float manaChange) {
		// TODO: Add scripting event (lua)
		SetMana(GetMana() + manaChange);
	}
}
