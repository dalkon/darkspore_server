
#ifndef _GAME_ATTRIBUTES_HEADER
#define _GAME_ATTRIBUTES_HEADER

// Include
#include <cstdint>
#include <vector>
#include <bitset>
#include <memory>

// RakNet
namespace RakNet {
	class BitStream;
}

// Game
namespace Game {
	// AttributeType
	enum class AttributeType : uint8_t {
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
		AoEResistance,
		EnergyDamageBuff,
		Intangible,
		HealingReduction,
		EnergyDamageIncrease,
		EnergyDamageIncreaseFlat,
		Immune,
		StealthDetection,
		LifeSteal,
		RejectModifier,
		AoEDamage,
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
		DoTDamageIncrease,
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
		DoTDamageDoneIncrease,
		AggroIncrease,
		AggroDecrease,
		PhysicalDamageDoneIncrease,
		PhysicalDamageDoneByAbilityIncrease,
		EnergyDamageDoneIncrease,
		EnergyDamageDoneByAbilityIncrease,
		ChannelTimeDecrease,
		CrowdControlDurationDecrease,
		DoTDurationDecrease,
		AoEDurationIncrease,
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
		ClientCount,

		ImmuneToShocked = ClientCount,
		ImmuneToTaunted,
		DistributeDamageAmongSquad,
		DefenseBoostPhysicalDamage,
		Count
	};

	// Attributes
	class Attributes {
		public:
			Attributes() = default;

			const auto& GetData() const { return mData; }
			const auto& GetErasedData() const { return mErasedData; }
			const auto& GetDataBits() const { return mDataBits; }

			float GetValue(uint8_t idx) const;
			float GetValue(AttributeType idx) const;
			void SetValue(uint8_t idx, float value);
			void SetValue(AttributeType idx, float value);

			void WriteTo(RakNet::BitStream& stream) const;
			void WriteReflection(RakNet::BitStream& stream) const;

			void ResetReflectionBits();

		private:
			static constexpr uint8_t sClientVisibleAttributeCount = static_cast<uint8_t>(AttributeType::ClientCount) + 2;

			std::vector<std::tuple<uint8_t, float>> mData;
			std::vector<uint8_t> mErasedData;
			std::bitset<sClientVisibleAttributeCount> mDataBits;

			float mMinWeaponDamage = 0;
			float mMaxWeaponDamage = 0;
	};

	using AttributesPtr = std::shared_ptr<Attributes>;
}

#endif
