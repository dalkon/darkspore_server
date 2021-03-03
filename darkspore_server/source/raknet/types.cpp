
// Include
#include "types.h"
#include "utils/functions.h"

#include <numbers>

// Campaign level order
constexpr std::array<std::string_view, 25> levelNames {
	// tutorial
	"Darkspore_Tutorial_cryos_1_v2",

	// 1-1 to 1-4
	"zelems_1",
	"zelems_3",
	"nocturna_4",
	"nocturna_1",

	// 2-1 to 2-4
	"verdanth_1",
	"verdanth_3",
	"zelems_2",
	"zelems_4",

	// 3-1 to 3-4
	"cryos_4",
	"cryos_3",
	"verdanth_2",
	"verdanth_4",

	// 4-1 to 4-4
	"infinity_2",
	"infinity_3",
	"cryos_1",
	"cryos_2",

	// 5-1 to 5-4
	"nocturna_3",
	"nocturna_2",
	"infinity_1",
	"infinity_4",

	// 6-1 to 6-4
	"scaldron_1",
	"scaldron_2",
	"scaldron_3",
	"scaldron_4"
};

// RakNet
namespace RakNet {
	// helpers
	BitSize_t ReallocateStream(RakNet::BitStream& stream, BitSize_t sizeInBits) {
		const auto writeOffset = stream.GetWriteOffset();
		if ((writeOffset & 7) > 0) {
			printf("ReallocateStream: not full bytes");
		}
		stream.AddBitsAndReallocate(sizeInBits);
		return writeOffset;
	}

