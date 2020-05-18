
// Include
#include "creature.h"
#include "instance.h"

#include "game/config.h"
#include "utils/functions.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

// SporeNet
namespace SporeNet {
	// CreatureType
	void from_string(const std::string& src, CreatureType& dst) {
		using namespace std::literals;

		std::string_view srcView = src;
		if (utils::string_iequals(srcView, "bio"sv)) {
			dst = CreatureType::Bio;
		} else if (utils::string_iequals(srcView, "cyber"sv)) {
			dst = CreatureType::Cyber;
		} else if (utils::string_iequals(srcView, "plasma"sv)) {
			dst = CreatureType::Plasma;
		} else if (utils::string_iequals(srcView, "necro"sv)) {
			dst = CreatureType::Necro;
		} else if (utils::string_iequals(srcView, "chrono"sv)) {
			dst = CreatureType::Chrono;
		} else if (utils::string_iequals(srcView, "all"sv)) {
			dst = CreatureType::All;
		} else {
			dst = CreatureType::Unknown;
		}
	}

	std::string to_string(CreatureType value) {
		switch (value) {
			case CreatureType::Bio:
				return "bio";

			case CreatureType::Cyber:
				return "cyber";

			case CreatureType::Plasma:
				return "plasma";

			case CreatureType::Necro:
				return "necro";

			case CreatureType::Chrono:
				return "chrono";

			case CreatureType::All:
				return "all";

			default:
				return "unknown";
		}
	}

	// CreatureClass
	void from_string(const std::string& src, CreatureClass& dst) {
		using namespace std::literals;

		std::string_view srcView = src;
		if (utils::string_iequals(srcView, "ravager"sv)) {
			dst = CreatureClass::Ravager;
		} else if (utils::string_iequals(srcView, "sentinel"sv)) {
			dst = CreatureClass::Sentinel;
		} else if (utils::string_iequals(srcView, "tempest"sv)) {
			dst = CreatureClass::Tempest;
		} else if (utils::string_iequals(srcView, "all"sv)) {
			dst = CreatureClass::All;
		} else {
			dst = CreatureClass::Unknown;
		}
	}

	std::string to_string(CreatureClass value) {
		switch (value) {
			case CreatureClass::Ravager:
				return "ravager";

			case CreatureClass::Sentinel:
				return "sentinel";

			case CreatureClass::Tempest:
				return "tempest";

			case CreatureClass::All:
				return "all";

			default:
				return "unknown";
		}
	}

	// CreatureParts
	void from_string(const std::string& src, CreatureParts& dst) {
		using namespace std::literals;

		std::string_view srcView = src;
		if (utils::string_iequals(srcView, "no_feet"sv)) {
			dst = CreatureParts::NoFeet;
		} else if (utils::string_iequals(srcView, "no_hands"sv)) {
			dst = CreatureParts::NoHands;
		} else if (utils::string_iequals(srcView, "all"sv)) {
			dst = CreatureParts::All;
		} else {
			dst = CreatureParts::Unknown;
		}
	}

	std::string to_string(CreatureParts value) {
		switch (value) {
			case CreatureParts::NoFeet:
				return "no_feet";

			case CreatureParts::NoHands:
				return "no_hands";

			case CreatureParts::All:
				return "all";

			default:
				return "unknown";
		}
	}

	/*
		name_locale_id
		text_locale_id

		if not template {
			creator_id
		}

		stats_template_ability
			example {
				full_string = item0;item1;itemN
				item = a!b,value
			}

		if not template {
			stats
			stats_template_ability_keyvalues
			stats_ability_keyvalues
		} else {
			stats_template
			stats_template_ability_keyvalues
		}

		if not template {
			parts
				part
		}

		if not template {
			png_large_url
			png_thumb_url
		}
	*/

	// TemplateCreature
	void TemplateCreature::Read(const pugi::xml_node& node) {
		mNoun = utils::hash_id(node.attribute("noun").as_string());

		mNameLocaleId = utils::xml_get_text_node(node, "name_locale_id");
		mTextLocaleId = utils::xml_get_text_node(node, "text_locale_id");

		mName = utils::xml_get_text_node(node, "name");
		from_string(utils::xml_get_text_node(node, "type_a"), mType);
		mBaseWeaponDamageMin = utils::xml_get_text_node<double>(node, "weapon_min_damage");
		mBaseWeaponDamageMax = utils::xml_get_text_node<double>(node, "weapon_max_damage");
		mGearScore = utils::xml_get_text_node<float>(node, "gear_score");
		from_string(utils::xml_get_text_node(node, "class"), mClass);
		from_string(utils::xml_get_text_node(node, "creature_parts"), mEquipableParts);

		mAbility[0] = utils::hash_id(utils::xml_get_text_node(node, "ability_basic").c_str());
		mAbility[1] = utils::hash_id(utils::xml_get_text_node(node, "ability_special_1").c_str());
		mAbility[2] = utils::hash_id(utils::xml_get_text_node(node, "ability_special_2").c_str());
		mAbility[3] = utils::hash_id(utils::xml_get_text_node(node, "ability_random").c_str());
		mAbility[4] = utils::hash_id(utils::xml_get_text_node(node, "ability_passive").c_str());

		// parse stats
		const auto& statsString = utils::xml_get_text_node(node, "stats_template");

		mStats.clear();
		for (const auto& stat : utils::explode_string(statsString, ';')) {
			if (stat.empty()) {
				continue;
			}

			const auto& statData = utils::explode_string(stat, ',');

			Stats stat;
			stat.statName = statData[0];
			stat.maxValue = utils::to_number<uint32_t>(statData[1]);
			stat.currentValue = utils::to_number<uint32_t>(statData[2]);

			mStats.push_back(std::move(stat));
		}
	}

