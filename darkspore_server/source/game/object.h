
#ifndef _GAME_OBJECT_HEADER
#define _GAME_OBJECT_HEADER

// Include
#include "noun.h"
#include "attributes.h"
#include "locomotion.h"
#include "lua.h"
#include <map>

// Predefined
namespace SporeNet { class Part; }
namespace RakNet { struct labsCrystal; }

// Game
namespace Game {
	class Object;
	class ObjectManager;
	
	// EffectList
	class EffectList {
		public:
			uint8_t Add(uint32_t effect);
			bool Remove(uint32_t effect);
			bool RemoveByIndex(uint8_t index);

		private:
			std::map<uint32_t, uint8_t> mIndexByEffect;

			std::vector<uint8_t> mOpenIndexes;
			std::vector<uint8_t> mUsedIndexes;
	};

	// CombatantData
	class CombatantData {
		public:
			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

		private:
			float mHitPoints = 0;
			float mManaPoints = 0;

			friend class Object;
	};

	// InteractableData
	class InteractableData {
		public:
			InteractableData(Object& object);

			uint32_t GetAbility() const;
			void SetAbility(uint32_t ability);

			int32_t GetTimesUsed() const;
			void SetTimesUsed(int32_t timesUsed);

			int32_t GetUsesAllowed() const;
			void SetUsesAllowed(int32_t usesAllowed);

			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

		private:
			Object& mObject;

			uint32_t mAbility = 0;

			int32_t mTimesUsed = 0;
			int32_t mUsesAllowed = 0;

			friend class Object;
	};

	// LootData
	class LootData {
		public:
			LootData(Object& object);

			uint32_t GetRigblockAsset() const;
			uint32_t GetPrefixAsset() const;
			uint32_t GetPrefixSecondaryAsset() const;
			uint32_t GetSuffixAsset() const;

			int32_t GetLevel() const;
			int32_t GetRarity() const;

			void SetId(uint64_t id);
			void SetInstanceId(uint64_t id);

			void SetPart(const SporeNet::Part& part);
			void SetDNAAmount(float amount);
			void SetCrystal(const RakNet::labsCrystal& crystal);

			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

		private:
			Object& mObject;

			uint64_t mId = 0;
			uint64_t mInstanceId = 0;

			uint32_t mRigblockAsset = 0;
			uint32_t mSuffixAssetId = 0;
			uint32_t mPrefixAssetId = 0;
			uint32_t mSecondaryPrefixAssetId = 0;

			int32_t mItemLevel = 0;
			int32_t mCrystalLevel = 0;
			int32_t mRarity = 0;

			float mDNAAmount = 0;

			friend class Object;
	};

	// AgentBlackboard
	class AgentBlackboard {
		public:
			AgentBlackboard(Object& object);

			uint32_t GetTargetId() const;
			void SetTargetId(uint32_t id);

			uint32_t GetNumAttackers() const;
			void SetNumAttackers(uint32_t attackers);

			uint8_t GetStealthType() const;
			void SetStealthType(uint8_t stealthType);

			bool IsInCombat() const;
			void SetInCombat(bool inCombat);

			bool IsTargetable() const;
			void SetTargetable(bool targetable);

			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

		private:
			Object& mObject;

			uint32_t mTargetId = 0;
			uint32_t mNumAttackers = 0;

			uint8_t mStealthType = 0;

			bool mInCombat = false;
			bool mTargetable = false;
	};

	// ObjectDataBits
	namespace ObjectDataBits {
		enum : uint8_t {
			Team = 0,
			PlayerControlled,
			InputSyncStamp,
			PlayerIndex,
			LinearVelocity,
			AngularVelocity,
			Position,
			Orientation,
			Scale,
			MarkerScale,
			LastAnimationState,
			LastAnimationPlayTime,
			OverrideMoveIdleAnimationState,
			GraphicsState,
			GraphicsStateStartTime,
			NewGraphicsStateStartTime,
			Visible,
			HasCollision,
			OwnerId,
			MovementType,
			DisableRepulsion,
			InteractableState,
			MarkerId,
			Count
		};
	}

	// Object
	class Object : public std::enable_shared_from_this<Object> {
		public:
			enum Flags : uint8_t {
				None							= 0,
				MarkedForDeletion				= 1 << 0,
				Dirty							= 1 << 1,
				UpdateCombatant					= 1 << 2,
				UpdateAttributes				= 1 << 3,
				UpdateLootData					= 1 << 4,
				UpdateAgentBlackboardData		= 1 << 5,
				UpdateInteractableData			= 1 << 6,
				UpdateLocomotion				= 1 << 7,

				UpdateFlags = UpdateCombatant | UpdateAttributes | UpdateLootData | UpdateAgentBlackboardData | UpdateInteractableData | UpdateLocomotion
			};

		protected:
			Object(ObjectManager& manager, uint32_t id, uint32_t noun);

		public:
			virtual ~Object() = default;

			virtual bool IsTrigger() const { return false; }

			virtual void OnActivate();
			virtual void OnDeactivate();
			virtual void OnTick();

			void SetTickOverride(sol::protected_function func);
			void SetPassiveAbility(const AbilityPtr& ability);

			CombatantData& GetCombatantData();
			const CombatantData& GetCombatantData() const;

			bool HasInteractableData() const;
			const std::unique_ptr<InteractableData>& CreateInteractableData();
			const std::unique_ptr<InteractableData>& GetInteractableData() const;

			bool HasAttributeData() const;
			const std::unique_ptr<Attributes>& GetAttributeData() const;

