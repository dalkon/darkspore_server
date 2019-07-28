
// Include
#include "user.h"
#include "../utils/functions.h"
#include <algorithm>

/*
<feed>
	<count>20</count>
	<total>161</total>
	<items>
		<item>
		<account_id>386571374</account_id>
		<id>1803</id>
		<message_id>2</message_id>
		<metadata>3524487169;1002340</metadata>
		<name>maxisjohn</name>
		<time>1296011517</time>
		</item>
		<item>
		<account_id>386571374</account_id>
		<id>1802</id>
		<message_id>2</message_id>
		<metadata>2128242117;1002339</metadata>
		<name>maxisjohn</name>
		<time>1296011487</time>
		</item>
		<item>
		<account_id>386571374</account_id>
		<id>1798</id>
		<message_id>3</message_id>
		<metadata>8</metadata>
		<name>maxisjohn</name>
		<time>1296008728</time>
		</item>
		<item>
		<account_id>386573487</account_id>
		<id>1773</id>
		<message_id>1</message_id>
		<metadata>Synesthesiac</metadata>
		<name>somegamerdood</name>
		<time>1295961120</time>
		</item>
		<item>
		<account_id>386573487</account_id>
		<id>1772</id>
		<message_id>1</message_id>
		<metadata>MaxisJohn</metadata>
		<name>somegamerdood</name>
		<time>1295961116</time>
		</item>
	</items>
</feed>
*/

// Game
namespace Game {
	// Account
	void Account::Read(const pugi::xml_node& node) {
		auto account = node.child("account");
		if (!account) {
			return;
		}

		tutorialCompleted = utils::xml_get_text_node(account, "tutorial_completed") == "Y";
		grantAllAccess = utils::xml_get_text_node<uint32_t>(account, "grant_all_access");
		grantOnlineAccess = utils::xml_get_text_node<uint32_t>(account, "grant_online_access");

		chainProgression = utils::xml_get_text_node<uint32_t>(account, "chain_progression");
		creatureRewards = utils::xml_get_text_node<uint32_t>(account, "creature_rewards");

		currentGameId = utils::xml_get_text_node<uint32_t>(account, "current_game_id");
		currentPlaygroupId = utils::xml_get_text_node<uint32_t>(account, "current_playgroup_id");

		defaultDeckPveId = utils::xml_get_text_node<uint32_t>(account, "default_deck_pve_id");
		defaultDeckPvpId = utils::xml_get_text_node<uint32_t>(account, "default_deck_pvp_id");

		level = utils::xml_get_text_node<uint32_t>(account, "level");
		xp = utils::xml_get_text_node<uint32_t>(account, "xp");
		dna = utils::xml_get_text_node<uint32_t>(account, "dna");
		avatarId = std::clamp<uint32_t>(utils::xml_get_text_node<uint32_t>(account, "avatar_id"), 0, 16);
		id = utils::xml_get_text_node<uint32_t>(account, "id");

		newPlayerInventory = utils::xml_get_text_node<uint32_t>(account, "new_player_inventory");
		newPlayerProgress = utils::xml_get_text_node<uint32_t>(account, "new_player_progress");

		cashoutBonusTime = utils::xml_get_text_node<uint32_t>(account, "cashout_bonus_time");
		starLevel = utils::xml_get_text_node<uint32_t>(account, "star_level");

		unlockCatalysts = utils::xml_get_text_node<uint32_t>(account, "unlock_catalysts");
		unlockDiagonalCatalysts = utils::xml_get_text_node<uint32_t>(account, "unlock_diagonal_catalysts");
		unlockFuelTanks = utils::xml_get_text_node<uint32_t>(account, "unlock_fuel_tanks");
		unlockInventoryIdentify = utils::xml_get_text_node<uint32_t>(account, "unlock_inventory");
		unlockPveDecks = utils::xml_get_text_node<uint32_t>(account, "unlock_pve_decks");
		unlockPvpDecks = utils::xml_get_text_node<uint32_t>(account, "unlock_pvp_decks");
		unlockStats = utils::xml_get_text_node<uint32_t>(account, "unlock_stats");
		unlockInventoryIdentify = utils::xml_get_text_node<uint32_t>(account, "unlock_inventory_identify");
		unlockEditorFlairSlots = utils::xml_get_text_node<uint32_t>(account, "unlock_editor_flair_slots");

		upsell = utils::xml_get_text_node<uint32_t>(account, "upsell");

		capLevel = utils::xml_get_text_node<uint32_t>(account, "cap_level");
		capProgression = utils::xml_get_text_node<uint32_t>(account, "cap_progression");
	}