	void TemplateCreature::WriteApi(pugi::xml_node& node, bool include_abilities) const {
		utils::xml_add_text_node(node, "name_locale_id", mNameLocaleId);
		utils::xml_add_text_node(node, "text_locale_id", mTextLocaleId);

		utils::xml_add_text_node(node, "name", mName);
		utils::xml_add_text_node(node, "type_a", to_string(mType));
		utils::xml_add_text_node(node, "weapon_min_damage", mBaseWeaponDamageMin);
		utils::xml_add_text_node(node, "weapon_max_damage", mBaseWeaponDamageMax);
		utils::xml_add_text_node(node, "gear_score", mGearScore);
		utils::xml_add_text_node(node, "class", to_string(mClass));
		utils::xml_add_text_node(node, "creature_parts", to_string(mEquipableParts));

		{
			std::string statsString;
			for (const auto& stat : mStats) {
				statsString += stat.statName + ',' + std::to_string(stat.maxValue) + ',' + std::to_string(stat.currentValue) + ';';
			}
			utils::xml_add_text_node(node, "stats_template", statsString);
		}
		// stats_template
		// stats_template_ability_keyvalues

		if (include_abilities) {
			utils::xml_add_text_node(node, "ability_basic", mAbility[0]);
			utils::xml_add_text_node(node, "ability_special_1", mAbility[1]);
			utils::xml_add_text_node(node, "ability_special_2", mAbility[2]);
			utils::xml_add_text_node(node, "ability_random", mAbility[3]);
			utils::xml_add_text_node(node, "ability_passive", mAbility[4]);

			if (auto abilityNode = node.append_child("ability")) {
				for (const auto value : mAbility) {
					utils::xml_add_text_node(abilityNode, "id", value);
				}
			}
		}
	}

	const std::string& TemplateCreature::GetName() const {
		return mName;
	}

	uint32_t TemplateCreature::GetNoun() const {
		return mNoun;
	}

	CreatureType TemplateCreature::GetType() const {
		return mType;
	}

	CreatureClass TemplateCreature::GetClass() const {
		return mClass;
	}

	CreatureParts TemplateCreature::GetEquipableParts() const {
		return mEquipableParts;
	}

