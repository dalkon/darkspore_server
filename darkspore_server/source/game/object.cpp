
// Include
#include "object.h"
#include "objectmanager.h"
#include "instance.h"

#include "sporenet/part.h"

#include "utils/functions.h"
#include "utils/log.h"

// magic numbers
constexpr std::array<float, 19> magicNumbers {
	1.f, // DamagePerPointOfStrength
	1.f, // DamagePerPointOfDexterity
	1.f, // DamagePerPointOfMind
	5.f, // HealthPerPointofStrength
	6.f, // PhysicalDefensePerPointofDexterity
	4.f, // CritRatingPerPointofDexterity
	6.f, // EnergyDefensePerPointofMind
	1.f, // ManaPerPointofMind
	6.f, // DefenseRatingDecreaseMultiplier
	1.05f, // DefenseRatingDecreaseBase
	6.f, // CriticalRatingDecreaseMultiplier
	1.05f, // CriticalRatingDecreaseBase
	2.f, // CriticalDamageBonus
	8.f, // PrimaryAttributeIgnoreAmount
	0.5f, // LeechEffectivenessForAoE
	1.f, // LeechEffectivenessForAbilities
	1.f, // LeechEffectivenessForBasics
	0.8f, // DodgePercentCap
	0.8f // ResistPercentCap
};