	void Account::Write(pugi::xml_node& node) const {
		if (auto account = node.append_child("account")) {
			utils::xml_add_text_node(account, "tutorial_completed", tutorialCompleted ? "Y" : "N");
			utils::xml_add_text_node(account, "chain_progression", chainProgression);
			utils::xml_add_text_node(account, "creature_rewards", creatureRewards);
			utils::xml_add_text_node(account, "current_game_id", currentGameId);
			utils::xml_add_text_node(account, "current_playgroup_id", currentPlaygroupId);
			utils::xml_add_text_node(account, "default_deck_pve_id", defaultDeckPveId);
			utils::xml_add_text_node(account, "default_deck_pvp_id", defaultDeckPvpId);
			utils::xml_add_text_node(account, "level", level);
			utils::xml_add_text_node(account, "avatar_id", avatarId);
			utils::xml_add_text_node(account, "blaze_id", id);
			utils::xml_add_text_node(account, "id", id);
			utils::xml_add_text_node(account, "dna", dna);
			utils::xml_add_text_node(account, "new_player_inventory", newPlayerInventory);
			utils::xml_add_text_node(account, "new_player_progress", newPlayerProgress);
			utils::xml_add_text_node(account, "cashout_bonus_time", cashoutBonusTime);
			utils::xml_add_text_node(account, "star_level", starLevel);
			utils::xml_add_text_node(account, "unlock_catalysts", unlockCatalysts);
			utils::xml_add_text_node(account, "unlock_diagonal_catalysts", unlockDiagonalCatalysts);
			utils::xml_add_text_node(account, "unlock_fuel_tanks", unlockFuelTanks);
			utils::xml_add_text_node(account, "unlock_inventory", unlockInventoryIdentify);
			utils::xml_add_text_node(account, "unlock_pve_decks", unlockPveDecks);
			utils::xml_add_text_node(account, "unlock_pvp_decks", unlockPvpDecks);
			utils::xml_add_text_node(account, "unlock_stats", unlockStats);
			utils::xml_add_text_node(account, "unlock_inventory_identify", unlockInventoryIdentify);
			utils::xml_add_text_node(account, "unlock_editor_flair_slots", unlockEditorFlairSlots);
			utils::xml_add_text_node(account, "upsell", upsell);
			utils::xml_add_text_node(account, "xp", xp);
			utils::xml_add_text_node(account, "grant_all_access", grantAllAccess ? "1" : "0");
			utils::xml_add_text_node(account, "grant_online_access", grantOnlineAccess ? "1" : "0");
			utils::xml_add_text_node(account, "cap_level", capLevel);
			utils::xml_add_text_node(account, "cap_progression", capProgression);
		}
	}

	// Feed
	void Feed::Read(const pugi::xml_node& node) {
		auto feed = node.child("feed");
		if (!feed) {
			return;
		}

		auto items = feed.child("items");
		if (!items) {
			return;
		}

		for (const auto& item : items) {
			decltype(auto) feedItem = mItems.emplace_back();
			feedItem.accountId = utils::xml_get_text_node<uint32_t>(item, "account_id");
			feedItem.id = utils::xml_get_text_node<uint32_t>(item, "id");
			feedItem.messageId = utils::xml_get_text_node<uint32_t>(item, "message_id");
			feedItem.metadata = utils::xml_get_text_node(item, "metadata");
			feedItem.name = utils::xml_get_text_node(item, "name");
			feedItem.timestamp = utils::xml_get_text_node<uint64_t>(item, "time");
		}
	}

	void Feed::Write(pugi::xml_node& node) const {
		if (auto feed = node.append_child("feed")) {
			auto items = feed.append_child("items");
			for (const auto& feedItem : mItems) {
				auto item = items.append_child("item");
				utils::xml_add_text_node(item, "account_id", feedItem.accountId);
				utils::xml_add_text_node(item, "id", feedItem.id);
				utils::xml_add_text_node(item, "message_id", feedItem.messageId);
				utils::xml_add_text_node(item, "metadata", feedItem.metadata);
				utils::xml_add_text_node(item, "name", feedItem.name);
				utils::xml_add_text_node(item, "time", feedItem.timestamp);
			}
		}
	}

	// Part
	Part::Part(uint32_t rigblock) {
		SetRigblock(rigblock);
		SetPrefix(0, false);
		SetPrefix(0, true);
		SetSuffix(0);
	}

