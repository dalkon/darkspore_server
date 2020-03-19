
#ifndef _GAME_CREATURE_HEADER
#define _GAME_CREATURE_HEADER

// Include
#include <cstdint>
#include <string>
#include <vector>
#include <pugixml.hpp>

namespace Ability {
	enum : uint32_t {
		Strength = 0,
		Dexterity,
		Mind,
		MaxHealthIncrease,
		MaxHealth,
		MaxMana,
		DamageReduction,
		PhysicalDefense,
		PhysicalDamageReduction,
		EnergyDefense,
		CriticalRating,
		NonCombatSpeed,
		CombatSpeed,
		DamageBuff,
		Silence,
		Immobilized,
		DefenseBoostBasicDamage,
		PhysicalDamageIncrease,
		PhysicalDamageIncreaseFlat,
		AutoCrit,
		BehindDirectDamageIncrease,
		BehindOrSideDirectDamageIncrease,
		CriticalDamageIncrease,
		AttackSpeedScale,
		CooldownScale,
		Frozen,
		ProjectileSpeedIncrease,
		AoeResistance,
		EnergyDamageBuff,
		Intangible,
		HealingReduction,
		EnergyDamageIncrease,
		EnergyDamageIncreaseFlat,
		Immune,
		StealthDetection,
		LifeSteal,
		RejectModifier,
		AoeDamage,
		TechnologyTypeDamage,
		SpacetimeTypeDamage,
		LifeTypeDamage,
		ElementsTypeDamage,
		SupernaturalTypeDamage,
		TechnologyTypeResistance,
		SpacetimeTypeResistance,
		LifeTypeResistance,
		ElementsTypeResistance,
		SupernaturalTypeResistance,
		MovementSpeedBuff,
		ImmuneToDebuffs,
		BuffDuration,
		DebuffDuration,
		ManaSteal,
		DebuffDurationIncrease,
		EnergyDamageReduction,
		Incorporeal,
		DotDamageIncrease,
		MindControlled,
		SwapDisabled,
		ImmuneToRandomTeleport,
		ImmuneToBanish,
		ImmuneToKnockback,
		AoeRadius,
		PetDamage,
		PetHealth,
		CrystalFind,
		DNADropped,
		RangeIncrease,
		OrbEffectiveness,
		OverdriveBuildup,
		OverdriveDuration,
		LootFind,
		Surefooted,
		ImmuneToStunned,
		ImmuneToSleep,
		ImmuneToTerrified,
		ImmuneToSilence,
		ImmuneToCursed,
		ImmuneToPoisonOrDisease,
		ImmuneToBurning,
		ImmuneToRooted,
		ImmuneToSlow,
		ImmuneToPull,
		DotDamageDoneIncrease,
		AggroIncrease,
		AggroDecrease,
		PhysicalDamageDoneIncrease,
		PhysicalDamageDoneByAbilityIncrease,
		EnergyDamageDoneIncrease,
		EnergyDamageDoneByAbilityIncrease,
		ChannelTimeDecrease,
		CrowdControlDurationDecrease,
		DotDurationDecrease,
		AoeDurationIncrease,
		HealIncrease,
		OnLockdown,
		HoTDoneIncrease,
		ProjectileDamageIncrease,
		DeployBonusInvincibilityTime,
		PhysicalDamageDecreaseFlat,
		EnergyDamageDecreaseFlat,
		MinWeaponDamage,
		MaxWeaponDamage,
		MinWeaponDamagePercent,
		MaxWeaponDamagePercent,
		DirectAttackDamage,
		DirectAttackDamagePercent,
		GetHitAnimDisabled,
		XPBoost,
		InvisibleToSecurityTeleporters,
		BodyScale,
		Count
	};
}

// Game
namespace Game {
	// Creature template IDs
	enum class CreatureID : uint64_t {
		BlitzAlpha = 1667741389,
		SageAlpha = 749013658,
		WraithAlpha = 3591937345,
		GoliathAlpha = 3376462303,
		ZrinAlpha = 576185321,
		AraknaAlpha = 2128242117,
		VexAlpha = 3618328019,
		ViperAlpha = 1237326306,
		LuminAlpha = 963807461,
		JinxAlpha = 3914325063,
		Srs42Alpha = 3524487169,
		MagnosAlpha = 2954534111,
		ArborusAlpha = 1770764384,
		TitanAlpha = 3068979135,
		MaldriAlpha = 3911756266,
		RevenantAlpha = 38011465583,
		KrelAlpha = 1392397193,
		AndromedaAlpha = 3877795735,
		MeditronAlpha = 1557965449,
		SavageAlpha = 3660811270,
		BlitzBeta = 454054015,
		SageBeta = 1986637994,
		WraithBeta = 4201941707,
		ZrinBeta = 2530835539,
		GoliathBeta = 3948469269,
		ViperBeta = 3600769362,
		SeraphXSDelta = 4234304479,
		SeraphXSGamma = 2464059380,
		SeraphXSAlpha = 820281267,
		SeraphXSBeta = 2726720353,
		VexBeta = 3918493889,
		LuminBeta = 1475341687,
		AraknaBeta = 818452503,
		SRS42Beta = 2445894411,
		MagnosBeta = 3639041301,
		JinxBeta = 1442915581,
		TorkAlpha = 1957997466,
	};

	enum class CreatureType : uint16_t {
		Bio,
		Cyber,
		Plasma,
		Necro,
		Chrono,
		All
	};

	std::string to_string(CreatureType type);

	// Creature
	struct Creature {
		using Ptr = std::shared_ptr<Creature>;

		std::string name;
		std::string nameLocaleId;
		std::string elementType;
		std::string classType;
		std::string pngLargeUrl;
		std::string pngThumbUrl;

		double gearScore = 0;
		double itemPoints = 0;

		uint32_t id = 0;
		uint64_t nounId = 0;
		uint32_t version = 0;

		CreatureType type = CreatureType::All;

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

			Creature::Ptr Get(size_t creatureId) const;

		private:
			std::vector<Creature::Ptr> mCreatures;
	};
}

#endif
