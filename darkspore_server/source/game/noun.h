
#ifndef _GAME_NOUN_HEADER
#define _GAME_NOUN_HEADER

// Include
#include "raknet/types.h"
#include <pugixml.hpp>
#include <array>
#include <memory>
#include <unordered_map>

// Game
namespace Game {
	// BoundingBox
	class BoundingBox {
		public:
			BoundingBox();
			BoundingBox(const glm::vec3& min, const glm::vec3& max);

			bool IsPoint() const;
			bool Contains(const glm::vec3& point) const;
			bool Contains(const BoundingBox& boundingBox) const;
			bool Intersects(const BoundingBox& boundingBox) const;
			bool Touches(const BoundingBox& boundingBox) const;

			void Transform(const glm::mat4& matrix);

			glm::vec3 GetMin() const;
			glm::vec3 GetMax() const;

			glm::vec3 GetSize() const;

		public:
			glm::vec3 mCenter;
			glm::vec3 mExtent;
	};

	// ClassAttribute
	namespace ClassAttribute {
		enum Key {
			Health = 0,
			Mana,
			Strength,
			Dexterity,
			Mind,
			PhysicalDefense,
			MagicalDefense,
			EnergyDefense,
			Critical,
			CombatSpeed,
			NonCombatSpeed,
			StealthDetection,
			MovementSpeedBuff,

			BaseCount = 13,
			MaxCount = 9
		};
	}

	// ClassAttributes
	class ClassAttributes {
		public:
			void Read(pugi::xml_node node);

			float GetBaseAttribute(ClassAttribute::Key key) const;
			float GetMaxAttribute(ClassAttribute::Key key) const;

		private:
			std::array<float, ClassAttribute::BaseCount> mBaseAttributes { 0 };
			std::array<float, ClassAttribute::MaxCount> mMaxAttributes { 0 };

			std::string mPath;

			uint32_t mId = 0;

			friend class NounDatabase;
	};

	// NpcType
	enum class NpcType : uint32_t {
		Minion = 0,
		Special,
		Boss,
		Destructible,
		Interactable,
		Agent,
		Victim,
		Captain
	};

	// DropType
	enum class DropType : uint32_t {
		None = 1 << 0,
		Orbs = 1 << 1,
		Catalysts = 1 << 2,
		Loot = 1 << 3,
		DNA = 1 << 4
	};

	// Homeworld
	enum class Homeworld : uint32_t {
		ZelemsNexus = 0,
		Nocturna,
		Verdanth,
		Cryos,
		Infinity
	};

	// NpcAffix
	class NpcAffix {
		public:
			void Read(pugi::xml_node node);

			const std::shared_ptr<NpcAffix>& GetChild() const;
			const std::shared_ptr<NpcAffix>& GetParent() const;

		private:
			std::shared_ptr<NpcAffix> mChild;
			std::shared_ptr<NpcAffix> mParent;

			std::string mChildName;
			std::string mParentName;
			std::string mModifier;
			std::string mPath;

			uint32_t mId = 0;

			friend class NounDatabase;
	};

	// EliteAffix
	class EliteAffix {
		public:
			void Read(pugi::xml_node node);

		private:
			std::shared_ptr<NpcAffix> mNpcAffix;

			int32_t mMinDifficulty = 0;
			int32_t mMaxDifficulty = 0;
	};

	// PlayerClass
	class PlayerClass {
		public:
			void Read(pugi::xml_node node);

			const std::shared_ptr<ClassAttributes>& GetAttributes() const;

		private:
			std::shared_ptr<ClassAttributes> mAttributes;

			std::array<std::string, 5> mAbilities;

			std::string mName;
			std::string mEffect;
			std::string mPath;

			Homeworld mHomeworld = Homeworld::ZelemsNexus;

			uint32_t mCreatureType = 0;
			uint32_t mCreatureClass = 0;
			uint32_t mPrimaryAttribute = 0; // str, dex, mind
			uint32_t mId = 0;

			int32_t mUnlockLevel = 0;

			float mWeaponMinDamage = 0;
			float mWeaponMaxDamage = 0;

