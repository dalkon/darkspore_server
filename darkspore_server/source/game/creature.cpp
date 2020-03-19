
// Include
#include "creature.h"
#include "../utils/functions.h"
#include <algorithm>

// Game
namespace Game {
	// CreatureType
	std::string to_string(CreatureType type) {
		switch (type) {
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

	// Creature
	void Creature::Read(const pugi::xml_node& node) {
		name = utils::xml_get_text_node(node, "name");
		nameLocaleId = utils::xml_get_text_node(node, "name_locale_id");
		elementType = utils::xml_get_text_node(node, "type_a");
		classType = utils::xml_get_text_node(node, "class");
		pngLargeUrl = utils::xml_get_text_node(node, "png_large_url");
		pngThumbUrl = utils::xml_get_text_node(node, "png_thumb_url");

		gearScore = utils::xml_get_text_node<double>(node, "gear_score");
		itemPoints = utils::xml_get_text_node<double>(node, "item_points");

		id = utils::xml_get_text_node<uint32_t>(node, "id");
		nounId = utils::xml_get_text_node<uint64_t>(node, "noun_id");
		version = utils::xml_get_text_node<uint32_t>(node, "version");
	}

	void Creature::Write(pugi::xml_node& node) const {
		utils::xml_add_text_node(node, "id", id);
		utils::xml_add_text_node(node, "name", name);
		utils::xml_add_text_node(node, "name_locale_id", nameLocaleId);
		utils::xml_add_text_node(node, "type_a", elementType);
		utils::xml_add_text_node(node, "class", classType);
		utils::xml_add_text_node(node, "png_large_url", pngLargeUrl);
		utils::xml_add_text_node(node, "png_thumb_url", pngThumbUrl);
		utils::xml_add_text_node(node, "noun_id", nounId);
		utils::xml_add_text_node(node, "version", version);
		utils::xml_add_text_node(node, "gear_score", gearScore);
		utils::xml_add_text_node(node, "item_points", itemPoints);
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

	void Creatures::Add(uint32_t templateId) {
		for (const auto& creature : mCreatures) {
			if (creature->nounId == templateId) {
				return;
			}
		}

		auto creature = std::make_shared<Creature>();

		creature->id = static_cast<uint32_t>(mCreatures.size()); // TODO: change this for a proper global id
		creature->nounId = templateId;

		mCreatures.push_back(std::move(creature));
	}

	Creature::Ptr Creatures::Get(size_t creatureId) const {
		for (const auto& creature : mCreatures) {
			if (creature->id == creatureId) {
				return creature;
			}
		}
		return nullptr;
	}
}
