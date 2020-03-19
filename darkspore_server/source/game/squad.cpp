
// Include
#include "squad.h"
#include "../utils/functions.h"
#include <algorithm>

// Game
namespace Game {
	// Squad
	void Squad::Read(const pugi::xml_node& node) {
		std::string_view nodeName = node.name();
		if (nodeName != "deck") {
			return;
		}

		name = utils::xml_get_text_node(node, "name");
		category = utils::xml_get_text_node(node, "category");

		id = utils::xml_get_text_node<uint32_t>(node, "id");
		slot = utils::xml_get_text_node<uint32_t>(node, "slot");

		locked = utils::xml_get_text_node<uint32_t>(node, "locked");

		creatures.Read(node);
	}

	void Squad::Write(pugi::xml_node& node) const {
		if (auto deck = node.append_child("deck")) {
			utils::xml_add_text_node(deck, "name", name);
			utils::xml_add_text_node(deck, "id", id);
			utils::xml_add_text_node(deck, "category", category);
			utils::xml_add_text_node(deck, "slot", slot);
			utils::xml_add_text_node(deck, "locked", locked ? 1 : 0);
			creatures.Write(deck);
		}
	}

	// Squads
	void Squads::Read(const pugi::xml_node& node) {
		auto decks = node.child("decks");
		if (!decks) {
			return;
		}

		for (const auto& deckNode : decks) {
			decltype(auto) squad = mSquads.emplace_back();
			squad.Read(deckNode);
		}
	}

	void Squads::Write(pugi::xml_node& node) const {
		if (auto decks = node.append_child("decks")) {
			for (const auto& squad : mSquads) {
				squad.Write(decks);
			}
		}
	}

	Squad* Squads::Create() {
		if (mSquads.size() >= 3) {
			return nullptr;
		}

		auto& squad = mSquads.emplace_back();
		squad.id = static_cast<uint32_t>(mSquads.size());
		squad.slot = squad.id;

		return &squad;
	}

	void Squads::Remove(size_t index) {
		if (index < mSquads.size()) {
			mSquads.erase(mSquads.begin() + index);
		}
	}

	Squad* Squads::Get(size_t index) {
		if (index < mSquads.size()) {
			return &mSquads[index];
		}
		return nullptr;
	}
}