			bool mNoHands = false;
			bool mNoFeet = false;

			friend class NounDatabase;
	};

	// NonPlayerClass
	class NonPlayerClass {
		public:
			void Read(pugi::xml_node node);
			
			const std::shared_ptr<ClassAttributes>& GetAttributes() const;

			bool IsTargetable() const;
			bool IsPet() const;

		private:
			std::shared_ptr<ClassAttributes> mAttributes;

			std::vector<EliteAffix> mEliteAffixes;
			std::vector<DropType> mDropTypes;

			std::string mName;
			std::string mEffect;
			std::string mPath;

			NpcType mNpcType = NpcType::Minion;

			uint32_t mCreatureType = 0;
			uint32_t mId = 0;

			int32_t mChallengeValue = 0;
			int32_t mNpcRank = 0;

			float mAggroRange = 0;
			float mAlertRange = 0;
			float mDropAggroRange = 0;
			float mDropDelay = 0;
			float mPlayerCountHealthScale = 0;

			bool mTargetable = false;
			bool mIsPet = false;

			friend class NounDatabase;
	};

	// AINode
	class AINode {
		public:
			void Read(pugi::xml_node node);

		private:
			std::vector<int32_t> mOutput;

			std::string mPhaseData;
			std::string mConditionData;

			int32_t mX;
			int32_t mY;
	};

	// AIDefinition
	class AIDefinition {
		public:
			void Read(pugi::xml_node node);

			const std::string& GetPassiveAbility() const;

		private:
			std::vector<AINode> mNodes;

			std::string mPath;
			std::string mDeathAbility;
			std::string mDeathCondition;
			std::string mFirstAggroAbility;
			std::string mSecondaryFirstAggroAbility;
			std::string mFirstAlertAbility;
			std::string mSubsequentAggroAbility;
			std::string mPassiveAbility;
			std::string mCombatIdle;
			std::string mSecondaryCombatIdle;
			std::string mSecondaryCombatIdleCondition;
			std::string mPassiveIdle;
			std::string mPreAggroIdle;
			std::string mSecondaryPreAggroIdle;
			std::string mTargetTooFar;

			uint32_t mId = 0;
			uint32_t mAggroType = 0;
			uint32_t mCombatIdleCooldown = 0;
			uint32_t mSecondaryCombatIdleCooldown = 0;
			uint32_t mTargetTooFarCooldown = 0;

			float mUseSecondaryStart = 0;

			bool mFaceTarget = false;
			bool mAlwaysRun = false;
			bool mRandomizeCooldowns = false;

			friend class NounDatabase;
	};

	// NounType
	enum class NounType : uint32_t {
		None					= 0x00000000,
		Creature				= 0x06C27D00,
		Vehicle					= 0x78BDDF27,
		Obstacle				= 0x02ADB47A,
		SpawnPoint				= 0xD9EAF104,
		PathPoint				= 0x9D99F2FA,
		Trigger					= 0x3CE46113,
		PointLight				= 0x93555DCB,
		SpotLight				= 0xBCD9673B,
		LineLight				= 0x3D58111D,
		ParallelLight			= 0x8B1018A4,
		HemisphereLight			= 0xE615AFDB,
		Animator				= 0xF90527D6,
		Animated				= 0xF3051E56,
		GraphicsControl			= 0xE9A24895,
		Material				= 0xE6640542,
		Flora					= 0x5BDCEC35,
		LevelshopObject			= 0xA12B2C18,
		Terrain					= 0x151DB008,
		Weapon					= 0xE810D505,
		Building				= 0xC710B6E9,
		Handle					= 0xADB0A86B,
		HealthOrb				= 0x75B43FF2,
		ManaOrb					= 0xF402465F,
		ResurrectOrb			= 0xC035AAAD,
		Movie					= 0x4927FA7F,
		Loot					= 0x292FEA33,
		PlacableEffect			= 0x383A0A75,
		LuaJob					= 0xA2908A12,
		AbilityObject			= 0x485FC991,
		LevelExitPoint			= 0x087E8047,
		Decal					= 0x4D7784B8,
		Water					= 0x9E3C3DFA,
		Grass					= 0xFD3D2ED9,
		Door					= 0x6FEDAE4D,
		Crystal					= 0xCD482419,
		Interactable			= 0x0977AF61,
		Projectile				= 0x253F6F5C,
		DestructibleOrnament	= 0x0013FBB4,
		MapCamera				= 0xFBFB36D0,
		Occluder				= 0x071FD3D4,
		SplineCamera			= 0x6CB99FFF,
		SplineCameraNode		= 0xFD487097,
		BossPortal				= 0xC1B461BC
	};

