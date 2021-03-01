
// Include
#include "object.h"
#include "objectmanager.h"

#include "utils/log.h"

// Game
namespace Game {
	// EffectList
	uint8_t EffectList::Add(uint32_t effect) {
		uint8_t index;
		if (mOpenIndexes.empty()) {
			if (mUsedIndexes.size() < 0xFF) {
				mUsedIndexes.push_back(1);
				index = mUsedIndexes.back();
			} else {
				index = 0xFF;
			}
		} else {
			index = mOpenIndexes.back();
			mOpenIndexes.pop_back();
		}

		if (index != 0xFF) {
			mIndexByEffect[effect] = index;
		}

		return index;
	}

	bool EffectList::Remove(uint32_t effect) {
		auto mapEnd = mIndexByEffect.end();
		auto mapIt = mIndexByEffect.find(effect);
		if (mapIt != mapEnd) {
			mIndexByEffect.erase(mapIt);
			return true;
		}
		return false;
	}

	bool EffectList::RemoveByIndex(uint8_t index) {
		auto end = mUsedIndexes.end();
		auto it = std::find(mUsedIndexes.begin(), end, index);
		if (it != end) {
			mUsedIndexes.erase(it);
			mOpenIndexes.push_back(index);

			auto mapEnd = mIndexByEffect.end();
			auto mapIt = std::find_if(mIndexByEffect.begin(), mapEnd, [index](const auto& entry) { return entry.second == index; });
			if (mapIt != mapEnd) {
				mIndexByEffect.erase(mapIt);
			}
			return true;
		}
		return false;
	}

	// Object
	Object::Object(ObjectManager& manager, uint32_t id, uint32_t noun) : mManager(manager), mId(id), mNounId(noun) {
		mNoun = NounDatabase::Instance().Get(mNounId);

		mBoundingBox.mCenter = glm::vec3(0);
		mBoundingBox.mExtent = glm::vec3(1);

		if (mNoun) {
			mAssetId = mNoun->GetAssetId();
			if (mNoun->IsCreature()) {
				mAgentBlackboardData = std::make_unique<RakNet::cAgentBlackboard>();
			}

			if (mNoun->IsPlayer()) {
				// Later
			} else {
				const auto& data = mNoun->GetNonPlayerClassData();
				if (!data) {
					return;
				}

				SetTargetable(data->IsTargetable());

				const auto& attributes = data->GetAttributes();
				if (attributes) {
					auto health = attributes->GetBaseAttribute(ClassAttribute::Health);
					if (health > 0) {
						SetAttributeValue(Attribute::MaxHealth, health);
						SetHealth(GetMaxHealth());
					}

					auto mana = attributes->GetBaseAttribute(ClassAttribute::Mana);
					if (mana > 0) {
						SetAttributeValue(Attribute::MaxMana, mana);
						SetMana(GetMaxMana());
					}
				}
			}
		}
	}

	void Object::OnActivate() {
		// nothing yet
	}

	void Object::OnDeactivate() {
		// nothing yet
	}

	void Object::OnTick() {
		if (mTickOverride) {
			mTickOverride.call<void>(shared_from_this());
		}
	}

	void Object::SetTickOverride(sol::protected_function func) {
		mTickOverride = func;
	}

	const std::unique_ptr<RakNet::cInteractableData>& Object::GetInteractableData() const {
		return mInteractableData;
	}

	void Object::SetInteractableData(const RakNet::cInteractableData& data) {
		mInteractableData = std::make_unique<RakNet::cInteractableData>(data);
		SetFlags(GetFlags() | Flags::UpdateInteractableData);
	}

	RakNet::cCombatantData& Object::GetCombatantData() {
		return mCombatantData;
	}

	const RakNet::cCombatantData& Object::GetCombatantData() const {
		return mCombatantData;
	}

	bool Object::HasAttributeData() const {
		return static_cast<bool>(mAttributes);
	}

