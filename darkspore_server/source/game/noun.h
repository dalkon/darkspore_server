
#ifndef _GAME_NOUN_HEADER
#define _GAME_NOUN_HEADER

// Include
#include "raknet/types.h"
#include <pugixml.hpp>
#include <memory>
#include <unordered_map>

// Game
namespace Game {
	// BoundingBox
	class BoundingBox {
		public:
			BoundingBox();
			BoundingBox(const glm::vec3& min, const glm::vec3& max);

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

	// Noun
	class Noun {
		public:
			void Read(pugi::xml_node node);

			const BoundingBox& GetBoundingBox() const;

			const std::string& GetName() const;

			uint64_t GetAssetId() const;
			uint32_t GetId() const;

			NounType GetType() const;

			bool IsCreature() const;
			bool IsPlayer() const;

		private:
			BoundingBox mBoundingBox;

			std::vector<uint64_t> mEliteAssetIds;

			std::string mNpcClassData;
			std::string mPlayerClassData;
			std::string mCharacterAnimationData;
			std::string mName;

			uint64_t mAssetId = 0;
			uint32_t mId = 0;

			NounType mType = NounType::None;

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

			void Load();

			NounPtr Get(uint32_t noun) const;

		private:
			std::unordered_map<uint32_t, NounPtr> mNouns;

			bool mLoaded = false;
	};
}

#endif