	Part::Part(const pugi::xml_node& node) {
		if (!Read(node)) {
			SetRigblock(1);
			SetPrefix(0, false);
			SetPrefix(0, true);
			SetSuffix(0);
		}
	}

	bool Part::Read(const pugi::xml_node& node) {
		std::string_view nodeName = node.name();
		if (nodeName != "part") {
			return false;
		}

		flair = utils::xml_get_text_node<bool>(node, "is_flair");
		cost = utils::xml_get_text_node<uint32_t>(node, "cost");
		equipped_to_creature_id = utils::xml_get_text_node<uint32_t>(node, "creature_id");
		level = utils::xml_get_text_node<uint16_t>(node, "level");
		market_status = utils::xml_get_text_node<uint8_t>(node, "market_status");
		rarity = utils::xml_get_text_node<uint8_t>(node, "rarity");
		status = utils::xml_get_text_node<uint8_t>(node, "status");
		usage = utils::xml_get_text_node<uint8_t>(node, "usage");
		timestamp = utils::xml_get_text_node<uint64_t>(node, "creation_date");

		SetRigblock(utils::xml_get_text_node<uint32_t>(node, "rigblock_asset_id"));
		SetPrefix(utils::xml_get_text_node<uint32_t>(node, "prefix_asset_id"), false);
		SetPrefix(utils::xml_get_text_node<uint32_t>(node, "prefix_secondary_asset_id"), true);
		SetSuffix(utils::xml_get_text_node<uint32_t>(node, "suffix_asset_id"));

		return true;
	}

	void Part::Write(pugi::xml_node& node, uint32_t index, bool api) const {
		if (auto part = node.append_child("part")) {
			utils::xml_add_text_node(part, "is_flair", flair);
			utils::xml_add_text_node(part, "cost", cost);
			utils::xml_add_text_node(part, "creature_id", equipped_to_creature_id);
			utils::xml_add_text_node(part, "level", level);
			utils::xml_add_text_node(part, "market_status", market_status);
			utils::xml_add_text_node(part, "rarity", rarity);
			utils::xml_add_text_node(part, "status", status);
			utils::xml_add_text_node(part, "usage", usage);
			utils::xml_add_text_node(part, "creation_date", timestamp);
			if (api) {
				utils::xml_add_text_node(part, "id", index);
				utils::xml_add_text_node(part, "reference_id", index);

				utils::xml_add_text_node(part, "rigblock_asset_id", rigblock_asset_hash);
				utils::xml_add_text_node(part, "prefix_asset_id", prefix_asset_hash);
				utils::xml_add_text_node(part, "prefix_secondary_asset_id", prefix_secondary_asset_hash);
				utils::xml_add_text_node(part, "suffix_asset_id", suffix_asset_hash);
			} else {
				utils::xml_add_text_node(part, "rigblock_asset_id", rigblock_asset_id);
				utils::xml_add_text_node(part, "prefix_asset_id", prefix_asset_id);
				utils::xml_add_text_node(part, "prefix_secondary_asset_id", prefix_secondary_asset_id);
				utils::xml_add_text_node(part, "suffix_asset_id", suffix_asset_id);
			}
		}
	}

	void Part::SetRigblock(uint16_t rigblock) {
		if (!(rigblock >= 1 && rigblock <= 1573) && !(rigblock >= 10001 && rigblock <= 10835)) {
			rigblock = 1;
		}

		const std::string& rigblock_string = "_Generated/LootRigblock" + std::to_string(rigblock) + ".LootRigblock";
		rigblock_asset_id = rigblock;
		rigblock_asset_hash = utils::hash_id(rigblock_string.c_str());
	}

	void Part::SetPrefix(uint16_t prefix, bool secondary) {
		if (!(prefix >= 1 && prefix <= 338)) {
			prefix = 0;
		}

		std::string prefix_string;
		if (prefix > 0) {
			prefix_string = "_Generated/LootPrefix" + std::to_string(prefix) + ".LootPrefix";
		}

		if (secondary) {
			prefix_secondary_asset_id = prefix;
			prefix_secondary_asset_hash = utils::hash_id(prefix_string.c_str());
		} else {
			prefix_asset_id = prefix;
			prefix_asset_hash = utils::hash_id(prefix_string.c_str());
		}
	}

	void Part::SetSuffix(uint16_t suffix) {
		if (!(suffix >= 1 && suffix <= 83) && !(suffix >= 10001 && suffix <= 10275)) {
			suffix = 0;
		}

		std::string suffix_string;
		if (suffix > 0) {
			suffix_string = "_Generated/LootSuffix" + std::to_string(suffix) + ".LootSuffix";
		}

		suffix_asset_id = suffix;
		suffix_asset_hash = utils::hash_id(suffix_string.c_str());
	}