			bool HasLootData() const;
			const std::unique_ptr<LootData>& CreateLootData();
			const std::unique_ptr<LootData>& GetLootData() const;

			bool HasLocomotionData() const;
			const std::unique_ptr<Locomotion>& CreateLocomotionData();
			const std::unique_ptr<Locomotion>& GetLocomotionData() const;

			bool HasAgentBlackboardData() const;
			const std::unique_ptr<AgentBlackboard>& CreateAgentBlackboardData();
			const std::unique_ptr<AgentBlackboard>& GetAgentBlackboardData() const;

			uint64_t GetAssetId() const;
			void SetAssetId(uint64_t assetId);

			const NounPtr& GetNoun() const;

			uint32_t GetId() const;
			uint32_t GetNounId() const;

			NounType GetType() const;

			const glm::vec3& GetPosition() const;
			void SetPosition(const glm::vec3& position);

			const glm::quat& GetOrientation() const;
			void SetOrientation(const glm::quat& orientation);

			const BoundingBox& GetBoundingBox() const;

			const glm::vec3& GetExtent() const;
			void SetExtent(const glm::vec3& extent);

			float GetFootprintRadius() const;

			// Effects
			uint8_t AddEffect(uint32_t effect);
			bool RemoveEffect(uint32_t effect);
			bool RemoveEffectByIndex(uint8_t index);

			// Properties
			float GetAttributeValue(uint8_t idx) const;
			float GetAttributeValue(AttributeType type) const;
			void SetAttributeValue(uint8_t idx, float value);
			void SetAttributeValue(AttributeType type, float value);

			float GetHealth() const;
			float GetMaxHealth() const;
			void SetHealth(float newHealth);

			float GetMana() const;
			float GetMaxMana() const;
			void SetMana(float newMana);

			uint32_t GetInteractableState() const;
			void SetInteractableState(uint32_t interactableState);

			uint32_t GetMarkerId() const;
			void SetMarkerId(uint32_t markerId);

			float GetScale() const;
			void SetScale(float scale);

			uint8_t GetTeam() const;
			void SetTeam(uint8_t team);

			uint8_t GetMovementType() const;
			void SetMovementType(uint8_t movementType);

			uint8_t GetPlayerIndex() const;
			void SetPlayerIndex(uint8_t playerIndex);

			bool IsPlayerControlled() const;

			bool HasCollision() const;
			void SetHasCollision(bool collision);

			bool IsVisible() const;
			void SetVisible(bool visible);

			bool IsDirty() const;
			void SetDirty(bool dirty);

			bool IsMarkedForDeletion() const;
			void MarkForDeletion();

			// Combatant functions
			std::tuple<bool, float, bool> TakeDamage(
				const AttributesPtr& attackerAttributes,
				const std::tuple<float, float>& damageRange,
				DamageType damageType, DamageSource damageSource, float damageCoefficient,
				int32_t descriptors, float damageMultiplier, const glm::vec3& direction
			);
			std::tuple<float, bool> Heal();

			void OnChangeHealth(float healthChange);
			void OnChangeMana(float manaChange);

			// Agent Blackboard
			uint32_t GetTargetId() const;
			void SetTargetId(uint32_t id);

			uint8_t GetStealthType() const;
			void SetStealthType(uint8_t stealthType);

			bool IsInCombat() const;
			void SetInCombat(bool inCombat);

			bool IsTargetable() const;
			void SetTargetable(bool targetable);

			// Network & reflection
			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

			void ResetUpdateBits();

		protected:
			bool NeedUpdate() const;

			uint8_t GetFlags() const;
			void SetFlags(uint8_t flags);

		protected:
			ObjectManager& mManager;

			sol::protected_function mTickOverride = sol::nil;
			AbilityPtr mPassiveAbility;

			BoundingBox mBoundingBox;

			CombatantData mCombatantData;

			std::unique_ptr<EffectList> mEffects;
			std::unique_ptr<Attributes> mAttributes;
			std::unique_ptr<InteractableData> mInteractableData;
			std::unique_ptr<LootData> mLootData;
			std::unique_ptr<Locomotion> mLocomotionData;
			std::unique_ptr<AgentBlackboard> mAgentBlackboardData;

			NounPtr mNoun;

			uint64_t mAssetId;

			uint32_t mId;
			uint32_t mNounId;

			Flags mFlags = Flags::None;

			// mostly reflection for now
			glm::quat mOrientation {};
			glm::vec3 mLinearVelocity {};
			glm::vec3 mAngularVelocity {};

			uint64_t mLastAnimationPlayTime = 0;
			uint64_t mGraphicsStateStartTime = 0;
			uint64_t mNewGraphicsStateStartTime = 0;

			uint32_t mOwnerId = 0;
			uint32_t mInputSyncStamp = 0;
			uint32_t mLastAnimationState = 0;
			uint32_t mOverrideMoveIdleAnimationState = 0;
			uint32_t mGraphicsState = 0;
			uint32_t mInteractableState = 0;
			uint32_t mMarkerId = 0;

			float mScale = 1;
			float mMarkerScale = 1;

			uint8_t mTeam = 0;
			uint8_t mMovementType = 0;
			uint8_t mPlayerIndex = 0xFF;

			bool mbPlayerControlled = false;
			bool mVisible = true;
			bool mbHasCollision = true;
			bool mDisableRepulsion = false;

			std::bitset<ObjectDataBits::Count> mDataBits;

			friend class Instance;
			friend class ObjectManager;
			friend class InteractableData;
			friend class LootData;
			friend class AgentBlackboard;
			friend class Locomotion;
	};

	using ObjectPtr = std::shared_ptr<Object>;
}

#endif
