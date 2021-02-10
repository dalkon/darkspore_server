
#ifndef _GAME_OBJECT_HEADER
#define _GAME_OBJECT_HEADER

// Include
#include "noun.h"
#include "lua.h"
#include <map>

// Game
namespace Game {
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
			SourceMarker,
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

				UpdateFlags = UpdateCombatant | UpdateAttributes | UpdateLootData | UpdateAgentBlackboardData
			};

		protected:
			Object(ObjectManager& manager, uint32_t id, uint32_t noun);

		public:
			virtual ~Object() = default;

			virtual bool IsTrigger() const { return false; }

			virtual void OnActivate();
			virtual void OnDeactivate();
			virtual void OnTick();

			void SetTickOverride(sol::function func);

			RakNet::cCombatantData& GetCombatantData();
			const RakNet::cCombatantData& GetCombatantData() const;

			bool HasAttributeData() const;
			const RakNet::AttributeData& GetAttributeData() const;

			bool HasLootData() const;
			const RakNet::cLootData& GetLootData() const;
			void SetLootData(RakNet::cLootData&& lootData);

			bool HasLocomotionData() const;
			const RakNet::LocomotionData& GetLocomotionData() const;

			bool HasAgentBlackboardData() const;
			const RakNet::cAgentBlackboard& GetAgentBlackboardData() const;

			uint64_t GetAssetId() const;

			uint32_t GetId() const;
			uint32_t GetNoun() const;

			NounType GetType() const;

			const glm::vec3& GetPosition() const;
			void SetPosition(const glm::vec3& position);

			const glm::quat& GetOrientation() const;
			void SetOrientation(const glm::quat& orientation);

			const BoundingBox& GetBoundingBox() const;

			// Effects
			uint8_t AddEffect(uint32_t effect);
			bool RemoveEffect(uint32_t effect);
			bool RemoveEffectByIndex(uint8_t index);

			// Properties
			float GetAttributeValue(uint8_t idx) const;
			void SetAttributeValue(uint8_t idx, float value);

			float GetHealth() const;
			float GetMaxHealth() const;
			void SetHealth(float newHealth);

			float GetMana() const;
			float GetMaxMana() const;
			void SetMana(float newMana);

			uint32_t GetInteractableState() const;
			void SetInteractableState(uint32_t interactableState);

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

			sol::function mTickOverride;

			BoundingBox mBoundingBox;

			RakNet::cCombatantData mCombatantData;

			std::unique_ptr<EffectList> mEffects;
			std::unique_ptr<RakNet::AttributeData> mAttributes;
			std::unique_ptr<RakNet::cLootData> mLootData;
			std::unique_ptr<RakNet::LocomotionData> mLocomotionData;
			std::unique_ptr<RakNet::cAgentBlackboard> mAgentBlackboardData;

			NounPtr mNoun;

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
			uint32_t sourceMarkerKey_markerId = 0;

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
	};

	using ObjectPtr = std::shared_ptr<Object>;
}

#endif
