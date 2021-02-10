
#ifndef _GAME_LEVEL_HEADER
#define _GAME_LEVEL_HEADER

// Include
#include "noun.h"

// Game
namespace Game {
	// Marker
	class Marker {
		public:
			void Read(pugi::xml_node node);

		private:
			glm::vec3 mPosition;
			glm::vec3 mRotation;

			std::string mName;
			std::string mNounName;

			uint64_t mOverrideAssetId;

			uint32_t mId;
			uint32_t mTargetId;

			float mScale;

			bool mVisible;
			bool mHasCollision;
	};

	// Markerset
	class Markerset {
		public:
			void Read(pugi::xml_node node);

		private:
			std::vector<Marker> mMarkers;
	};

	// DirectorClass
	class DirectorClass {
		public:
			void Read(pugi::xml_node node);

		private:
			std::string mNounName;

			int32_t mMinLevel;
			int32_t mMaxLevel;

			bool mHordeLegal;
	};

	// LevelConfig
	class LevelConfig {
		public:
			void Read(pugi::xml_node node);

		private:
			std::vector<DirectorClass> mMinions;
			std::vector<DirectorClass> mSpecial;
			std::vector<DirectorClass> mBoss;
			std::vector<DirectorClass> mAgent;
			std::vector<DirectorClass> mCaptain;
	};

	// Level
	class Level {
		public:
			Level();
			~Level();

			bool Load(const std::string& path);

			void Reload();

		private:
			std::vector<std::string> mMarkersets;

			LevelConfig mConfig;
			LevelConfig mFirstTimeConfig;
			LevelConfig mPlanetConfig;
	};
}

#endif
