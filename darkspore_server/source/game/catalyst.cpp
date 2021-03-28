
// Include
#include "catalyst.h"

#include "utils/functions.h"

#include "raknet/types.h"

// Game
namespace Game {
	std::string to_string(CatalystColor color) {
		switch (color) {
			case CatalystColor::Blue: return "blue";
			case CatalystColor::Yellow: return "yellow";
			case CatalystColor::Pink: return "pink";
			case CatalystColor::Red: return "red";
			case CatalystColor::Green: return "green";
			case CatalystColor::Prismatic: return "prismatic";
		}
		return "unknown";
	}

	std::string to_string(CatalystRarity rarity) {
		switch (rarity) {
			case CatalystRarity::Common: return "common";
			case CatalystRarity::Rare: return "rare";
			case CatalystRarity::Epic: return "epic";
		}
		return "unknown";
	}

	std::string to_string(CatalystType type) {
		switch (type) {
			case CatalystType::AoEDamage: return "aoedamage";
			case CatalystType::AttackSpeed: return "attackspeed";
			case CatalystType::BuffDuration: return "buffduration";
			case CatalystType::CCReduction: return "ccreduction";
			case CatalystType::Cooldown: return "cooldown";
			case CatalystType::Crit: return "crit";
			case CatalystType::Damage: return "damage";
			case CatalystType::DamageAura: return "damageaura";
			case CatalystType::DebuffDuration: return "debuffduration";
			case CatalystType::DebuffIncrease: return "debuffincrease";
			case CatalystType::DefenseRating: return "defenserating";
			case CatalystType::DeflectionRating: return "deflectionrating";
			case CatalystType::Dexterity: return "dexterity";
			case CatalystType::DodgeRating: return "dodgerating";
			case CatalystType::Health: return "health";
			case CatalystType::ImmunePoison: return "immunepoison";
			case CatalystType::ImmuneSleep: return "immunesleep";
			case CatalystType::ImmuneSlow: return "immuneslow";
			case CatalystType::ImmuneStun: return "immunestun";
			case CatalystType::Knockback: return "knockback";
			case CatalystType::LifeLeech: return "lifeleech";
			case CatalystType::Mana: return "mana";
			case CatalystType::ManaCostReduction: return "manacostreduction";
			case CatalystType::ManaLeech: return "manaleech";
			case CatalystType::Mind: return "mind";
			case CatalystType::MoveSpeed: return "movespeed";
			case CatalystType::OrbEffectiveness: return "orbeffectiveness";
			case CatalystType::OverdriveBuildup: return "overdrivebuildup";
			case CatalystType::PetDamage: return "petdamage";
			case CatalystType::PetHealth: return "pethealth";
			case CatalystType::ProjectileSpeed: return "projectilespeed";
			case CatalystType::RangeIncrease: return "rangeincrease";
			case CatalystType::Strength: return "strength";
			case CatalystType::Surefooted: return "surefooted";
			case CatalystType::Thorns: return "thorns";
		}
		return "unknown";
	}

	// Catalyst
	Catalyst::Catalyst(CatalystType type, CatalystRarity rarity, bool prismatic) {
		std::string noun = "crystal_";
		if (prismatic) {
			noun += "wild_";
		}

		noun += to_string(type);
		switch (rarity) {
			case CatalystRarity::Rare:
				noun += "_rare";
				break;

			case CatalystRarity::Epic:
				noun += "_epic";
				break;
		}

		mNounId = utils::hash_id(noun + ".noun");
		mLevel = static_cast<uint16_t>(rarity);
	}

	uint32_t Catalyst::GetNounId() const {
		return mNounId;
	}

	uint16_t Catalyst::GetLevel() const {
		return mLevel;
	}

	void Catalyst::WriteTo(RakNet::BitStream& stream) const {
		Write<uint32_t>(stream, mNounId);
		Write<uint16_t>(stream, mLevel);

		// padding?
		Write<uint32_t>(stream, 0x00);
		Write<uint32_t>(stream, 0x00);
		Write<uint16_t>(stream, 0x00);
	}

	void Catalyst::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<2> reflector(stream);
		reflector.begin();
		reflector.write<0>(mNounId);
		reflector.write<1>(mLevel);
		reflector.end();
	}
}