	void Part::SetStatus(uint8_t newStatus) {
		status = newStatus;
	}

	// Parts
	void Parts::Read(const pugi::xml_node& node) {
		for (const auto& part : node.child("parts")) {
			mItems.emplace_back(part);
		}
	}

	void Parts::Write(pugi::xml_node& node, bool api) const {
		if (auto parts = node.append_child("parts")) {
			uint32_t index = 0;
			for (const auto& part : mItems) {
				part.Write(parts, ++index, api);
			}
		}
	}

	// User
	User::User(const std::string& username) : mUsername(username) {
		// Empty
	}

	User::~User() {
		Save();
	}

	bool User::UpdateState(uint32_t newState) {
		if (mState != newState) {
			mState = newState;
			return true;
		}
		return false;
	}

	Creature* User::GetCreatureById(uint32_t id) {
		Creature* creaturePtr = nullptr;
		for (auto& creature : mCreatures) {
			if (creature.id == id) {
				creaturePtr = &creature;
				break;
			}
		}
		return creaturePtr;
	}

	const Creature* User::GetCreatureById(uint32_t id) const {
		const Creature* creaturePtr = nullptr;
		for (const auto& creature : mCreatures) {
			if (creature.id == id) {
				creaturePtr = &creature;
				break;
			}
		}
		return creaturePtr;
	}

	void User::UnlockCreature(uint32_t templateId) {
		if (mAccount.creatureRewards > 0) {
			mCreatures.Add(templateId);
			mAccount.creatureRewards--;
		}
	}

	void User::UnlockUpgrade(uint32_t unlockId) {
		switch (unlockId) {
			case 1: // Catalysts
				mAccount.unlockCatalysts++;
				break;

			case 8: // Stats
			case 9:
				mAccount.unlockStats++;
				break;

			case 36: // PVE Squads
				mAccount.unlockPveDecks++;
				break;

			case 38: // PVP Squads
				mAccount.unlockPvpDecks++;
				break;

			case 46: // Editor detail slots
				mAccount.unlockEditorFlairSlots++;
				break;

			default:
				break;
		}
	}

	void User::Logout() {
		UserManager::RemoveUser(mUsername);
		Save();
	}

	bool User::Load() {
		std::string filepath = "data/user/" + mUsername + ".xml";

		pugi::xml_document document;
		if (!document.load_file(filepath.c_str())) {
			return false;
		}

		if (auto user = document.child("user")) {
			mName = utils::xml_get_text_node(user, "name");
			// mEmail = utils::xml_get_text_node(user, "email");
			mPassword = utils::xml_get_text_node(user, "password"); // Hash this later?

			mAccount.Read(user);
			mCreatures.Read(user);
			mSquads.Read(user);
			mFeed.Read(user);
			mParts.Read(user);
		}

		// static uint32_t id = 0;
		// mId = ++id;

		return true;
	}

	bool User::Save() {
		std::string filepath = "data/user/" + mUsername + ".xml";

		pugi::xml_document document;
		if (auto user = document.append_child("user")) {
			utils::xml_add_text_node(user, "name", mName);
			// utils::xml_add_text_node(user, "email", mEmail);
			utils::xml_add_text_node(user, "password", mPassword);

			mAccount.Write(user);
			mCreatures.Write(user);
			mSquads.Write(user);
			mFeed.Write(user);
			mParts.Write(user);
		}

		return document.save_file(filepath.c_str(), "\t", 1U, pugi::encoding_latin1);
	}

	// UserManager
	std::map<std::string, UserPtr> UserManager::sUsersByEmail;

	UserPtr UserManager::GetUserByEmail(const std::string& username) {
		UserPtr user;

		auto it = sUsersByEmail.find(username);
		if (it != sUsersByEmail.end()) {
			user = it->second;
		} else {
			user = std::make_shared<User>(username);
			if (user->Load()) {
				sUsersByEmail.emplace(username, user);
			} else {
				user.reset();
			}
		}

		return user;
	}

	UserPtr UserManager::GetUserByAuthToken(const std::string& authToken) {
		for (const auto& [_, user] : sUsersByEmail) {
			if (user->get_auth_token() == authToken) {
				return user;
			}
		}
		return nullptr;
	}

	void UserManager::RemoveUser(const std::string& email) {
		auto it = sUsersByEmail.find(email);
		if (it != sUsersByEmail.end()) {
			sUsersByEmail.erase(it);
		}
	}
}