	// cAIDirector
	void cAIDirector::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x4D0);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x00D));
		Write<bool>(stream, mbBossSpawned);
		Write<bool>(stream, mbBossHorde);
		Write<bool>(stream, mbCaptainSpawned);
		Write<bool>(stream, mbBossComplete);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x014));
		Write<tObjID>(stream, mBossId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x47C));
		Write<int32_t>(stream, mActiveHordeWaves);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x48C));
		Write<bool>(stream, mbHordeSpawned);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cAIDirector::WriteReflection(BitStream& stream) const {
		reflection_serializer<7> reflector(stream);
		reflector.begin();
		reflector.write<0>(mbBossSpawned);
		reflector.write<1>(mbBossHorde);
		reflector.write<2>(mbCaptainSpawned);
		reflector.write<3>(mbBossComplete);
		reflector.write<4>(mbHordeSpawned);
		reflector.write<5>(mBossId);
		reflector.write<6>(mActiveHordeWaves);
		reflector.end();
	}

	// labsCharacter
	/*
		memory layout (hex)
			0 : unk
			8 : assetID
			10 : version
			B4 : nounDef

			B8 - 1DC : mAttribute (0x4A)
			1E4 : mAttribute (0x01)
			1EC - 244 : mAttribute (0x17)
			24C - 27C : mAttribute (0x0D)

			3B8 : mCreatureType

			3C0 : mDeployCooldownMs
			3C8 : mAbilityPoints
			3CC : mAbilityRanks

			3F0 : mHealthPoints
			3F4 : mMaxHealthPoints
			3F8 : mManaPoints
			3FC : mMaxManaPoints
			400 : mGearScore
			404 : mGearScoreFlattened
	*/

	labsCharacter::labsCharacter() {
		partsAttributes[Attribute::Strength] = 1.f;
		partsAttributes[Attribute::Dexterity] = 1.f;
		partsAttributes[Attribute::Mind] = 1.f;
		partsAttributes[Attribute::MaxHealthIncrease] = 0.f;
		partsAttributes[Attribute::MaxHealth] = 0.f;
		partsAttributes[Attribute::MaxMana] = 0.f;
		partsAttributes[Attribute::DamageReduction] = 0.f;
		partsAttributes[Attribute::PhysicalDefense] = 0.f;
		partsAttributes[Attribute::PhysicalDamageReduction] = 0.f;
		partsAttributes[Attribute::EnergyDefense] = 0.f;
		partsAttributes[Attribute::CriticalRating] = 0.f;
		partsAttributes[Attribute::NonCombatSpeed] = 10.f;
		partsAttributes[Attribute::CombatSpeed] = 10.f;
		partsAttributes[Attribute::DamageBuff] = 0.f;
		partsAttributes[Attribute::Silence] = 0.f;
		partsAttributes[Attribute::Immobilized] = 0.f;
		partsAttributes[Attribute::DefenseBoostBasicDamage] = 0.f;
		partsAttributes[Attribute::PhysicalDamageIncrease] = 0.f;
		partsAttributes[Attribute::PhysicalDamageIncreaseFlat] = 0.f;
		partsAttributes[Attribute::AutoCrit] = 0.f;
		partsAttributes[Attribute::BehindDirectDamageIncrease] = 0.f;
		partsAttributes[Attribute::BehindOrSideDirectDamageIncrease] = 0.f;
		partsAttributes[Attribute::CriticalDamageIncrease] = 0.f;
		partsAttributes[Attribute::AttackSpeedScale] = 1.f;
		partsAttributes[Attribute::CooldownScale] = 1.f;
		partsAttributes[Attribute::Frozen] = 0.f;
		partsAttributes[Attribute::ProjectileSpeedIncrease] = 0.f;
		partsAttributes[Attribute::AoEResistance] = 0.f;
		partsAttributes[Attribute::EnergyDamageBuff] = 0.f;
		partsAttributes[Attribute::Intangible] = 0.f;
		partsAttributes[Attribute::HealingReduction] = 0.f;
		partsAttributes[Attribute::EnergyDamageIncrease] = 0.f;
		partsAttributes[Attribute::EnergyDamageIncreaseFlat] = 0.f;
		partsAttributes[Attribute::Immune] = 0.f;
		partsAttributes[Attribute::StealthDetection] = 0.f;
		partsAttributes[Attribute::LifeSteal] = 0.f;
		partsAttributes[Attribute::RejectModifier] = 0.f;
		partsAttributes[Attribute::AoEDamage] = 0.f;
		partsAttributes[Attribute::TechnologyTypeDamage] = 0.f;
		partsAttributes[Attribute::SpacetimeTypeDamage] = 0.f;
		partsAttributes[Attribute::LifeTypeDamage] = 0.f;
		partsAttributes[Attribute::ElementsTypeDamage] = 0.f;
		partsAttributes[Attribute::SupernaturalTypeDamage] = 0.f;
		partsAttributes[Attribute::TechnologyTypeResistance] = 0.f;
		partsAttributes[Attribute::SpacetimeTypeResistance] = 0.f;
		partsAttributes[Attribute::LifeTypeResistance] = 0.f;
		partsAttributes[Attribute::ElementsTypeResistance] = 0.f;
		partsAttributes[Attribute::SupernaturalTypeResistance] = 0.f;
		partsAttributes[Attribute::MovementSpeedBuff] = 0.f;
		partsAttributes[Attribute::ImmuneToDebuffs] = 0.f;
		partsAttributes[Attribute::BuffDuration] = 0.f;
		partsAttributes[Attribute::DebuffDuration] = 0.f;
		partsAttributes[Attribute::ManaSteal] = 0.f;
		partsAttributes[Attribute::DebuffDurationIncrease] = 0.f;
		partsAttributes[Attribute::EnergyDamageReduction] = 0.f;
		partsAttributes[Attribute::Incorporeal] = 0.f;
		partsAttributes[Attribute::DoTDamageIncrease] = 0.f;
		partsAttributes[Attribute::MindControlled] = 0.f;
		partsAttributes[Attribute::SwapDisabled] = 0.f;
		partsAttributes[Attribute::ImmuneToRandomTeleport] = 0.f;
		partsAttributes[Attribute::ImmuneToBanish] = 0.f;
		partsAttributes[Attribute::ImmuneToKnockback] = 0.f;
		partsAttributes[Attribute::AoeRadius] = 0.f;
		partsAttributes[Attribute::PetDamage] = 0.f;
		partsAttributes[Attribute::PetHealth] = 0.f;
		partsAttributes[Attribute::CrystalFind] = 0.f;
		partsAttributes[Attribute::DNADropped] = 0.f;
		partsAttributes[Attribute::RangeIncrease] = 0.f;
		partsAttributes[Attribute::OrbEffectiveness] = 0.f;
		partsAttributes[Attribute::OverdriveBuildup] = 0.f;
		partsAttributes[Attribute::OverdriveDuration] = 0.f;
		partsAttributes[Attribute::LootFind] = 0.f;
		partsAttributes[Attribute::Surefooted] = 0.f;
		partsAttributes[Attribute::ImmuneToStunned] = 0.f;
		partsAttributes[Attribute::ImmuneToSleep] = 0.f;
		partsAttributes[Attribute::ImmuneToTerrified] = 0.f;
		partsAttributes[Attribute::ImmuneToSilence] = 0.f;
		partsAttributes[Attribute::ImmuneToCursed] = 0.f;
		partsAttributes[Attribute::ImmuneToPoisonOrDisease] = 0.f;
		partsAttributes[Attribute::ImmuneToBurning] = 0.f;
		partsAttributes[Attribute::ImmuneToRooted] = 0.f;
		partsAttributes[Attribute::ImmuneToSlow] = 0.f;
		partsAttributes[Attribute::ImmuneToPull] = 0.f;
		partsAttributes[Attribute::DoTDamageDoneIncrease] = 0.f;
		partsAttributes[Attribute::AggroIncrease] = 0.f;
		partsAttributes[Attribute::AggroDecrease] = 0.f;
		partsAttributes[Attribute::PhysicalDamageDoneIncrease] = 0.f;
		partsAttributes[Attribute::PhysicalDamageDoneByAbilityIncrease] = 0.f;
		partsAttributes[Attribute::EnergyDamageDoneIncrease] = 0.f;
		partsAttributes[Attribute::EnergyDamageDoneByAbilityIncrease] = 0.f;
		partsAttributes[Attribute::ChannelTimeDecrease] = 0.f;
		partsAttributes[Attribute::CrowdControlDurationDecrease] = 0.f;
		partsAttributes[Attribute::DoTDurationDecrease] = 0.f;
		partsAttributes[Attribute::AoEDurationIncrease] = 0.f;
		partsAttributes[Attribute::HealIncrease] = 0.f;
		partsAttributes[Attribute::OnLockdown] = 0.f;
		partsAttributes[Attribute::HoTDoneIncrease] = 0.f;
		partsAttributes[Attribute::ProjectileDamageIncrease] = 0.f;
		partsAttributes[Attribute::DeployBonusInvincibilityTime] = 0.f;
		partsAttributes[Attribute::PhysicalDamageDecreaseFlat] = 0.f;
		partsAttributes[Attribute::EnergyDamageDecreaseFlat] = 0.f;
		partsAttributes[Attribute::MinWeaponDamage] = 1.f;
		partsAttributes[Attribute::MaxWeaponDamage] = 1.f;
		partsAttributes[Attribute::MinWeaponDamagePercent] = 0.f;
		partsAttributes[Attribute::MaxWeaponDamagePercent] = 0.f;
		partsAttributes[Attribute::DirectAttackDamage] = 0.f;
		partsAttributes[Attribute::DirectAttackDamagePercent] = 0.f;
		partsAttributes[Attribute::GetHitAnimDisabled] = 0.f;
		partsAttributes[Attribute::XPBoost] = 0.f;
		partsAttributes[Attribute::InvisibleToSecurityTeleporters] = 1.f;
		partsAttributes[Attribute::BodyScale] = 1.f;
	}

	void labsCharacter::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x620);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x008));
		Write<uint64_t>(stream, assetID);
		Write<int32_t>(stream, version);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B4));
		Write<asset>(stream, nounDef);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B8));
		for (size_t i = 0; i <= 0x49; ++i) {
			Write<float>(stream, partsAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1E4));
		Write<float>(stream, partsAttributes[0x4A]);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1EC));
		for (size_t i = 0x4B; i <= 0x61; ++i) {
			Write<float>(stream, partsAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x24C));
		for (size_t i = 0x62; i <= 0x6E; ++i) {
			Write<float>(stream, partsAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3B8));
		Write<uint32_t>(stream, mCreatureType);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3C0));
		Write<uint64_t>(stream, mDeployCooldownMs);
		Write<uint32_t>(stream, mAbilityPoints);
		for (auto value : mAbilityRanks) {
			Write<uint32_t>(stream, value);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3F0));
		Write<float>(stream, mHealthPoints);
		Write<float>(stream, mMaxHealthPoints);
		Write<float>(stream, mManaPoints);
		Write<float>(stream, mMaxManaPoints);
		Write<float>(stream, mGearScore);
		Write<float>(stream, mGearScoreFlattened);

		stream.SetWriteOffset(writeOffset + size);
	}

	void labsCharacter::WriteReflection(BitStream& stream) const {
		reflection_serializer<124> reflector(stream);
		reflector.begin();
		reflector.write<0>(version);
		reflector.write<1>(nounDef);
		reflector.write<2>(assetID);
		reflector.write<3>(mCreatureType);
		reflector.write<4>(mDeployCooldownMs);
		reflector.write<5>(mAbilityPoints);
		reflector.write<6>(mAbilityRanks);
		reflector.write<7>(mHealthPoints);
		reflector.write<8>(mMaxHealthPoints);
		reflector.write<9>(mManaPoints);
		reflector.write<10>(mMaxManaPoints);
		reflector.write<11>(mGearScore);
		reflector.write<12>(mGearScoreFlattened);
		
		uint8_t field = 13;
		for (auto value : partsAttributes) {
			reflector.write(field++, value);
		}

		reflector.end();
	}

	// labsCrystal
	labsCrystal::labsCrystal(Type type, uint16_t rarity, bool prismatic) {
		level = rarity;

		std::string nounName = "crystal_";
		if (prismatic) {
			nounName += "wild_";
		}

		switch (type) {
			case AoEDamage:
				nounName += "aoedamage";
				break;

			case AttackSpeed:
				nounName += "attackspeed";
				break;

			case BuffDuration:
				nounName += "buffduration";
				break;

			case CCReduction:
				nounName += "ccreduction";
				break;

			case Cooldown:
				nounName += "cooldown";
				break;

			case Crit:
				nounName += "crit";
				break;

			case Damage:
				nounName += "damage";
				break;

			case DamageAura:
				nounName += "damageaura";
				break;

			case DebuffDuration:
				nounName += "debuffduration";
				break;

			case DebuffIncrease:
				nounName += "debuffincrease";
				break;

			case DefenseRating:
				nounName += "defenserating";
				break;

			case DeflectionRating:
				nounName += "deflectionrating";
				break;

			case Dexterity:
				nounName += "dexterity";
				break;

			case DodgeRating:
				nounName += "dodgerating";
				break;

			case Health:
				nounName += "health";
				break;

			case ImmunePoison:
				nounName += "immunepoison";
				break;

			case ImmuneSleep:
				nounName += "immunesleep";
				break;

			case ImmuneSlow:
				nounName += "immuneslow";
				break;

			case ImmuneStun:
				nounName += "immunestun";
				break;

			case Knockback:
				nounName += "knockback";
				break;

			case LifeLeech:
				nounName += "lifeleech";
				break;

			case Mana:
				nounName += "mana";
				break;

			case ManaCostReduction:
				nounName += "manacostreduction";
				break;

			case ManaLeech:
				nounName += "manaleech";
				break;

			case Mind:
				nounName += "mind";
				break;

			case MoveSpeed:
				nounName += "movespeed";
				break;

			case OrbEffectiveness:
				nounName += "orbeffectiveness";
				break;

			case OverdriveBuildup:
				nounName += "overdrivebuildup";
				break;

			case PetDamage:
				nounName += "petdamage";
				break;

			case PetHealth:
				nounName += "pethealth";
				break;

			case ProjectileSpeed:
				nounName += "projectilespeed";
				break;

			case RangeIncrease:
				nounName += "rangeincrease";
				break;

			case Strength:
				nounName += "strength";
				break;

			case Surefooted:
				nounName += "surefooted";
				break;

			case Thorns:
				nounName += "thorns";
				break;
		}

		if (level == 1) {
			nounName += "_rare";
		} else if (level == 2) {
			nounName += "_epic";
		}

		crystalNoun = utils::hash_id(nounName + ".noun");
	}

	uint32_t labsCrystal::GetType() const {
		return 0;
	}

	void labsCrystal::WriteTo(BitStream& stream) const {
		Write<uint32_t>(stream, crystalNoun);
		Write<uint16_t>(stream, level);

		// padding?
		Write<uint32_t>(stream, 0x00);
		Write<uint32_t>(stream, 0x00);
		Write<uint16_t>(stream, 0x00);
	}

	void labsCrystal::WriteReflection(BitStream& stream) const {
		reflection_serializer<2> reflector(stream);
		reflector.begin();
		reflector.write<0>(crystalNoun);
		reflector.write<1>(level);
		reflector.end();
	}

	// labsPlayer
	labsPlayer::labsPlayer() {
		// mReflectionBits.set();
	}

	void labsPlayer::SetUpdateBits(uint8_t bitIndex) {
		mReflectionBits.set(bitIndex);
	}

	void labsPlayer::SetUpdateBits(std::initializer_list<uint8_t>&& bitIndexes) {
		for (uint8_t bitIndex : bitIndexes) {
			mReflectionBits.set(bitIndex);
		}
	}

	void labsPlayer::ResetUpdateBits() {
		mReflectionBits.reset();
	}

	void labsPlayer::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x14B8);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, mbDataSetup);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0004));
		Write(stream, mStatus);
		Write(stream, mStatusProgress);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x000C));
		Write(stream, mCurrentDeckIndex);
		Write(stream, mQueuedDeckIndex);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0018));
		for (const auto& character : mCharacters) {
			character.WriteTo(stream);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1278));
		Write(stream, mPlayerIndex);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1280));
		Write(stream, mPlayerOnlineId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x12DC));
		Write(stream, mTeam);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x12E0));
		Write<tObjID>(stream, mCurrentCreatureId);
		Write(stream, mEnergyPoints);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x12EC));
		Write(stream, mbIsCharged);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x12F0));
		Write(stream, mDNA);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x12F8));
		Write(stream, mAvatarXP);
		Write(stream, mAvatarLevel);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1304));
		Write(stream, mChainProgression);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x133C));
		for (const auto& crystal : mCrystals) {
			crystal.WriteTo(stream);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x13CC));
		for (auto crystalBonus : mCrystalBonuses) {
			Write(stream, crystalBonus);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1408));
		Write(stream, mLockCamera);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x140C));
		Write(stream, mDeckScore);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1414));
		Write(stream, mbLockedOverdrive);
		Write(stream, mbLockedCrystals);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1418));
		Write(stream, mLockedAbilityMin);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1420));
		Write(stream, mLockedDeckIndexMin);

		stream.SetWriteOffset(writeOffset + size);
	}

	void labsPlayer::WriteReflection(BitStream& stream) const {
		reflection_serializer<FieldCount> reflector(stream);
		reflector.begin();
		if (mReflectionBits.any()) {
			if (mReflectionBits.test(0)) { reflector.write<0>(mbDataSetup); }
			if (mReflectionBits.test(1)) { reflector.write<1>(mCurrentDeckIndex); }
			if (mReflectionBits.test(2)) { reflector.write<2>(mQueuedDeckIndex); }
			if (mReflectionBits.test(3)) { reflector.write<3>(mCharacters); }
			if (mReflectionBits.test(4)) { reflector.write<4>(mPlayerIndex); }
			if (mReflectionBits.test(5)) { reflector.write<5>(mTeam); }
			if (mReflectionBits.test(6)) { reflector.write<6>(mPlayerOnlineId); }
			if (mReflectionBits.test(7)) { reflector.write<7>(mStatus); }
			if (mReflectionBits.test(8)) { reflector.write<8>(mStatusProgress); }
			if (mReflectionBits.test(9)) { reflector.write<9>(mCurrentCreatureId); }
			if (mReflectionBits.test(10)) { reflector.write<10>(mEnergyPoints); }
			if (mReflectionBits.test(11)) { reflector.write<11>(mbIsCharged); }
			if (mReflectionBits.test(12)) { reflector.write<12>(mDNA); }
			if (mReflectionBits.test(13)) { reflector.write<13>(mCrystals); }
			if (mReflectionBits.test(14)) { reflector.write<14>(mCrystalBonuses); }
			if (mReflectionBits.test(15)) { reflector.write<15>(mAvatarLevel); }
			if (mReflectionBits.test(16)) { reflector.write<16>(mAvatarXP); }
			if (mReflectionBits.test(17)) { reflector.write<17>(mChainProgression); }
			if (mReflectionBits.test(18)) { reflector.write<18>(mLockCamera); }
			if (mReflectionBits.test(19)) { reflector.write<19>(mbLockedOverdrive); }
			if (mReflectionBits.test(20)) { reflector.write<20>(mbLockedCrystals); }
			if (mReflectionBits.test(21)) { reflector.write<21>(mLockedAbilityMin); }
			if (mReflectionBits.test(22)) { reflector.write<22>(mLockedDeckIndexMin); }
			if (mReflectionBits.test(23)) { reflector.write<23>(mDeckScore); }
		}
		reflector.end();
	}

	// cGameObjectCreateData
	void cGameObjectCreateData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x70);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, noun);
		Write(stream, position);
		Write(stream, rotXDegrees);
		Write(stream, rotYDegrees);
		Write(stream, rotZDegrees);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x20));
		Write(stream, assetId);
		Write(stream, scale);
		Write(stream, team);
		Write(stream, hasCollision);
		Write(stream, playerControlled);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cGameObjectCreateData::WriteReflection(BitStream& stream) const {
		reflection_serializer<10> reflector(stream);
		reflector.begin();
		reflector.write<0>(noun);
		reflector.write<1>(position);
		reflector.write<2>(rotXDegrees);
		reflector.write<3>(rotYDegrees);
		reflector.write<4>(rotZDegrees);
		reflector.write<5>(assetId);
		reflector.write<6>(scale);
		reflector.write<7>(team);
		reflector.write<8>(hasCollision);
		reflector.write<9>(playerControlled);
		reflector.end();
	}

	// sporelabsObject
	/*
		Extra data ([offset] = value)
			0x99 = mark cannot drop loot
			0x9A = mark not worth xp

			-- internal pointers --
			0x2C4 = attribute data (edited via attribute update packet)
			0x298 = locomotion data (edited via locomotion update packet(s))
			0x2CC = combatant data (edited via combatant update packet)
			0x2F0 = interactable data
			0x2B0 = agent blackboard data
			0x2E8 = loot data
			0x2EC = gravity force
			0x2C0 = team data?
	*/
	sporelabsObject::sporelabsObject() :
		mTeam(1), mbPlayerControlled(false),
		mInputSyncStamp(0), mPlayerIdx(0),
		mLinearVelocity(0, 0, 0), mAngularVelocity(0, 0, 0),
		mPosition(0, 0, 0), mOrientation {},
		mScale(1.f), mMarkerScale(1.f),
		mLastAnimationState(0), mLastAnimationPlayTimeMs(0),
		mOverrideMoveIdleAnimationState(0),
		mGraphicsState(0), mGraphicsStateStartTimeMs(0),
		mNewGraphicsStateStartTimeMs(0),
		mVisible(true), mbHasCollision(false),
		mMovementType(0), mDisableRepulsion(false),
		mInteractableState(0), sourceMarkerKey_markerId(0) {}

	void sporelabsObject::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x308);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x010));
		Write<float>(stream, mScale);
		Write<float>(stream, mMarkerScale);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x018));
		Write(stream, mPosition);
		Write(stream, mOrientation);
		Write(stream, mLinearVelocity);
		Write(stream, mAngularVelocity);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x050));
		Write<tObjID>(stream, mOwnerID);
		Write<uint8_t>(stream, mTeam);
		Write<uint8_t>(stream, mPlayerIdx);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x058));
		Write<uint32_t>(stream, mInputSyncStamp);
		Write<bool>(stream, mbPlayerControlled);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x05F));
		Write<bool>(stream, mVisible);
		Write<bool>(stream, mbHasCollision);

		/*
			Movement type
				0 = nothing special?
				1 = projectile? (uses locomotion projectile params)
				2 = projectile again
				3 = projectile again
				4 = bouncing? (uses locomotion lob params)
				5 = uses locomotion offset... somehow
				6 = projectile again
		*/
		Write<uint8_t>(stream, mMovementType); // 0 to 6

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x088));
		Write(stream, sourceMarkerKey_markerId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0AC));
		Write(stream, mLastAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B8));
		Write(stream, mLastAnimationPlayTimeMs);
		Write(stream, mOverrideMoveIdleAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x258));
		Write(stream, mGraphicsState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x260));
		Write(stream, mGraphicsStateStartTimeMs);
		Write(stream, mNewGraphicsStateStartTimeMs);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x284));
		Write(stream, mDisableRepulsion);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x288));
		Write(stream, mInteractableState);

		stream.SetWriteOffset(writeOffset + size);
	}

	void sporelabsObject::WriteReflection(BitStream& stream) const {
		reflection_serializer<23> reflector(stream);
		reflector.begin();
		reflector.write<0>(mTeam);
		reflector.write<1>(mbPlayerControlled);
		reflector.write<2>(mInputSyncStamp);
		reflector.write<3>(mPlayerIdx);
		reflector.write<4>(mLinearVelocity);
		reflector.write<5>(mAngularVelocity);
		reflector.write<6>(mPosition);
		reflector.write<7>(mOrientation);
		reflector.write<8>(mScale);
		reflector.write<9>(mMarkerScale);
		reflector.write<10>(mLastAnimationState);
		reflector.write<11>(mLastAnimationPlayTimeMs);
		reflector.write<12>(mOverrideMoveIdleAnimationState);
		reflector.write<13>(mGraphicsState);
		reflector.write<14>(mGraphicsStateStartTimeMs);
		reflector.write<15>(mNewGraphicsStateStartTimeMs);
		reflector.write<16>(mVisible);
		reflector.write<17>(mbHasCollision);
		reflector.write<18>(mOwnerID);
		reflector.write<19>(mMovementType);
		reflector.write<20>(mDisableRepulsion);
		reflector.write<21>(mInteractableState);
		reflector.write<22>(sourceMarkerKey_markerId);
		reflector.end();
	}

	// cAgentBlackboard
	void cAgentBlackboard::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x598);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x550));
		Write(stream, targetId);
		Write(stream, mbInCombat);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x556));
		Write(stream, mStealthType);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x578));
		Write(stream, mbTargetable);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x57C));
		Write(stream, mNumAttackers);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cAgentBlackboard::WriteReflection(BitStream& stream) const {
		reflection_serializer<5> reflector(stream);
		reflector.begin();
		reflector.write<0>(targetId);
		reflector.write<1>(mbInCombat);
		reflector.write<2>(mStealthType);
		reflector.write<3>(mbTargetable);
		reflector.write<4>(mNumAttackers);
		reflector.end();
	}

	// cLobParams
	void cLobParams::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x54);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x18));
		Write(stream, lobUpDir);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x30));
		Write(stream, bounceNum);
		Write(stream, bounceRestitution);
		Write(stream, groundCollisionOnly);
		Write(stream, stopBounceOnCreatures);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3C));
		Write(stream, planeDir);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x48));
		Write(stream, planeDirLinearParam);
		Write(stream, upLinearParam);
		Write(stream, upQuadraticParam);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cLobParams::WriteReflection(BitStream& stream) const {
		reflection_serializer<9> reflector(stream);
		reflector.begin();
		reflector.write<0>(planeDirLinearParam);
		reflector.write<1>(upLinearParam);
		reflector.write<2>(upQuadraticParam);
		reflector.write<3>(lobUpDir);
		reflector.write<4>(planeDir);
		reflector.write<5>(bounceNum);
		reflector.write<6>(bounceRestitution);
		reflector.write<7>(groundCollisionOnly);
		reflector.write<8>(stopBounceOnCreatures);
		reflector.end();
	}

	// cProjectileParams
	void cProjectileParams::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x3C);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, mSpeed);
		Write(stream, mAcceleration);
		Write(stream, mJinkInfo);
		Write(stream, mRange);
		Write(stream, mSpinRate);
		Write(stream, mDirection);
		Write(stream, mProjectileFlags);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x24));
		Write(stream, mHomingDelay);
		Write(stream, mTurnRate);
		Write(stream, mTurnAcceleration);
		Write(stream, mPiercing);
		Write(stream, mIgnoreGroundCollide);
		Write(stream, mIgnoreCreatureCollide);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x34));
		Write(stream, mEccentricity);
		Write(stream, mCombatantSweepHeight);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cProjectileParams::WriteReflection(BitStream& stream) const {
		reflection_serializer<15> reflector(stream);
		reflector.begin();
		reflector.write<0>(mSpeed);
		reflector.write<1>(mAcceleration);
		reflector.write<2>(mJinkInfo);
		reflector.write<3>(mRange);
		reflector.write<4>(mSpinRate);
		reflector.write<5>(mDirection);
		reflector.write<6>(mProjectileFlags);
		reflector.write<7>(mHomingDelay);
		reflector.write<8>(mTurnRate);
		reflector.write<9>(mTurnAcceleration);
		reflector.write<10>(mEccentricity);
		reflector.write<11>(mPiercing);
		reflector.write<12>(mIgnoreGroundCollide);
		reflector.write<13>(mIgnoreCreatureCollide);
		reflector.write<14>(mCombatantSweepHeight);
		reflector.end();
	}

	// LocomotionData
	void LocomotionData::SetGoalPosition(const glm::vec3& position) {
		constexpr auto vec3_zero = glm::zero<glm::vec3>();

		// reset data
		mTargetObjectId = 0;
		mFacing = vec3_zero;
		mTargetPosition = vec3_zero;
		mExternalLinearVelocity = vec3_zero;

		// set data
		mGoalFlags = 0x001;
		mGoalPosition = position;
		mAllowedStopDistance = 0;
		mDesiredStopDistance = 0;
	}

	void LocomotionData::SetGoalPositionWithDistance(const glm::vec3& position, float distance) {
		SetGoalPosition(position);

		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;
	}
	/*
	void LocomotionData::SetGoalObject(const Game::ObjectPtr& object, float distance) {
		if (!object) {
			return;
		}

		// distance = GetAllowedStoppingDistance(object) + distance;
		SetGoalPosition(data, object->GetPosition());

		// mGoalFlags |= (1 & 0x40);
		// mGoalFlags |= (2 & 0x80);

		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;
	}
	
	void LocomotionData::SetGoalObjectEx(const Game::ObjectPtr& object, float distance) {
		constexpr auto vec3_zero = glm::zero<glm::vec3>();
		if (!object) {
			return;
		}

		// reset data
		mTargetObjectId = 0;
		mFacing = vec3_zero;
		mTargetPosition = vec3_zero;
		mExternalLinearVelocity = vec3_zero;

		// set data
		mGoalFlags = 0x400;
		// mGoalFlags = 1 & 0x040;
		mTargetObjectId = object->GetId();
		mGoalPosition = object->GetPosition();
		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;
	}
	*/
	void LocomotionData::Stop() {
		constexpr auto vec3_zero = glm::zero<glm::vec3>();

		// reset data
		mTargetObjectId = 0;
		mFacing = vec3_zero;

		// set data
		mGoalFlags = 0x020;
	}
	/*
	void LocomotionData::MoveToPointWhileFacingTarget(const glm::vec3& position, const Game::ObjectPtr& object) {
		constexpr auto vec3_zero = glm::zero<glm::vec3>();
		if (!object) {
			return;
		}

		// reset data
		mFacing = vec3_zero;
		mTargetPosition = vec3_zero;
		mExternalLinearVelocity = vec3_zero;

		// set data
		mGoalFlags = 0x001 | 0x100;
		mGoalPosition = position;
		mTargetObjectId = object->GetId();
		mAllowedStopDistance = 0;
		mDesiredStopDistance = 0;

		// extra_func(0x100);
	}
	*/
	void LocomotionData::ApplyExternalVelocity(const glm::vec3& velocity) {
		constexpr auto vec3_zero = glm::zero<glm::vec3>();

		// reset data
		mTargetObjectId = 0;
		mFacing = vec3_zero;

		// set data
		mGoalFlags = 0x008;
		mExternalLinearVelocity = velocity;
	}

	void LocomotionData::ClearExternalVelocity(const glm::vec3& velocity) {
		// set data
		mGoalFlags = 0x020;
		mExternalLinearVelocity = glm::zero<glm::vec3>();
	}

	void LocomotionData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x290);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, reflectedLastUpdate);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x44));
		mProjectileParams.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x84));
		Write(stream, mExpectedGeoCollision);
		Write(stream, mTargetObjectId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x9C));
		Write(stream, mInitialDirection);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0xD8));
		Write(stream, lobStartTime);
		Write(stream, lobPrevSpeedModifier);
		lobParams.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x138));
		Write(stream, mOffset);
		Write(stream, mGoalFlags);
		Write(stream, mGoalPosition);
		Write(stream, mPartialGoalPosition);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x178));
		Write(stream, mFacing);
		Write(stream, mExternalLinearVelocity);
		Write(stream, mExternalForce);
		Write(stream, mAllowedStopDistance);
		Write(stream, mDesiredStopDistance);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1AC));
		Write(stream, mTargetPosition);

		stream.SetWriteOffset(writeOffset + size);
	}

	void LocomotionData::WriteReflection(BitStream& stream) const {
		reflection_serializer<18> reflector(stream);
		reflector.begin();
		reflector.write<0>(lobStartTime);
		reflector.write<1>(lobPrevSpeedModifier);
		reflector.write<2>(lobParams);
		reflector.write<3>(mProjectileParams);
		reflector.write<4>(mGoalFlags);
		reflector.write<5>(mGoalPosition);
		reflector.write<6>(mPartialGoalPosition);
		reflector.write<7>(mFacing);
		reflector.write<8>(mExternalLinearVelocity);
		reflector.write<9>(mExternalForce);
		reflector.write<10>(mAllowedStopDistance);
		reflector.write<11>(mDesiredStopDistance);
		reflector.write<12>(mTargetObjectId);
		reflector.write<13>(mTargetPosition);
		reflector.write<14>(mExpectedGeoCollision);
		reflector.write<15>(mInitialDirection);
		reflector.write<16>(mOffset);
		reflector.write<17>(reflectedLastUpdate);
		reflector.end();
	}

	// TODO: fix all attribute reflection properly
	// cAttributeData in Darkspore
	AttributeData::AttributeData() {
		SetValue(Attribute::NonCombatSpeed, 10.f);
		SetValue(Attribute::CombatSpeed, 10.f);
		SetValue(Attribute::AttackSpeedScale, 1.f);
		SetValue(Attribute::CooldownScale, 1.f);
		/*
		mData[Attribute::Strength] = 1.f;
		mData[Attribute::Dexterity] = 1.f;
		mData[Attribute::Mind] = 1.f;
		mData[Attribute::MaxHealthIncrease] = 0.f;
		mData[Attribute::MaxHealth] = 0.f;
		mData[Attribute::MaxMana] = 0.f;
		mData[Attribute::DamageReduction] = 0.f;
		mData[Attribute::PhysicalDefense] = 0.f;
		mData[Attribute::PhysicalDamageReduction] = 0.f;
		mData[Attribute::EnergyDefense] = 0.f;
		mData[Attribute::CriticalRating] = 0.f;
		mData[Attribute::NonCombatSpeed] = 10.f;
		mData[Attribute::CombatSpeed] = 10.f;
		mData[Attribute::DamageBuff] = 0.f;
		mData[Attribute::Silence] = 0.f;
		mData[Attribute::Immobilized] = 0.f;
		mData[Attribute::DefenseBoostBasicDamage] = 0.f;
		mData[Attribute::PhysicalDamageIncrease] = 0.f;
		mData[Attribute::PhysicalDamageIncreaseFlat] = 0.f;
		mData[Attribute::AutoCrit] = 0.f;
		mData[Attribute::BehindDirectDamageIncrease] = 0.f;
		mData[Attribute::BehindOrSideDirectDamageIncrease] = 0.f;
		mData[Attribute::CriticalDamageIncrease] = 0.f;
		mData[Attribute::AttackSpeedScale] = 1.f;
		mData[Attribute::CooldownScale] = 1.f;
		mData[Attribute::Frozen] = 0.f;
		mData[Attribute::ProjectileSpeedIncrease] = 0.f;
		mData[Attribute::AoEResistance] = 0.f;
		mData[Attribute::EnergyDamageBuff] = 0.f;
		mData[Attribute::Intangible] = 0.f;
		mData[Attribute::HealingReduction] = 0.f;
		mData[Attribute::EnergyDamageIncrease] = 0.f;
		mData[Attribute::EnergyDamageIncreaseFlat] = 0.f;
		mData[Attribute::Immune] = 0.f;
		mData[Attribute::StealthDetection] = 0.f;
		mData[Attribute::LifeSteal] = 0.f;
		mData[Attribute::RejectModifier] = 0.f;
		mData[Attribute::AoEDamage] = 0.f;
		mData[Attribute::TechnologyTypeDamage] = 0.f;
		mData[Attribute::SpacetimeTypeDamage] = 0.f;
		mData[Attribute::LifeTypeDamage] = 0.f;
		mData[Attribute::ElementsTypeDamage] = 0.f;
		mData[Attribute::SupernaturalTypeDamage] = 0.f;
		mData[Attribute::TechnologyTypeResistance] = 0.f;
		mData[Attribute::SpacetimeTypeResistance] = 0.f;
		mData[Attribute::LifeTypeResistance] = 0.f;
		mData[Attribute::ElementsTypeResistance] = 0.f;
		mData[Attribute::SupernaturalTypeResistance] = 0.f;
		mData[Attribute::MovementSpeedBuff] = 0.f;
		mData[Attribute::ImmuneToDebuffs] = 0.f;
		mData[Attribute::BuffDuration] = 0.f;
		mData[Attribute::DebuffDuration] = 0.f;
		mData[Attribute::ManaSteal] = 0.f;
		mData[Attribute::DebuffDurationIncrease] = 0.f;
		mData[Attribute::EnergyDamageReduction] = 0.f;
		mData[Attribute::Incorporeal] = 0.f;
		mData[Attribute::DoTDamageIncrease] = 0.f;
		mData[Attribute::MindControlled] = 0.f;
		mData[Attribute::SwapDisabled] = 0.f;
		mData[Attribute::ImmuneToRandomTeleport] = 0.f;
		mData[Attribute::ImmuneToBanish] = 0.f;
		mData[Attribute::ImmuneToKnockback] = 0.f;
		mData[Attribute::AoeRadius] = 0.f;
		mData[Attribute::PetDamage] = 0.f;
		mData[Attribute::PetHealth] = 0.f;
		mData[Attribute::CrystalFind] = 0.f;
		mData[Attribute::DNADropped] = 0.f;
		mData[Attribute::RangeIncrease] = 0.f;
		mData[Attribute::OrbEffectiveness] = 0.f;
		mData[Attribute::OverdriveBuildup] = 0.f;
		mData[Attribute::OverdriveDuration] = 0.f;
		mData[Attribute::LootFind] = 0.f;
		mData[Attribute::Surefooted] = 0.f;
		mData[Attribute::ImmuneToStunned] = 0.f;
		mData[Attribute::ImmuneToSleep] = 0.f;
		mData[Attribute::ImmuneToTerrified] = 0.f;
		mData[Attribute::ImmuneToSilence] = 0.f;
		mData[Attribute::ImmuneToCursed] = 0.f;
		mData[Attribute::ImmuneToPoisonOrDisease] = 0.f;
		mData[Attribute::ImmuneToBurning] = 0.f;
		mData[Attribute::ImmuneToRooted] = 0.f;
		mData[Attribute::ImmuneToSlow] = 0.f;
		mData[Attribute::ImmuneToPull] = 0.f;
		mData[Attribute::DoTDamageDoneIncrease] = 0.f;
		mData[Attribute::AggroIncrease] = 0.f;
		mData[Attribute::AggroDecrease] = 0.f;
		mData[Attribute::PhysicalDamageDoneIncrease] = 0.f;
		mData[Attribute::PhysicalDamageDoneByAbilityIncrease] = 0.f;
		mData[Attribute::EnergyDamageDoneIncrease] = 0.f;
		mData[Attribute::EnergyDamageDoneByAbilityIncrease] = 0.f;
		mData[Attribute::ChannelTimeDecrease] = 0.f;
		mData[Attribute::CrowdControlDurationDecrease] = 0.f;
		mData[Attribute::DoTDurationDecrease] = 0.f;
		mData[Attribute::AoEDurationIncrease] = 0.f;
		mData[Attribute::HealIncrease] = 0.f;
		mData[Attribute::OnLockdown] = 0.f;
		mData[Attribute::HoTDoneIncrease] = 0.f;
		mData[Attribute::ProjectileDamageIncrease] = 0.f;
		mData[Attribute::DeployBonusInvincibilityTime] = 0.f;
		mData[Attribute::PhysicalDamageDecreaseFlat] = 0.f;
		mData[Attribute::EnergyDamageDecreaseFlat] = 0.f;
		mData[Attribute::MinWeaponDamage] = 1.f;
		mData[Attribute::MaxWeaponDamage] = 1.f;
		mData[Attribute::MinWeaponDamagePercent] = 0.f;
		mData[Attribute::MaxWeaponDamagePercent] = 0.f;
		mData[Attribute::DirectAttackDamage] = 0.f;
		mData[Attribute::DirectAttackDamagePercent] = 0.f;
		mData[Attribute::GetHitAnimDisabled] = 0.f;
		mData[Attribute::XPBoost] = 0.f;
		mData[Attribute::InvisibleToSecurityTeleporters] = 0.f;
		mData[Attribute::BodyScale] = 1.f;
		*/
		mMinWeaponDamage = 0;
		mMaxWeaponDamage = 10;
	}

	float AttributeData::GetValue(uint8_t idx) const {
		// test performance some time
		auto end = mData.end();
		auto it = std::find_if(mData.begin(), end, [idx](const auto& data) { return std::get<0>(data) == idx; });
		if (it != end) {
			return std::get<1>(*it);
		}

		return 0;
	}

	void AttributeData::SetValue(uint8_t idx, float value) {
		auto end = mData.end();
		auto it = std::find_if(mData.begin(), end, [idx](const auto& data) { return std::get<0>(data) == idx; });
		if (it != end) {
			if (value == 0) {
				mData.erase(it);
				mErasedData.push_back(idx);
			} else {
				std::get<1>(*it) = value;
			}
		} else if (value != 0) {
			if (mDataBits.test(idx)) {
				mErasedData.erase(std::find(mErasedData.begin(), mErasedData.end(), idx));
			}

			mData.push_back({ idx, value });
			std::sort(mData.begin(), mData.end(), [](const auto& lhs, const auto& rhs) { return std::get<0>(lhs) < std::get<0>(rhs); });
		}

		mDataBits.set(idx);
	}

	void AttributeData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x634);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x26C));
		for (uint8_t i = 0; i < 0x63; ++i) {
			Write(stream, GetValue(i));
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x400));
		for (uint8_t i = 0x63; i < mData.size(); ++i) {
			Write(stream, GetValue(i));
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x43C));
		Write(stream, mMinWeaponDamage);
		Write(stream, mMaxWeaponDamage);

		stream.SetWriteOffset(writeOffset + size);
	}

	void AttributeData::WriteReflection(BitStream& stream) const {
		reflection_serializer<113> reflector(stream);
		reflector.begin();
		for (const auto& [idx, value] : mData) {
			if (mDataBits.test(idx)) {
				reflector.write(idx, value);
			}
		}

		for (const auto& idx : mErasedData) {
			reflector.write(idx, 0.f);
		}
		reflector.write<111>(mMinWeaponDamage);
		reflector.write<112>(mMaxWeaponDamage);
		reflector.end();
	}

	void AttributeData::ResetReflectionBits() {
		mDataBits.reset();
		mErasedData.clear();
	}

	// cCombatantData
	void cCombatantData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x70);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x40));
		Write(stream, mHitPoints);
		Write(stream, mManaPoints);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cCombatantData::WriteReflection(BitStream& stream) const {
		reflection_serializer<2> reflector(stream);
		reflector.begin();
		reflector.write<0>(mHitPoints);
		reflector.write<1>(mManaPoints);
		reflector.end();
	}

	// cInteractableData
	void cInteractableData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x34);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, mNumTimesUsed);
		Write(stream, mNumUsesAllowed);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x14));
		Write(stream, mInteractableAbility);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cInteractableData::WriteReflection(BitStream& stream) const {
		reflection_serializer<3> reflector(stream);
		reflector.begin();
		reflector.write<0>(mNumTimesUsed);
		reflector.write<1>(mNumUsesAllowed);
		reflector.write<2>(mInteractableAbility);
		reflector.end();
	}

	// cLootData
	void cLootData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x80);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, crystalLevel);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x10));
		Write(stream, mId);
		Write(stream, mRigblockAsset);
		Write(stream, mSuffixAssetId);
		Write(stream, mPrefixAssetId1);
		Write(stream, mPrefixAssetId2);
		Write(stream, mItemLevel);
		Write(stream, mRarity);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x40));
		Write(stream, mLootInstanceId);
		Write(stream, mDNAAmount);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cLootData::WriteReflection(BitStream& stream) const {
		reflection_serializer<10> reflector(stream);
		reflector.begin();
		reflector.write<0>(crystalLevel);
		reflector.write<1>(mId);
		reflector.write<2>(mRigblockAsset);
		reflector.write<3>(mSuffixAssetId);
		reflector.write<4>(mPrefixAssetId1);
		reflector.write<5>(mPrefixAssetId2);
		reflector.write<6>(mItemLevel);
		reflector.write<7>(mRarity);
		reflector.write<8>(mLootInstanceId);
		reflector.write<9>(mDNAAmount);
		reflector.end();
	}

	// GravityForce
	void GravityForce::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x20);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, radius);
		Write(stream, force);
		Write(stream, forceForMover);

		stream.SetWriteOffset(writeOffset + size);
	}

	void GravityForce::WriteReflection(BitStream& stream) const {
		reflection_serializer<3> reflector(stream);
		reflector.begin();
		reflector.write<0>(radius);
		reflector.write<1>(force);
		reflector.write<2>(forceForMover);
		reflector.end();
	}

	// CombatEvent
	void CombatEvent::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x28);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, flags);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x04));
		Write(stream, deltaHealth);
		Write(stream, absorbedAmount);
		Write(stream, targetID);
		Write(stream, sourceID);
		Write(stream, abilityID);
		Write(stream, damageDirection);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x24));
		Write(stream, integerHpChange);

		stream.SetWriteOffset(writeOffset + size);
	}

	void CombatEvent::WriteReflection(BitStream& stream) const {
		reflection_serializer<8> reflector(stream);
		reflector.begin();
		reflector.write<0>(flags);
		reflector.write<1>(deltaHealth);
		reflector.write<2>(absorbedAmount);
		reflector.write<3>(targetID);
		reflector.write<4>(sourceID);
		reflector.write<5>(abilityID);
		reflector.write<6>(damageDirection);
		reflector.write<7>(integerHpChange);
		reflector.end();
	}

	// DifficultyTuning
	void DifficultyTuning::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x48);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x00));
		Write<float>(stream, HealthPercentIncrease[0]);
		Write<float>(stream, HealthPercentIncrease[1]);

		Write<float>(stream, DamagePercentIncrease[0]);
		Write<float>(stream, DamagePercentIncrease[1]);

		Write<float>(stream, ItemLevelRange[0]);
		Write<float>(stream, ItemLevelRange[1]);

		Write(stream, GearScoreRange);
		Write(stream, GearScoreMax);

		Write<int32_t>(stream, ExpectedAvatarLevel[0]);
		Write<int32_t>(stream, ExpectedAvatarLevel[1]);

		Write<float>(stream, RatingConversion[0]);
		Write<float>(stream, RatingConversion[1]);

		Write<float>(stream, StarModeHealthMult);
		Write<float>(stream, StarModeDamageMult);
		Write<float>(stream, StarModeEliteChanceAdd);
		Write<float>(stream, StarModeSuggestedLevelAdd);

		stream.SetWriteOffset(writeOffset + size);
	}

	void DifficultyTuning::WriteReflection(BitStream& stream) const {
		reflection_serializer<11> reflector(stream);
		reflector.begin();
		reflector.write<0>(HealthPercentIncrease);
		reflector.write<1>(DamagePercentIncrease);
		reflector.write<2>(ItemLevelRange);
		reflector.write<3>(GearScoreRange);
		reflector.write<4>(GearScoreMax);
		reflector.write<5>(ExpectedAvatarLevel);
		reflector.write<6>(RatingConversion);
		reflector.write<7>(StarModeHealthMult);
		reflector.write<8>(StarModeDamageMult);
		reflector.write<9>(StarModeEliteChanceAdd);
		reflector.write<10>(StarModeSuggestedLevelAdd);
		reflector.end();
	}

	// ChainVoteData
	ChainVoteData::ChainVoteData() {
		// 1-1
		mEnemyNouns[0] = utils::hash_id("VerdanthBasicMelee.Noun");
		mEnemyNouns[1] = utils::hash_id("ZelemBasicHybrid.Noun");
		mEnemyNouns[2] = utils::hash_id("ZelemBasicPackMelee.Noun");
		mEnemyNouns[3] = utils::hash_id("ZelemBasicRangedHoming.Noun");
		mEnemyNouns[4] = utils::hash_id("ZelemBasicFlyingMelee.Noun");
		mEnemyNouns[5] = utils::hash_id("NomadSnipe.Noun");

		mLevelNouns[0] = utils::hash_id("ZelemBoss.Noun");
		mLevelNouns[1] = utils::hash_id("NomadSpacetimeAgent.Noun");
	}

	float ChainVoteData::GetTimeRemaining() const {
		return mTimeRemaining;
	}

	void ChainVoteData::SetTimeRemaining(float milliseconds) {
		mTimeRemaining = milliseconds;
	}

	std::string ChainVoteData::GetDifficultyName() const {
		if (GetLevelIndex() == 0) {
			return "tutorial";
		}
		return std::to_string(GetMajorDifficulty()) + "-" + std::to_string(GetMinorDifficulty());
	}

	std::string_view ChainVoteData::GetName() const {
		return levelNames[mLevelIndex];
	}

	uint32_t ChainVoteData::GetLevel() const {
		return mLevel;
	}

	void ChainVoteData::SetLevel(uint32_t level) {
		mLevel = level;
		mLevelIndex = 0;
	}

	uint32_t ChainVoteData::GetLevelIndex() const {
		return mLevelIndex;
	}

	void ChainVoteData::SetLevelByIndex(uint32_t index) {
		if (index < levelNames.size()) {
			mLevelIndex = index;
			mLevel = utils::hash_id(GetName().data() + std::string(".Level"));
		}
	}

	void ChainVoteData::SetLevelInstance(uint32_t index) {
		mLevelIndex = index;
	}

	uint32_t ChainVoteData::GetMarkerSet() const {
		std::string markerSet = "levelshop";
		markerSet = "ai_1";

		std::string levelName = GetName().data();
		levelName += "_" + markerSet + ".Markerset";
		return utils::hash_id(levelName);
	}

	uint32_t ChainVoteData::GetMinorDifficulty() const {
		return ((mLevelIndex - 1) % 4) + 1;
	}

	uint32_t ChainVoteData::GetMajorDifficulty() const {
		return ((mLevelIndex - 1) / 4) + 1;
	}

	uint32_t ChainVoteData::GetStarLevel() const {
		return mStarLevel;
	}

	void ChainVoteData::SetStarLevel(uint32_t starLevel) {
		mStarLevel = starLevel;
	}

	bool ChainVoteData::IsCompleted() const {
		return mCompletedLevel;
	}

	void ChainVoteData::SetCompleted(bool completed) {
		mCompletedLevel = completed;
	}

	uint32_t ChainVoteData::GetEnemyNoun(uint32_t index) const {
		if (index < mEnemyNouns.size()) {
			return mEnemyNouns[index];
		}
		return 0;
	}

	void ChainVoteData::SetEnemyNoun(std::string_view nounStr, uint32_t index) {
		if (index < mEnemyNouns.size()) {
			mEnemyNouns[index] = utils::hash_id(nounStr);
		}
	}

	void ChainVoteData::WriteTo(RakNet::BitStream& stream) const {
		// This data is set directly into the state machine?

		/*
			0x00 - 0x13 = memcpy'd in client ()
			0x4D - 0x8E = memcpy'd in client (Maybe DifficultyTuning)
		*/

		/*
			Player progress
				6500 = start mission
				6800 = on chainvote data sent?
		*/

		constexpr auto size = bytes_to_bits(0x151);

		uint32_t minorDifficulty;
		uint32_t majorDifficulty;
		if (mLevelIndex > 0) {
			minorDifficulty = GetMinorDifficulty();
			majorDifficulty = GetMajorDifficulty();
		} else {
			minorDifficulty = 0;
			majorDifficulty = 0;
		}

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x00));
		if (mCompletedLevel) {
			// this is different somehow.
			Write<uint32_t>(stream, minorDifficulty);
			Write<uint32_t>(stream, majorDifficulty);
			Write<uint32_t>(stream, mStarLevel);
			Write<float>(stream, 30 * 60 * 1000.f); // time remaining?
		} else {
			Write<uint32_t>(stream, mLevel);
			Write<uint32_t>(stream, mLevelIndex);
			Write<uint32_t>(stream, mStarLevel);
			Write<float>(stream, 30 * 60 * 1000.f); // time remaining?
		}

		Write<bool>(stream, mCompletedLevel);
		for (const auto enemy : mEnemyNouns) {
			Write<uint32_t>(stream, enemy);
		}

		// TODO: understand this. (same function tests these as the enemy nouns)
		for (const auto data : mLevelNouns) {
			Write<uint32_t>(stream, data);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x39));
		Write<uint32_t>(stream, 0); // party value?, used in sub_5313B0
		Write<uint32_t>(stream, utils::hash_id("fmv_02_zelems.vp6")); // some value
		Write<uint32_t>(stream, utils::hash_id("fmv_02_zelems.vp6")); // some value

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x45));
		Write<uint32_t>(stream, 1); // Unknown so far, arg0 of sub_4E4910 (hash lookup)
		Write<uint32_t>(stream, 0); // tests against "mCompletedLevel" for something, but this is not a boolean.

		if (mCompletedLevel) {
			stream.SetWriteOffset(writeOffset + bytes_to_bits(0x4D));
			Write<uint32_t>(stream, mPlayerAsset);
			Write<uint32_t>(stream, 0x12345678);

#if 1
			for (size_t i = 0; i < 0x39; ++i) {
				Write<uint8_t>(stream, 0xDD);
			}
#else
			WriteDebugData(stream, 0x39);
#endif
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0xD9));
		Write<uint32_t>(stream, 0); // overrides party value
		Write<uint32_t>(stream, utils::hash_id("fmv_03_nocturna.vp6")); // ignored?
		Write<uint32_t>(stream, utils::hash_id("fmv_03_nocturna.vp6")); // ignored?

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0xE9));
		Write<uint32_t>(stream, mLevel); // some hash, finds text in labsplanet.locale

		for (size_t i = 0; i < 0x68; ++i) {
			Write<uint8_t>(stream, 0xDD);
		}

		stream.SetWriteOffset(writeOffset + size);
	}

	// Objective
	void Objective::WriteTo(RakNet::BitStream& stream) const {
		constexpr size_t maxDescriptionLength = 0x30;
		constexpr size_t objectiveSize = sizeof(uint32_t) + sizeof(uint32_t) + maxDescriptionLength;

		const auto writeOffset = ReallocateStream(stream, objectiveSize);

		Write<uint32_t>(stream, id);
		Write<uint32_t>(stream, value);

		size_t length = std::min<size_t>(maxDescriptionLength, description.length());
		size_t padding = maxDescriptionLength - length;

		for (size_t i = 0; i < length; ++i) { Write<char>(stream, description[i]); }
		for (size_t i = 0; i < padding; ++i) { Write<char>(stream, 0x00); }

		stream.SetWriteOffset(writeOffset + objectiveSize);
	}
}