	// PresetExtents
	enum class PresetExtents : uint32_t {
		None = 0,
		CritterVertical,
		CritterHorizontal,
		MinionVertical,
		MinionHorizontal,
		EliteMinionVertical,
		EliteMinionHorizontal,
		PlayerTempest,
		PlayerRavager,
		PlayerSentinel,
		LieutenantVertical,
		LieutenantHorizontal,
		EliteLieutenantVertical,
		EliteLieutenantHorizontal,
		CaptainVertical,
		CaptainHorizontal,
		BossVertical,
		BossHorizontal
	};

	// Noun
	class Noun {
		public:
			void Read(pugi::xml_node node);

			const BoundingBox& GetBoundingBox() const;

			const std::shared_ptr<NonPlayerClass>& GetNonPlayerClassData() const;
			const std::shared_ptr<PlayerClass>& GetPlayerClassData() const;
			const std::shared_ptr<AIDefinition>& GetAIDefinition() const;

			const std::string& GetName() const;

			uint64_t GetAssetId() const;
			uint32_t GetId() const;

			NounType GetType() const;
			PresetExtents GetPresetExtents() const;

			bool IsCreature() const;
			bool IsPlayer() const;

		private:
			BoundingBox mBoundingBox;

			std::shared_ptr<NonPlayerClass> mNpcClassData;
			std::shared_ptr<PlayerClass> mPlayerClassData;
			std::shared_ptr<AIDefinition> mAIDefinition;

			std::vector<uint64_t> mEliteAssetIds;

			std::string mCharacterAnimationData;
			std::string mName;

			uint64_t mAssetId = 0;
			uint32_t mId = 0;

			NounType mType = NounType::None;
			PresetExtents mPresetExtents = PresetExtents::None;

			bool mIsFlora = false;
			bool mIsMineral = false;
			bool mIsCreature = false;
			bool mIsPlayer = false;
			bool mIsSpawned = false;

			friend class NounDatabase;
	};

	using NounPtr = std::shared_ptr<Noun>;

	// NounDatabase
	class NounDatabase {
		public:
			static NounDatabase& Instance();

			bool Load();

			NounPtr Get(uint32_t id) const;
			std::shared_ptr<NonPlayerClass> GetNonPlayerClass(uint32_t id) const;
			std::shared_ptr<PlayerClass> GetPlayerClass(uint32_t id) const;
			std::shared_ptr<NpcAffix> GetNpcAffix(uint32_t id) const;
			std::shared_ptr<ClassAttributes> GetClassAttributes(uint32_t id) const;
			std::shared_ptr<AIDefinition> GetAIDefinition(uint32_t id) const;

		private:
			bool LoadNouns();
			bool LoadNonPlayerClasses();
			bool LoadPlayerClasses();
			bool LoadNpcAffixes();
			bool LoadClassAttributes();
			bool LoadAIDefinitions();

		private:
			std::unordered_map<uint32_t, NounPtr> mNouns;
			std::unordered_map<uint32_t, std::shared_ptr<NonPlayerClass>> mNonPlayerClasses;
			std::unordered_map<uint32_t, std::shared_ptr<PlayerClass>> mPlayerClasses;
			std::unordered_map<uint32_t, std::shared_ptr<NpcAffix>> mNpcAffixes;
			std::unordered_map<uint32_t, std::shared_ptr<ClassAttributes>> mClassAttributes;
			std::unordered_map<uint32_t, std::shared_ptr<AIDefinition>> mAIDefinitions;

			bool mLoaded = false;
	};
}

#endif
