
#ifndef _GAME_OBJECT_HEADER
#define _GAME_OBJECT_HEADER

// Include
#include "raknet/types.h"
#include <memory>

// Game
namespace Game {
	class ObjectManager;

	// Object
	class Object {
		private:
			Object(ObjectManager& manager, uint32_t id, uint32_t noun);

		public:
			~Object();

			RakNet::sporelabsObject& GetData();
			const RakNet::sporelabsObject& GetData() const;

			bool HasAttributeData() const;
			const RakNet::AttributeData& GetAttributeData() const;

			bool HasLootData() const;
			const RakNet::cLootData& GetLootData() const;
			void SetLootData(RakNet::cLootData&& lootData);

			bool HasLocomotionData() const;
			const RakNet::cLocomotionData& GetLocomotionData() const;

			uint32_t GetId() const;
			uint32_t GetNoun() const;

			//
			auto GetAssetId() const { return mAssetId; }
			void SetAssetId(uint64_t id) { mAssetId = id; }
			//

			const RakNet::cSPVector3& GetPosition() const;
			void SetPosition(const RakNet::cSPVector3& position);
			void SetPosition(RakNet::cSPVector3&& position);

			const RakNet::cSPQuaternion& GetOrientation() const;
			void SetOrientation(const RakNet::cSPQuaternion& orientation);
			void SetOrientation(RakNet::cSPQuaternion&& orientation);

			// Properties
			float GetValue(uint8_t idx) const;
			void SetValue(uint8_t idx, float value);

			float GetHealth() const;
			float GetMaxHealth() const;
			void SetHealth(float newHealth);

			float GetMana() const;
			float GetMaxMana() const;
			void SetMana(float newMana);

			// Combatant functions
			void OnChangeHealth(float healthChange);
			void OnChangeMana(float manaChange);

		protected:
			ObjectManager& mManager;

			RakNet::sporelabsObject mData;
			RakNet::cCombatantData mCombatantData;

			std::unique_ptr<RakNet::AttributeData> mAttributes;
			std::unique_ptr<RakNet::cLootData> mLootData;
			std::unique_ptr<RakNet::cLocomotionData> mLocomotionData;

			// std::unique_ptr<RakNet::cAgentBlackboard> mAgentBlackboardData;

			// TEMPORARY, remove after object files are done
			uint64_t mAssetId = 0;
			//

			uint32_t mUpdateFlags = 0;
			uint32_t mId;
			uint32_t mNoun;

			friend class ObjectManager;
	};

	using ObjectPtr = std::shared_ptr<Object>;
}

#endif
