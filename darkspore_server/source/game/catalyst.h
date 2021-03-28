
#ifndef _GAME_CATALYST_HEADER
#define _GAME_CATALYST_HEADER

// Include
#include <cstdint>
#include <string>
#include <memory>

// Predefine
namespace RakNet {
	class BitStream;
}

// Game
namespace Game {
	enum class CatalystColor : uint8_t {
		Blue = 0,
		Yellow,
		Pink,
		Red,
		Green,
		Prismatic
	};

	enum class CatalystRarity : uint8_t {
		Common = 0,
		Rare,
		Epic
	};

	enum class CatalystType : uint8_t {
		AoEDamage = 0,
		AttackSpeed,
		BuffDuration,
		CCReduction,
		Cooldown,
		Crit,
		Damage,
		DamageAura,
		DebuffDuration,
		DebuffIncrease,
		DefenseRating,
		DeflectionRating,
		Dexterity,
		DodgeRating,
		Health,
		ImmunePoison,
		ImmuneSleep,
		ImmuneSlow,
		ImmuneStun,
		Knockback,
		LifeLeech,
		Mana,
		ManaCostReduction,
		ManaLeech,
		Mind,
		MoveSpeed,
		OrbEffectiveness,
		OverdriveBuildup,
		PetDamage,
		PetHealth,
		ProjectileSpeed,
		RangeIncrease,
		Strength,
		Surefooted,
		Thorns
	};

	std::string to_string(CatalystColor color);
	std::string to_string(CatalystRarity rarity);
	std::string to_string(CatalystType type);

	// Catalyst
	class Catalyst {
		public:
			Catalyst() = default;
			Catalyst(CatalystType type, CatalystRarity rarity, bool prismatic);

			uint32_t GetNounId() const;
			uint16_t GetLevel() const;

			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

		private:
			uint32_t mNounId = 0;

			uint16_t mLevel = 0;
	};
}

#endif
