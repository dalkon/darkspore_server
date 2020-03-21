
#ifndef _GAME_USER_HEADER
#define _GAME_USER_HEADER

// Include
#include "game.h"
#include "squad.h"
#include "room.h"

#include <cstdint>
#include <string>
#include <map>
#include <pugixml.hpp>

// Game
namespace Game {
	// Account
	struct Account {
		bool tutorialCompleted = false;
		bool grantAllAccess = false;
		bool grantOnlineAccess = false;

		uint32_t chainProgression = 0;
		uint32_t creatureRewards = 0;

		uint32_t currentGameId = 1;
		uint32_t currentPlaygroupId = 1;

		uint32_t defaultDeckPveId = 1;
		uint32_t defaultDeckPvpId = 1;

		uint32_t level = 1;
		uint32_t xp = 0;
		uint32_t dna = 0;
		uint32_t avatarId = 0;
		uint32_t id = 1;

		uint32_t newPlayerInventory = 0;
		uint32_t newPlayerProgress = 0;

		uint32_t cashoutBonusTime = 0;
		uint32_t starLevel = 0;

		uint32_t unlockCatalysts = 0;
		uint32_t unlockDiagonalCatalysts = 0;
		uint32_t unlockInventory = 0;
		uint32_t unlockFuelTanks = 0;
		uint32_t unlockPveDecks = 0;
		uint32_t unlockPvpDecks = 0;
		uint32_t unlockStats = 0;
		uint32_t unlockInventoryIdentify = 0;
		uint32_t unlockEditorFlairSlots = 0;

		uint32_t upsell = 0;

		uint32_t capLevel = 0;
		uint32_t capProgression = 0;

		void Read(const pugi::xml_node& node);
		void Write(pugi::xml_node& node) const;
	};

	// FeedItem
	struct FeedItem {
		std::string metadata;
		std::string name;

		uint64_t timestamp;

		uint32_t accountId;
		uint32_t id;
		uint32_t messageId;
	};

	// Feed
	class Feed {
		public:
			decltype(auto) begin() { return mItems.begin(); }
			decltype(auto) begin() const { return mItems.begin(); }
			decltype(auto) end() { return mItems.end(); }
			decltype(auto) end() const { return mItems.end(); }

			auto& data() { return mItems; }
			const auto& data() const { return mItems; }

			void Read(const pugi::xml_node& node);
			void Write(pugi::xml_node& node) const;

			void Add(FeedItem&& item);

		private:
			std::vector<FeedItem> mItems;
	};

	// Part
	class Part {
		public:
			Part(uint32_t rigblock);
			Part(const pugi::xml_node& node);

			bool Read(const pugi::xml_node& node);
			void Write(pugi::xml_node& node, uint32_t index, bool api = false) const;

			void SetRigblock(uint16_t rigblock);
			void SetPrefix(uint16_t prefix, bool secondary = false);
			void SetSuffix(uint16_t suffix);

			void SetStatus(uint8_t newStatus);

		private:
			uint64_t timestamp;

			uint32_t rigblock_asset_hash;
			uint32_t prefix_asset_hash;
			uint32_t prefix_secondary_asset_hash;
			uint32_t suffix_asset_hash;
			uint32_t cost;
			uint32_t equipped_to_creature_id;

			uint16_t rigblock_asset_id;
			uint16_t prefix_asset_id;
			uint16_t prefix_secondary_asset_id;
			uint16_t suffix_asset_id;
			uint16_t level;

			uint8_t rarity;
			uint8_t market_status;
			uint8_t status;
			uint8_t usage;

			bool flair;

			friend class Parts;
	};

	// Parts
	class Parts {
		public:
			decltype(auto) begin() { return mItems.begin(); }
			decltype(auto) begin() const { return mItems.begin(); }
			decltype(auto) end() { return mItems.end(); }
			decltype(auto) end() const { return mItems.end(); }

			auto& data() { return mItems; }
			const auto& data() const { return mItems; }

			void Read(const pugi::xml_node& node);
			void Write(pugi::xml_node& node, bool api = false) const;

		private:
			std::vector<Part> mItems;
	};

	// User
	class User {
		public:
			User(const std::string& username);
			~User();

			//
			auto& get_account() { return mAccount; }
			const auto& get_account() const { return mAccount; }

			auto& get_creatures() { return mCreatures; }
			const auto& get_creatures() const { return mCreatures; }

			auto& get_squads() { return mSquads; }
			const auto& get_squads() const { return mSquads; }

			auto& get_feed() { return mFeed; }
			const auto& get_feed() const { return mFeed; }

			auto& get_parts() { return mParts; }
			const auto& get_parts() const { return mParts; }

			const std::string& get_auth_token() const { return mAuthToken; }
			void set_auth_token(const std::string& authToken) { mAuthToken = authToken; }

			const Game::Ptr& get_game() const { return mGame; }
			void set_game(const Game::Ptr& game) { mGame = game; }

			auto get_id() const { return mAccount.id; }

			const auto& get_username() const { return mUsername; }
			const auto& get_password() const { return mPassword; }
			const auto& get_name() const { return mName; }

			bool UpdateState(uint32_t newState);

			// Creature
			Creature::Ptr GetCreatureById(uint32_t id) const;

			void UnlockCreature(uint32_t templateId);

			// Squads
			void UpdateSquad(uint32_t slot, const std::string& creatureStringList, bool pvp);

			// Upgrades
			bool UnlockUpgrade(uint32_t unlockId);

			// Auth
			void Logout();

			// Rooms
			const RoomPtr& GetRoom() const;
			void SetRoom(const RoomPtr& room);

			// Storage
			bool Load();
			bool Save();

		private:
			Account mAccount;

			Creatures mCreatures;
			Squads mSquads;
			Feed mFeed;
			Parts mParts;

			std::string mUsername;
			std::string mPassword;
			std::string mName;
			std::string mAuthToken;

			Game::Ptr mGame;
			RoomPtr mRoom;

			uint32_t mId = 0;
			uint32_t mState = 0;
	};

	using UserPtr = std::shared_ptr<User>;

	// UserManager | TODO: move to Application?
	class UserManager {
		public:
			static UserPtr GetUserById(int64_t id);
			static UserPtr GetUserByEmail(const std::string& username);
			static UserPtr GetUserByAuthToken(const std::string& authToken);

		private:
			static void RemoveUser(const std::string& username);

		private:
			static std::map<std::string, UserPtr> sUsersByEmail;

			friend class User;
	};
}

#endif
