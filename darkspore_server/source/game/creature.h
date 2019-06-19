
#ifndef _GAME_CREATURE_HEADER
#define _GAME_CREATURE_HEADER

// Include
#include <cstdint>
#include <string>
#include <vector>
#include <pugixml.hpp>

// Game
namespace Game {
	// Creature template IDs
	enum class CreatureID : uint32_t {
		BlitzAlpha = 1667741389,
		SageAlpha = 749013658,
		WraithAlpha = 3591937345
	};

	// Creature
	struct Creature {
		std::string name;
		std::string pngThumbUrl;

		uint32_t id = 0;
		uint32_t nounId = 0;
		uint32_t version = 0;
		uint32_t gearScore = 0;
		uint32_t itemPoints = 0;

		void Read(const pugi::xml_node& node);
		void Write(pugi::xml_node& node) const;
	};

	// Creatures
	class Creatures {
		public:
			decltype(auto) begin() { return mCreatures.begin(); }
			decltype(auto) begin() const { return mCreatures.begin(); }
			decltype(auto) end() { return mCreatures.end(); }
			decltype(auto) end() const { return mCreatures.end(); }

			auto& data() { return mCreatures; }
			const auto& data() const { return mCreatures; }

			void Read(const pugi::xml_node& node);
			void Write(pugi::xml_node& node) const;

			void Add(uint32_t templateId);

		private:
			std::vector<Creature> mCreatures;
	};
}

#endif