// difficulty tuning


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

	// CombatantData
	void CombatantData::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x70);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x40));
		Write(stream, mHitPoints);
		Write(stream, mManaPoints);

		stream.SetWriteOffset(writeOffset + size);
	}

	void CombatantData::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<2> reflector(stream);
		reflector.begin();
		reflector.write<0>(mHitPoints);
		reflector.write<1>(mManaPoints);
		reflector.end();
	}

	// InteractableData
	InteractableData::InteractableData(Object& object) : mObject(object) {}

	uint32_t InteractableData::GetAbility() const {
		return mAbility;
	}

	void InteractableData::SetAbility(uint32_t ability) {
		mAbility = ability;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateInteractableData);
	}

	int32_t InteractableData::GetTimesUsed() const {
		return mTimesUsed;
	}

	void InteractableData::SetTimesUsed(int32_t timesUsed) {
		mTimesUsed = timesUsed;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateInteractableData);
	}

	int32_t InteractableData::GetUsesAllowed() const {
		return mUsesAllowed;
	}

	void InteractableData::SetUsesAllowed(int32_t usesAllowed) {
		mUsesAllowed = usesAllowed;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateInteractableData);
	}

	void InteractableData::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x34);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, mTimesUsed);
		Write(stream, mUsesAllowed);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x14));
		Write(stream, mAbility);

		stream.SetWriteOffset(writeOffset + size);
	}

	void InteractableData::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<3> reflector(stream);
		reflector.begin();
		reflector.write<0>(mTimesUsed);
		reflector.write<1>(mUsesAllowed);
		reflector.write<2>(mAbility);
		reflector.end();
	}

	// LootData
	LootData::LootData(Object& object) : mObject(object) {}

	uint32_t LootData::GetRigblockAsset() const {
		return mRigblockAsset;
	}

	uint32_t LootData::GetPrefixAsset() const {
		return mPrefixAssetId;
	}

	uint32_t LootData::GetPrefixSecondaryAsset() const {
		return mSecondaryPrefixAssetId;
	}

	uint32_t LootData::GetSuffixAsset() const {
		return mSuffixAssetId;
	}

	int32_t LootData::GetLevel() const {
		return mItemLevel;
	}

	int32_t LootData::GetRarity() const {
		return mRarity;
	}

	void LootData::SetId(uint64_t id) {
		mId = id;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLootData);
	}

	void LootData::SetInstanceId(uint64_t id) {
		mInstanceId = id;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLootData);
	}

	void LootData::SetPart(const SporeNet::Part& part) {
		mRigblockAsset = part.GetRigblockAssetHash();
		mSuffixAssetId = part.GetSuffixAssetHash();
		mPrefixAssetId = part.GetPrefixAssetHash();
		mSecondaryPrefixAssetId = part.GetPrefixSecondaryAssetHash();
		mItemLevel = part.GetLevel();
		mRarity = static_cast<int32_t>(part.GetRarity());
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLootData);
	}

	void LootData::SetDNAAmount(float amount) {
		mDNAAmount = amount;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLootData);
	}

	void LootData::SetCrystal(const RakNet::labsCrystal& crystal) {
		mCrystalLevel = crystal.level;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLootData);
	}

	void LootData::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x80);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, mCrystalLevel);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x10));
		Write(stream, mId);
		Write(stream, mRigblockAsset);
		Write(stream, mSuffixAssetId);
		Write(stream, mPrefixAssetId);
		Write(stream, mSecondaryPrefixAssetId);
		Write(stream, mItemLevel);
		Write(stream, mRarity);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x40));
		Write(stream, mInstanceId);
		Write(stream, mDNAAmount);

		stream.SetWriteOffset(writeOffset + size);
	}

	void LootData::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<10> reflector(stream);
		reflector.begin();
		reflector.write<0>(mCrystalLevel);
		reflector.write<1>(mId);
		reflector.write<2>(mRigblockAsset);
		reflector.write<3>(mSuffixAssetId);
		reflector.write<4>(mPrefixAssetId);
		reflector.write<5>(mSecondaryPrefixAssetId);
		reflector.write<6>(mItemLevel);
		reflector.write<7>(mRarity);
		reflector.write<8>(mInstanceId);
		reflector.write<9>(mDNAAmount);
		reflector.end();
	}

	// AgentBlackboard
	AgentBlackboard::AgentBlackboard(Object& object) : mObject(object) {}

	uint32_t AgentBlackboard::GetTargetId() const {
		return mTargetId;
	}

	void AgentBlackboard::SetTargetId(uint32_t id) {
		mTargetId = id;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateAgentBlackboardData);
	}

	uint32_t AgentBlackboard::GetNumAttackers() const {
		return mNumAttackers;
	}

	void AgentBlackboard::SetNumAttackers(uint32_t attackers) {
		mNumAttackers = attackers;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateAgentBlackboardData);
	}

	uint8_t AgentBlackboard::GetStealthType() const {
		return mStealthType;
	}

	void AgentBlackboard::SetStealthType(uint8_t stealthType) {
		mStealthType = stealthType;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateAgentBlackboardData);
	}

	bool AgentBlackboard::IsInCombat() const {
		return mInCombat;
	}

	void AgentBlackboard::SetInCombat(bool inCombat) {
		mInCombat = inCombat;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateAgentBlackboardData);
	}

	bool AgentBlackboard::IsTargetable() const {
		return mTargetable;
	}

	void AgentBlackboard::SetTargetable(bool targetable) {
		mTargetable = targetable;
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateAgentBlackboardData);
	}

	void AgentBlackboard::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x598);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x550));
		Write(stream, mTargetId);
		Write(stream, mInCombat);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x556));
		Write(stream, mStealthType);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x578));
		Write(stream, mTargetable);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x57C));
		Write(stream, mNumAttackers);

		stream.SetWriteOffset(writeOffset + size);
	}

	void AgentBlackboard::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<5> reflector(stream);
		reflector.begin();
		reflector.write<0>(mTargetId);
		reflector.write<1>(mInCombat);
		reflector.write<2>(mStealthType);
		reflector.write<3>(mTargetable);
		reflector.write<4>(mNumAttackers);
		reflector.end();
	}

	// Object
	Object::Object(ObjectManager& manager, uint32_t id, uint32_t noun) : mManager(manager), mId(id), mNounId(noun) {
		mNoun = NounDatabase::Instance().Get(mNounId);

		mBoundingBox.mCenter = glm::vec3(0);
		mBoundingBox.mExtent = glm::vec3(1);

		if (mNoun) {
			mAssetId = mNoun->GetAssetId();
			if (mNoun->IsCreature()) {
				CreateAgentBlackboardData();
				CreateLocomotionData();
			}

			if (mNoun->IsPlayer()) {
				const auto& data = mNoun->GetPlayerClassData();
				if (!data) {
					return;
				}

				const auto& attributes = data->GetAttributes();
				if (attributes) {
					auto health = attributes->GetBaseAttribute(ClassAttribute::Health);
					if (health > 0) {
						SetAttributeValue(AttributeType::MaxHealth, health);
						SetHealth(GetMaxHealth());
					}

					auto mana = attributes->GetBaseAttribute(ClassAttribute::Mana);
					if (mana > 0) {
						SetAttributeValue(AttributeType::MaxMana, mana);
						SetMana(GetMaxMana());
					}

					auto nonCombatSpeed = attributes->GetBaseAttribute(ClassAttribute::NonCombatSpeed);
					if (nonCombatSpeed > 0) {
						SetAttributeValue(AttributeType::NonCombatSpeed, nonCombatSpeed);
					}

					auto combatSpeed = attributes->GetBaseAttribute(ClassAttribute::NonCombatSpeed);
					if (combatSpeed > 0) {
						SetAttributeValue(AttributeType::CombatSpeed, combatSpeed);
					}

					mAttributes->SetWeaponDamage(1, 5);
				}
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
						SetAttributeValue(AttributeType::MaxHealth, health);
						SetHealth(GetMaxHealth());
					}

					auto mana = attributes->GetBaseAttribute(ClassAttribute::Mana);
					if (mana > 0) {
						SetAttributeValue(AttributeType::MaxMana, mana);
						SetMana(GetMaxMana());
					}

					auto nonCombatSpeed = attributes->GetBaseAttribute(ClassAttribute::NonCombatSpeed);
					if (nonCombatSpeed > 0) {
						// SetAttributeValue(AttributeType::NonCombatSpeed, nonCombatSpeed);
					}

					auto combatSpeed = attributes->GetBaseAttribute(ClassAttribute::NonCombatSpeed);
					if (combatSpeed > 0) {
						// SetAttributeValue(AttributeType::CombatSpeed, combatSpeed);
					}
				}
			}
		}
	}

	void Object::OnActivate() {
		if (mPassiveAbility) {
			mPassiveAbility->Activate(shared_from_this());
		}
	}

	void Object::OnDeactivate() {
		if (mPassiveAbility) {
			mPassiveAbility->Deactivate(shared_from_this());
		}
	}

	void Object::OnTick() {
		if (mTickOverride != sol::nil) {
			mTickOverride.call<void>(mPassiveAbility, shared_from_this());
		}
	}

	void Object::SetTickOverride(sol::protected_function func) {
		mTickOverride = func;
		mPassiveAbility = nullptr;
	}

	void Object::SetPassiveAbility(const AbilityPtr& ability) {
		mPassiveAbility = ability;
		if (mPassiveAbility) {
			mTickOverride = sol::nil;
		}
	}

	CombatantData& Object::GetCombatantData() {
		return mCombatantData;
	}

	const CombatantData& Object::GetCombatantData() const {
		return mCombatantData;
	}

	bool Object::HasInteractableData() const {
		return static_cast<bool>(mInteractableData);
	}

	const std::unique_ptr<InteractableData>& Object::CreateInteractableData() {
		if (!mInteractableData) {
			mInteractableData = std::make_unique<InteractableData>(*this);
			SetFlags(GetFlags() | Flags::UpdateInteractableData);
		}
		return GetInteractableData();
	}

	const std::unique_ptr<InteractableData>& Object::GetInteractableData() const {
		return mInteractableData;
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

	const std::unique_ptr<LootData>& Object::CreateLootData() {
		if (!mLootData) {
			mLootData = std::make_unique<LootData>(*this);
			SetFlags(GetFlags() | Flags::UpdateLootData);
		}
		return GetLootData();
	}

	const std::unique_ptr<LootData>& Object::GetLootData() const {
		return mLootData;
	}

	bool Object::HasLocomotionData() const {
		return static_cast<bool>(mLocomotionData);
	}

	const std::unique_ptr<Locomotion>& Object::CreateLocomotionData() {
		if (!mLocomotionData) {
			mLocomotionData = std::make_unique<Locomotion>(*this);
			SetFlags(GetFlags() | Flags::UpdateLocomotion);
		}
		return GetLocomotionData();
	}

	const std::unique_ptr<Locomotion>& Object::GetLocomotionData() const {
		return mLocomotionData;
	}

	bool Object::HasAgentBlackboardData() const {
		return static_cast<bool>(mAgentBlackboardData);
	}

	const std::unique_ptr<AgentBlackboard>& Object::CreateAgentBlackboardData() {
		if (!mAgentBlackboardData) {
			mAgentBlackboardData = std::make_unique<AgentBlackboard>(*this);
			SetFlags(GetFlags() | Flags::UpdateAgentBlackboardData);
		}
		return GetAgentBlackboardData();
	}

	const std::unique_ptr<AgentBlackboard>& Object::GetAgentBlackboardData() const {
		return mAgentBlackboardData;
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

	const glm::vec3& Object::GetExtent() const {
		return mBoundingBox.mExtent;
	}

	void Object::SetExtent(const glm::vec3& extent) {
		bool dirty = extent != mBoundingBox.mCenter;
		mBoundingBox.mExtent = glm::abs(extent);
		if (dirty) {
			SetDirty(true);
		}
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

	float Object::GetAttributeValue(AttributeType type) const {
		return GetAttributeValue(static_cast<uint8_t>(type));
	}

	void Object::SetAttributeValue(uint8_t idx, float value) {
		if (!mAttributes) {
			mAttributes = std::make_unique<Attributes>();
		}
		mAttributes->SetValue(idx, value);
		SetFlags(GetFlags() | Flags::UpdateAttributes);
	}

	void Object::SetAttributeValue(AttributeType type, float value) {
		SetAttributeValue(static_cast<uint8_t>(type), value);
	}

	float Object::GetHealth() const {
		return mCombatantData.mHitPoints;
	}

	float Object::GetMaxHealth() const {
		if (mAttributes) {
			return mAttributes->GetValue(AttributeType::MaxHealth);
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
			return mAttributes->GetValue(AttributeType::MaxMana);
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
	std::tuple<bool, float, bool> Object::TakeDamage(
		const AttributesPtr& attackerAttributes,
		const std::tuple<float, float>& damageRange,
		DamageType damageType, DamageSource damageSource, float damageCoefficient,
		int32_t descriptors, float damageMultiplier, const glm::vec3& direction
	) {
		// calculate damage stuff
		float damage = 0;
		bool critical = false;
		if (damage <= 0) {
			damage = utils::random::get<float>(std::get<0>(damageRange), std::get<1>(damageRange));

			if (attackerAttributes->GetValue(AttributeType::AutoCrit) > 0) {
				critical = true;
			} else {
				// calculate critical hit
				if (utils::random::get<uint32_t>(0, 1)) {
					critical = true;
				}
			}

			if (critical) {
				auto criticalIncrease = attackerAttributes->GetValue(AttributeType::CriticalDamageIncrease);
				damage *= criticalIncrease + 1;
			}
		}
		
		if (damage > 0) {
			SetHealth(GetHealth() - damage);
			if (GetHealth() <= 0) {
				mManager.GetGame().SendObjectGfxState(shared_from_this(), utils::hash_id("dead"));
			}
		}

		return { true, damage, critical };
	}

	std::tuple<float, bool> Object::Heal() {
		float healingReduction = GetAttributeValue(AttributeType::HealingReduction) - 1.f;
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
		return HasAgentBlackboardData() ? mAgentBlackboardData->GetTargetId() : 0;
	}

	void Object::SetTargetId(uint32_t id) {
		if (HasAgentBlackboardData()) {
			mAgentBlackboardData->SetTargetId(id);
		}
	}

	uint8_t Object::GetStealthType() const {
		return HasAgentBlackboardData() ? mAgentBlackboardData->GetStealthType() : 0;
	}

	void Object::SetStealthType(uint8_t stealthType) {
		if (HasAgentBlackboardData()) {
			mAgentBlackboardData->SetStealthType(stealthType);
		}
	}

	bool Object::IsInCombat() const {
		return HasAgentBlackboardData() ? mAgentBlackboardData->IsInCombat() : 0;
	}

	void Object::SetInCombat(bool inCombat) {
		if (HasAgentBlackboardData()) {
			mAgentBlackboardData->SetInCombat(inCombat);
		}
	}

	bool Object::IsTargetable() const {
		return HasAgentBlackboardData() ? mAgentBlackboardData->IsTargetable() : 0;
	}

	void Object::SetTargetable(bool targetable) {
		if (HasAgentBlackboardData()) {
			mAgentBlackboardData->SetTargetable(targetable);
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