	// TemplateDatabase
	void TemplateDatabase::Load() {
		const auto& templateCreaturePath = Game::Config::Get(Game::ConfigValue::CONFIG_TEMPLATE_CREATURE_PATH);
		for (const auto& path : std::filesystem::directory_iterator(templateCreaturePath)) {
			const auto& filePath = path.path();
			if (path.is_regular_file() && filePath.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(filePath.c_str())) {
					TemplateCreaturePtr templateCreature = std::make_shared<TemplateCreature>();
					templateCreature->Read(document.child("template"));

					mTemplateCreatures.try_emplace(templateCreature->GetName(), templateCreature);
				} else {
					std::cout << "Could not load '" << filePath << "'." << std::endl;
				}
			}
		}
	}

	TemplateCreaturePtr TemplateDatabase::Get(const std::string& name) const {
		auto it = mTemplateCreatures.find(name);
		return it != mTemplateCreatures.end() ? it->second : nullptr;
	}

	TemplateCreaturePtr TemplateDatabase::Get(uint32_t noun) const {
		for (const auto& [_, templateCreature] : mTemplateCreatures) {
			if (templateCreature->GetNoun() == noun) {
				return templateCreature;
			}
		}
		return nullptr;
	}

	// Creature
	Creature::Creature(const TemplateCreaturePtr& templateCreature)
		: mTemplate(templateCreature) {}

	void Creature::Read(const pugi::xml_node& node) {
		auto templateName = node.attribute("template").as_string();
		mTemplate = SporeNet::Get().GetTemplateDatabase().Get(templateName);

		mId = utils::xml_get_text_node<uint32_t>(node, "id");
		mVersion = utils::xml_get_text_node<uint32_t>(node, "version");

		mGearScore = utils::xml_get_text_node<float>(node, "gear_score"); // Shows up as "level" ingame
		mItemPoints = utils::xml_get_text_node<float>(node, "item_points");

		mPngLargeUrl = utils::xml_get_text_node(node, "png_large_url");
		mPngThumbUrl = utils::xml_get_text_node(node, "png_thumb_url");
	}

	void Creature::Write(pugi::xml_node& node) const {
		node.append_attribute("template").set_value(mTemplate->GetName().c_str());

		utils::xml_add_text_node(node, "id", mId);
		utils::xml_add_text_node(node, "version", mVersion);
		utils::xml_add_text_node(node, "gear_score", mGearScore);
		utils::xml_add_text_node(node, "item_points", mItemPoints);
		utils::xml_add_text_node(node, "png_large_url", mPngLargeUrl);
		utils::xml_add_text_node(node, "png_thumb_url", mPngThumbUrl);
	}

	void Creature::WriteApi(pugi::xml_node& node, bool profile_data, bool include_abilities, bool include_parts) const {
		if (!mTemplate) {
			// Invalid creature object
			return;
		}

		if (profile_data) {
			mTemplate->WriteApi(node, include_abilities);
			node.remove_child("stats_template");

			utils::xml_add_text_node(node, "creator_id", mCreatorId);
			utils::xml_add_text_node(node, "stats", "TODO");

			// stats_ability_keyvalues (uses template by default if this doesnt exist)
		} else {
			utils::xml_add_text_node(node, "id", mId);
			utils::xml_add_text_node(node, "name", mTemplate->GetName());
			utils::xml_add_text_node(node, "png_large_url", mPngLargeUrl);
			utils::xml_add_text_node(node, "png_thumb_url", mPngThumbUrl);
			utils::xml_add_text_node(node, "noun_id", mTemplate->GetNoun());
			utils::xml_add_text_node(node, "version", mVersion);
			utils::xml_add_text_node(node, "gear_score", mGearScore);
			utils::xml_add_text_node(node, "item_points", mItemPoints);
		}

		if (include_parts) {
			if (auto partsNode = node.append_child("parts")) {
				// add parts
			}
		}
	}

	const std::string& Creature::GetName() const {
		return mTemplate != nullptr ? mTemplate->GetName() : std::string();
	}

	uint32_t Creature::GetNoun() const {
		return mTemplate != nullptr ? mTemplate->GetNoun() : 0;
	}

	CreatureType Creature::GetType() const {
		return mTemplate != nullptr ? mTemplate->GetType() : CreatureType::Unknown;
	}

	CreatureClass Creature::GetClass() const {
		return mTemplate != nullptr ? mTemplate->GetClass() : CreatureClass::Unknown;
	}

	CreatureParts Creature::GetEquipableParts() const {
		return mTemplate != nullptr ? mTemplate->GetEquipableParts() : CreatureParts::Unknown;
	}

	const std::string& Creature::GetLargeImageURL() const {
		return mPngLargeUrl;
	}

	const std::string& Creature::GetThumbImageURL() const {
		return mPngThumbUrl;
	}

	float Creature::GetGearScore() const {
		return mGearScore;
	}

	float Creature::GetGearScoreFlattened() const {
		return std::floor(mGearScore);
	}

	float Creature::GetItemPoints() const {
		return mItemPoints;
	}

	int64_t Creature::GetCreatorId() const {
		return mCreatorId;
	}

	uint32_t Creature::GetId() const {
		return mId;
	}

	void Creature::SetId(uint32_t id) {
		mId = id;
	}

	uint32_t Creature::GetVersion() const {
		return mVersion;
	}

	void Creature::SetVersion(uint32_t version) {
		mVersion = version;
	}

	// Creatures
	void Creatures::Read(const pugi::xml_node& node) {
		auto creatures = node.child("creatures");
		if (!creatures) {
			return;
		}

		for (const auto& creatureNode : creatures) {
			std::string_view nodeName = creatureNode.name();
			if (nodeName == "creature") {
				auto creature = std::make_shared<Creature>();
				creature->Read(creatureNode);
				mCreatures.push_back(std::move(creature));
			}
		}
	}

	void Creatures::Write(pugi::xml_node& node) const {
		if (auto creaturesNode = node.append_child("creatures")) {
			for (const auto& creature : mCreatures) {
				if (auto creatureNode = creaturesNode.append_child("creature")) {
					creature->Write(creatureNode);
				}
			}
		}
	}

	void Creatures::WriteApi(pugi::xml_node& node) const {
		if (auto creaturesNode = node.append_child("creatures")) {
			for (const auto& creature : mCreatures) {
				if (auto creatureNode = creaturesNode.append_child("creature")) {
					creature->WriteApi(creatureNode, false, false, false);
				}
			}
		}
	}

	void Creatures::Add(uint32_t templateId) {
		// TODO: create unique ids for all creatures
		auto templateCreature = SporeNet::Get().GetTemplateDatabase().Get(templateId);
		if (templateCreature) {
			// TODO: global ids (need a database first)
			auto creature = std::make_shared<Creature>(templateCreature);
			if (mCreatures.empty()) {
				creature->SetId(1);
			} else {
				creature->SetId(mCreatures.back()->GetId() + 1);
			}
			creature->SetVersion(2);
			mCreatures.push_back(std::move(creature));
		}
	}

	CreaturePtr Creatures::Get(size_t creatureId) const {
		for (const auto& creature : mCreatures) {
			if (creature->GetId() == creatureId) {
				return creature;
			}
		}
		return nullptr;
	}
}