	const std::unique_ptr<Attributes>& Object::GetAttributeData() const {
		return mAttributes;
	}

	bool Object::HasLootData() const {
		return static_cast<bool>(mLootData);
	}

	const RakNet::cLootData& Object::GetLootData() const {
		if (!HasLootData()) {
			static RakNet::cLootData _data {};
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

	const RakNet::LocomotionData& Object::GetLocomotionData() const {
		if (!HasLocomotionData()) {
			static RakNet::LocomotionData _data {};
			return _data;
		}
		return *mLocomotionData;
	}

	bool Object::HasAgentBlackboardData() const {
		return static_cast<bool>(mAgentBlackboardData);
	}

	const RakNet::cAgentBlackboard& Object::GetAgentBlackboardData() const {
		if (!HasAgentBlackboardData()) {
			static RakNet::cAgentBlackboard _data;
			return _data;
		}
		return *mAgentBlackboardData;
	}

	uint64_t Object::GetAssetId() const {
		return mAssetId;
	}

	void Object::SetAssetId(uint64_t assetId) {
		mAssetId = assetId;
	}

	const NounPtr& Object::GetNoun() const {
		return mNoun;
	}

	uint32_t Object::GetId() const {
		return mId;
	}

	uint32_t Object::GetNounId() const {
		return mNounId;
	}

	NounType Object::GetType() const {
		return mNoun ? mNoun->GetType() : NounType::None;
	}

	const glm::vec3& Object::GetPosition() const {
		return mBoundingBox.mCenter;
	}

	void Object::SetPosition(const glm::vec3& position) {
		bool dirty = position != mBoundingBox.mCenter;
		mBoundingBox.mCenter = position;
		if (dirty) {
			SetDirty(true);
		}
		mDataBits.set(ObjectDataBits::Position);
	}

	const glm::quat& Object::GetOrientation() const {
		return mOrientation;
	}

	void Object::SetOrientation(const glm::quat& orientation) {
		mOrientation = orientation;
		mDataBits.set(ObjectDataBits::Orientation);
	}

	const BoundingBox& Object::GetBoundingBox() const {
		return mBoundingBox;
	}

	float Object::GetFootprintRadius() const {
		constexpr std::array<std::tuple<glm::vec3, float>, 17> objectExtents {
			// Critter
			std::make_tuple(glm::vec3(1, 1, 1), -10.f),
			std::make_tuple(glm::vec3(1, 1, 1), -10.f),

			// Minion
			std::make_tuple(glm::vec3(1.5f, 1.5f, 2.25f), 1.f),
			std::make_tuple(glm::vec3(1.6f, 1.6f, 1.5f), 1.f),

			// Elite minion
			std::make_tuple(glm::vec3(1.5f, 1.5f, 2.5f), 1.f),
			std::make_tuple(glm::vec3(1.75f, 1.75f, 1.75f), 1.f),

			// Player (ravager, tempest, sentinel)
			std::make_tuple(glm::vec3(1.5f, 1.5f, 3.75f), 1.f),
			std::make_tuple(glm::vec3(1.75f, 1.75f, 3.5f), 1.f),
			std::make_tuple(glm::vec3(2, 2, 4), 1.f),

			// Lieutenant
			std::make_tuple(glm::vec3(2.2f, 2.2f, 4), 1.f),
			std::make_tuple(glm::vec3(2.5f, 2.5f, 3.5f), 1.f),

			// Elite lieutenant
			std::make_tuple(glm::vec3(2.25f, 2.25f, 5.5f), 1.f),
			std::make_tuple(glm::vec3(2.5f, 2.5f, 4.5f), 1.f),

			// Captain
			std::make_tuple(glm::vec3(3, 3, 5), 1.f),
			std::make_tuple(glm::vec3(3.5f, 3.5f, 4.5f), 1.f),
			
			// Boss
			std::make_tuple(glm::vec3(4.5f, 4.5f, 9), 1.f),
			std::make_tuple(glm::vec3(5.5f, 5.5f, 8), 1.f)
		};

		float result = 0;
		if (mNoun) {
			auto presetExtents = mNoun->GetPresetExtents();
			if (presetExtents == PresetExtents::None) {
				const auto& bBox = mNoun->GetBoundingBox();

				auto min = bBox.GetMin();
				auto max = bBox.GetMax();

				auto x = glm::min(glm::abs(min.x), glm::abs(max.x));
				auto y = glm::min(glm::abs(min.y), glm::abs(max.y));

				result = glm::sqrt((x * x) + (y * y)) * GetScale();
			} else {
				auto presetIndex = static_cast<uint32_t>(presetExtents) - 1;
				if (objectExtents.size() < presetIndex) {
					const auto& [extent, _] = objectExtents[presetIndex];
					result = glm::sqrt(extent.x * extent.x * 2) * GetScale() * 0.5f;
				}
			}
		}

		return result;
	}

	// Effects
	uint8_t Object::AddEffect(uint32_t effect) {
		if (!mEffects) {
			mEffects = std::make_unique<EffectList>();
		}
		return mEffects->Add(effect);
	}

	bool Object::RemoveEffect(uint32_t effect) {
		if (mEffects) {
			return mEffects->Remove(effect);
		}
		return false;
	}

	bool Object::RemoveEffectByIndex(uint8_t index) {
		if (mEffects) {
			return mEffects->RemoveByIndex(index);
		}
		return false;
	}

	// Properties
	float Object::GetAttributeValue(uint8_t idx) const {
		if (mAttributes) {
			return mAttributes->GetValue(idx);
		}
		return 0.f;
	}

	void Object::SetAttributeValue(uint8_t idx, float value) {
		if (!mAttributes) {
			mAttributes = std::make_unique<Attributes>();
		}
		mAttributes->SetValue(idx, value);
		SetFlags(GetFlags() | Flags::UpdateAttributes);
	}

	float Object::GetHealth() const {
		return mCombatantData.mHitPoints;
	}

	float Object::GetMaxHealth() const {
		if (mAttributes) {
			return mAttributes->GetValue(Attribute::MaxHealth);
		}
		return 100.f;
	}

	void Object::SetHealth(float newHealth) {
		mCombatantData.mHitPoints = std::max<float>(0, std::min<float>(newHealth, GetMaxHealth()));
		SetFlags(GetFlags() | Flags::UpdateCombatant);
	}

	float Object::GetMana() const {
		return mCombatantData.mManaPoints;
	}

	float Object::GetMaxMana() const {
		if (mAttributes) {
			return mAttributes->GetValue(Attribute::MaxMana);
		}
		return 100.f;
	}

	void Object::SetMana(float newMana) {
		mCombatantData.mManaPoints = std::max<float>(0, std::min<float>(newMana, GetMaxMana()));
		SetFlags(GetFlags() | Flags::UpdateCombatant);
	}

	uint32_t Object::GetInteractableState() const {
		return mInteractableState;
	}

	void Object::SetInteractableState(uint32_t interactableState) {
		mInteractableState = interactableState;
		mDataBits.set(ObjectDataBits::InteractableState);
	}

	uint32_t Object::GetMarkerId() const {
		return mMarkerId;
	}

	void Object::SetMarkerId(uint32_t markerId) {
		mMarkerId = markerId;
		mDataBits.set(ObjectDataBits::MarkerId);
	}

	float Object::GetScale() const {
		return mScale;
	}

	void Object::SetScale(float scale) {
		mScale = scale;
		mDataBits.set(ObjectDataBits::Scale);
	}

	uint8_t Object::GetTeam() const {
		return mTeam;
	}

	void Object::SetTeam(uint8_t team) {
		mTeam = team;
		mDataBits.set(ObjectDataBits::Team);
	}

	uint8_t Object::GetMovementType() const {
		return mMovementType;
	}

	void Object::SetMovementType(uint8_t movementType) {
		mMovementType = movementType;
		mDataBits.set(ObjectDataBits::MovementType);
	}

	uint8_t Object::GetPlayerIndex() const {
		return mPlayerIndex;
	}

	void Object::SetPlayerIndex(uint8_t playerIndex) {
		mPlayerIndex = playerIndex;
		mbPlayerControlled = mPlayerIndex < 0xFF;
		mDataBits.set(ObjectDataBits::PlayerIndex);
		mDataBits.set(ObjectDataBits::PlayerControlled);
	}

	bool Object::IsPlayerControlled() const {
		return mbPlayerControlled;
	}

	bool Object::HasCollision() const {
		return mbHasCollision;
	}

	void Object::SetHasCollision(bool collision) {
		mbHasCollision = collision;
		mDataBits.set(ObjectDataBits::HasCollision);
	}

	bool Object::IsVisible() const {
		return mVisible;
	}

	void Object::SetVisible(bool visible) {
		mVisible = visible;
		mDataBits.set(ObjectDataBits::Visible);
	}

	bool Object::IsDirty() const {
		return mFlags & Flags::Dirty;
	}

	void Object::SetDirty(bool dirty) {
		if (dirty) {
			SetFlags(GetFlags() | Flags::Dirty);
		} else {
			SetFlags(GetFlags() & ~Flags::Dirty);
		}
	}

	bool Object::IsMarkedForDeletion() const {
		return mFlags & Flags::MarkedForDeletion;
	}

	void Object::MarkForDeletion() {
		mManager.MarkForDeletion(shared_from_this());
		SetFlags(GetFlags() | Flags::MarkedForDeletion);
	}

	// Combatant functions
	std::tuple<bool, float, bool> Object::TakeDamage() {
		float damage = 0;
		return { true, damage, false };
	}

	std::tuple<float, bool> Object::Heal() {
		return { 0.f, false };
	}

	void Object::OnChangeHealth(float healthChange) {
		// TODO: Add scripting event (lua)
		SetHealth(GetHealth() + healthChange);
	}

	void Object::OnChangeMana(float manaChange) {
		// TODO: Add scripting event (lua)
		SetMana(GetMana() + manaChange);
	}

	// Agent Blackboard
	uint32_t Object::GetTargetId() const {
		return GetAgentBlackboardData().targetId;
	}

	void Object::SetTargetId(uint32_t id) {
		if (mAgentBlackboardData) {
			mAgentBlackboardData->targetId = id;
			SetFlags(GetFlags() | Flags::UpdateAgentBlackboardData);
		}
	}

	uint8_t Object::GetStealthType() const {
		return GetAgentBlackboardData().mStealthType;
	}

	void Object::SetStealthType(uint8_t stealthType) {
		if (mAgentBlackboardData) {
			mAgentBlackboardData->mStealthType = stealthType;
			SetFlags(GetFlags() | Flags::UpdateAgentBlackboardData);
		}
	}

	bool Object::IsInCombat() const {
		return GetAgentBlackboardData().mbInCombat;
	}

	void Object::SetInCombat(bool inCombat) {
		if (mAgentBlackboardData) {
			mAgentBlackboardData->mbInCombat = inCombat;
			SetFlags(GetFlags() | Flags::UpdateAgentBlackboardData);
		}
	}

	bool Object::IsTargetable() const {
		return GetAgentBlackboardData().mbTargetable;
	}

	void Object::SetTargetable(bool targetable) {
		if (mAgentBlackboardData) {
			mAgentBlackboardData->mbTargetable = targetable;
			SetFlags(GetFlags() | Flags::UpdateAgentBlackboardData);
		}
	}

	// Network & reflection
	void Object::WriteTo(RakNet::BitStream& stream) const {
		/*
			Movement type
				0 = nothing special?
				1 = projectile? (uses locomotion projectile params)
				2 = projectile again
				3 = projectile again
				4 = bouncing? (uses locomotion lob params)
				5 = uses locomotion offset... somehow
				6 = projectile again
		*/
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x308);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x010));
		Write<float>(stream, mScale);
		Write<float>(stream, mMarkerScale);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x018));
		Write(stream, GetPosition());
		Write(stream, mOrientation);
		Write(stream, mLinearVelocity);
		Write(stream, mAngularVelocity);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x050));
		Write(stream, mOwnerId);
		Write<uint8_t>(stream, mTeam);
		Write<uint8_t>(stream, mPlayerIndex);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x058));
		Write<uint32_t>(stream, mInputSyncStamp);
		Write<bool>(stream, mbPlayerControlled);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x05F));
		Write<bool>(stream, mVisible);
		Write<bool>(stream, mbHasCollision);
		Write<uint8_t>(stream, mMovementType); // 0 to 6

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x088));
		Write(stream, mMarkerId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0AC));
		Write(stream, mLastAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B8));
		Write(stream, mLastAnimationPlayTime);
		Write(stream, mOverrideMoveIdleAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x258));
		Write(stream, mGraphicsState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x260));
		Write(stream, mGraphicsStateStartTime);
		Write(stream, mNewGraphicsStateStartTime);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x284));
		Write(stream, mDisableRepulsion);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x288));
		Write(stream, mInteractableState);

		stream.SetWriteOffset(writeOffset + size);
	}

	void Object::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<23> reflector(stream);
		reflector.begin();
		if (mDataBits.any()) {
			if (mDataBits.test(0)) { reflector.write<0>(mTeam); }
			if (mDataBits.test(1)) { reflector.write<1>(mbPlayerControlled); }
			if (mDataBits.test(2)) { reflector.write<2>(mInputSyncStamp); }
			if (mDataBits.test(3)) { reflector.write<3>(mPlayerIndex); }
			if (mDataBits.test(4)) { reflector.write<4>(mLinearVelocity); }
			if (mDataBits.test(5)) { reflector.write<5>(mAngularVelocity); }
			if (mDataBits.test(6)) { reflector.write<6>(GetPosition()); }
			if (mDataBits.test(7)) { reflector.write<7>(mOrientation); }
			if (mDataBits.test(8)) { reflector.write<8>(mScale); }
			if (mDataBits.test(9)) { reflector.write<9>(mMarkerScale); }
			if (mDataBits.test(10)) { reflector.write<10>(mLastAnimationState); }
			if (mDataBits.test(11)) { reflector.write<11>(mLastAnimationPlayTime); }
			if (mDataBits.test(12)) { reflector.write<12>(mOverrideMoveIdleAnimationState); }
			if (mDataBits.test(13)) { reflector.write<13>(mGraphicsState); }
			if (mDataBits.test(14)) { reflector.write<14>(mGraphicsStateStartTime); }
			if (mDataBits.test(15)) { reflector.write<15>(mNewGraphicsStateStartTime); }
			if (mDataBits.test(16)) { reflector.write<16>(mVisible); }
			if (mDataBits.test(17)) { reflector.write<17>(mbHasCollision); }
			if (mDataBits.test(18)) { reflector.write<18>(mOwnerId); }
			if (mDataBits.test(19)) { reflector.write<19>(mMovementType); }
			if (mDataBits.test(20)) { reflector.write<20>(mDisableRepulsion); }
			if (mDataBits.test(21)) { reflector.write<21>(mInteractableState); }
			if (mDataBits.test(22)) { reflector.write<22>(mMarkerId); }
		}
		reflector.end();
	}

	void Object::ResetUpdateBits() {
		mDataBits.reset();
	}

	bool Object::NeedUpdate() const {
		return (mFlags & Object::UpdateFlags) || mDataBits.any();
	}

	uint8_t Object::GetFlags() const {
		return mFlags;
	}

	void Object::SetFlags(uint8_t flags) {
		mFlags = static_cast<Flags>(flags);
	}
}
