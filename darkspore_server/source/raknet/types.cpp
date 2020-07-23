
// Include
#include "types.h"
#include "utils/functions.h"

#include <numbers>

// Campaign level order (incorrect)
constexpr const char* levelNames[] {
	// tutorial
	"Darkspore_Tutorial_cryos_1_v2"

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

/*
	ServerEvent types (append .ServerEventDef)
		character_filter_exit_bio
		character_filter_exit_necro
		character_filter_exit_spacetime
		character_filter_exit_plasma_electric
		character_filter_exit_cyber
		character_filter_entry_bio
		character_filter_entry_necro
		character_filter_entry_spacetime
		character_filter_entry_plasma_electric
		character_filter_entry_cyber
		character_filter_character_holo_effect_locked
		character_filter_character_holo_effect
		character_filter_base_ring1_effect
		character_filter_base_ring2_effect
		character_lobby_swap_effect
		character_lobby_dias_glow_effect
		dna_pickup
		resurrect_orb_full
		resurrect_orb_pickup
		mana_orb_full
		mana_orb_pickup
		health_orb_full
		health_orb_pickup
		combattext_damage
		combattext_damage_critical
		combattext_enemy_damage
		combattext_enemy_damage_critical
		combattext_absorb
		combattext_absorb_critical
		combattext_enemy_absorb
		combattext_enemy_absorb_critical
		combattext_heal
		combattext_critical_heal
		life_healing_target_effect
		combattext_dodge
		combattext_deflect
		combattext_resist
		combattext_immune
		combattext_resurrected
		combattext_slowed
		combattext_stunned
		combattext_taunted
		combattext_suppressed
		combattext_rooted
		combattext_terrified
		combattext_banished
		common_death_regular_melee_effect_bio
		common_death_regular_melee_effect_cyber
		common_death_regular_melee_effect_necro
		common_death_regular_melee_effect_plasma
		common_death_regular_melee_effect_spacetime
		common_death_critical_melee_effect_bio
		common_death_critical_melee_effect_cyber
		common_death_critical_melee_effect_necro
		common_death_critical_melee_effect_plasma
		common_death_critical_melee_effect_spacetime
		common_death_regular_melee_effect_bio_noblood
		common_death_regular_melee_effect_cyber_noblood
		common_death_regular_melee_effect_necro_noblood
		common_death_regular_melee_effect_plasma_noblood
		common_death_regular_melee_effect_spacetime_noblood
		common_death_critical_melee_effect_bio_noblood
		common_death_critical_melee_effect_cyber_noblood
		common_death_critical_melee_effect_necro_noblood
		common_death_critical_melee_effect_plasma_noblood
		common_death_critical_melee_effect_spacetime_noblood
		common_death_critical_cyber_effect_bio
		common_death_critical_cyber_effect_cyber
		common_death_critical_cyber_effect_necro
		common_death_critical_cyber_effect_plasma
		common_death_critical_cyber_effect_spacetime
		common_death_critical_cyber_effect_bio_noblood
		common_death_critical_cyber_effect_cyber_noblood
		common_death_critical_cyber_effect_necro_noblood
		common_death_critical_cyber_effect_plasma_noblood
		common_death_critical_cyber_effect_spacetime_noblood
		common_death_critical_plasma_effect_bio
		common_death_critical_plasma_effect_cyber
		common_death_critical_plasma_effect_necro
		common_death_critical_plasma_effect_plasma
		common_death_critical_plasma_effect_spacetime
		common_death_critical_plasma_effect_bio_noblood
		common_death_critical_plasma_effect_cyber_noblood
		common_death_critical_plasma_effect_necro_noblood
		common_death_critical_plasma_effect_plasma_noblood
		common_death_critical_plasma_effect_spacetime_noblood
		common_death_critical_bio_effect_bio
		common_death_critical_bio_effect_cyber
		common_death_critical_bio_effect_necro
		common_death_critical_bio_effect_plasma
		common_death_critical_bio_effect_spacetime
		common_death_critical_bio_effect_bio_noblood
		common_death_critical_bio_effect_cyber_noblood
		common_death_critical_bio_effect_necro_noblood
		common_death_critical_bio_effect_plasma_noblood
		common_death_critical_bio_effect_spacetime_noblood
		common_death_critical_necro_effect_bio
		common_death_critical_necro_effect_cyber
		common_death_critical_necro_effect_necro
		common_death_critical_necro_effect_plasma
		common_death_critical_necro_effect_spacetime
		common_death_critical_necro_effect_bio_noblood
		common_death_critical_necro_effect_cyber_noblood
		common_death_critical_necro_effect_necro_noblood
		common_death_critical_necro_effect_plasma_noblood
		common_death_critical_necro_effect_spacetime_noblood
		common_death_critical_spacetime_effect_bio
		common_death_critical_spacetime_effect_cyber
		common_death_critical_spacetime_effect_necro
		common_death_critical_spacetime_effect_plasma
		common_death_critical_spacetime_effect_spacetime
		common_death_critical_spacetime_effect_bio_noblood
		common_death_critical_spacetime_effect_cyber_noblood
		common_death_critical_spacetime_effect_necro_noblood
		common_death_critical_spacetime_effect_plasma_noblood
		common_death_critical_spacetime_effect_spacetime_noblood
*/

#define DEFINE_HASH(x) constexpr auto x = utils::hash_id(#x)
namespace Hash {
	DEFINE_HASH(cSpaceshipCameraTuning);
	DEFINE_HASH(SpaceshipTuning);
	DEFINE_HASH(cToolPos);
	DEFINE_HASH(cAssetQueryString);
	DEFINE_HASH(cLayerPrefs);
	DEFINE_HASH(EditorPrefs);
	DEFINE_HASH(cCinematicView);
	DEFINE_HASH(Cinematic);
	DEFINE_HASH(PopupTip);
	DEFINE_HASH(UnlockDef);
	DEFINE_HASH(UnlocksTuning);
	DEFINE_HASH(WeaponDef);
	DEFINE_HASH(WeaponTuning);
	DEFINE_HASH(labsCharacter);
	DEFINE_HASH(labsCrystal);
	DEFINE_HASH(labsPlayer);
	DEFINE_HASH(cControllerState);
	DEFINE_HASH(CatalogEntry);
	DEFINE_HASH(Catalog);
	DEFINE_HASH(TestAsset);
	DEFINE_HASH(TestProcessedAsset);
	DEFINE_HASH(cLootData);
	DEFINE_HASH(LootData);
	DEFINE_HASH(LootSuffix);
	DEFINE_HASH(LootPrefix);
	DEFINE_HASH(LootRigblock);
	DEFINE_HASH(LootPreferences);
	DEFINE_HASH(cKeyAsset);
	DEFINE_HASH(PlayerClass);
	DEFINE_HASH(cLongDescription);
	DEFINE_HASH(cEliteAffix);
	DEFINE_HASH(NonPlayerClass);
	DEFINE_HASH(ClassAttributes);
	DEFINE_HASH(CharacterAnimation);
	DEFINE_HASH(CharacterType);
	DEFINE_HASH(ServerEvent);
	DEFINE_HASH(cAudioEventData);
	DEFINE_HASH(cHardpointInfo);
	DEFINE_HASH(cEffectEventData);
	DEFINE_HASH(ServerEventDef);
	DEFINE_HASH(CombatEvent);
	DEFINE_HASH(InteractableDef);
	DEFINE_HASH(cInteractableData);
	DEFINE_HASH(CombatantDef);
	DEFINE_HASH(cCombatantData);
	DEFINE_HASH(cGfxComponentDef);
	DEFINE_HASH(Gfx);
	DEFINE_HASH(cSPBoundingBox);
	DEFINE_HASH(ExtentsCategory);
	DEFINE_HASH(ObjectExtents);
	DEFINE_HASH(cThumbnailCaptureParameters);
	DEFINE_HASH(Noun);
	DEFINE_HASH(DifficultyTuning);
	DEFINE_HASH(cAttributeData);
	DEFINE_HASH(cAgentBlackboard);
	DEFINE_HASH(NavMeshLayer);
	DEFINE_HASH(NavPowerTuning);
	DEFINE_HASH(Markerset);
	DEFINE_HASH(LevelMarkerset);
	DEFINE_HASH(LevelCameraSettings);
	DEFINE_HASH(LevelKey);
	DEFINE_HASH(LevelObjectives);
	DEFINE_HASH(Level);
	DEFINE_HASH(DirectorClass);
	DEFINE_HASH(LevelConfig);
	DEFINE_HASH(DirectorBucket);
	DEFINE_HASH(SectionConfig);
	DEFINE_HASH(SpawnPointDef);
	DEFINE_HASH(SpawnTriggerDef);
	DEFINE_HASH(DirectorTuning);
	DEFINE_HASH(cAIDirector);
	DEFINE_HASH(cMapCameraData);
	DEFINE_HASH(LocomotionTuning);
	DEFINE_HASH(cLobParams);
	DEFINE_HASH(cProjectileParams);
	DEFINE_HASH(cLocomotionData);
	DEFINE_HASH(TriggerVolumeEvents);
	DEFINE_HASH(TriggerVolumeDef);
	DEFINE_HASH(NPCAffix);
	DEFINE_HASH(cAffixDifficultyTuning);
	DEFINE_HASH(EliteNPCGlobals);
	DEFINE_HASH(CrystalDef);
	DEFINE_HASH(CrystalDropDef);
	DEFINE_HASH(CrystalLevel);
	DEFINE_HASH(CrystalTuning);
	DEFINE_HASH(cGameObjectGfxStateData);
	DEFINE_HASH(cGameObjectGfxStates);
	DEFINE_HASH(GameObjectGfxStateTuning);
	DEFINE_HASH(cVolumeDef);
	DEFINE_HASH(cPressureSwitchDef);
	DEFINE_HASH(cSwitchDef);
	DEFINE_HASH(cDoorDef);
	DEFINE_HASH(cNewGfxState);
	DEFINE_HASH(cWaterSimData);
	DEFINE_HASH(cGraphicsData);
	DEFINE_HASH(cGrassData);
	DEFINE_HASH(cSplineCameraData);
	DEFINE_HASH(cAnimatorData);
	DEFINE_HASH(cAnimatedData);
	DEFINE_HASH(cLabsMarker);
	DEFINE_HASH(sporelabsObject);
	DEFINE_HASH(cGameObjectCreateData);
	DEFINE_HASH(cAssetProperty);
	DEFINE_HASH(cAssetPropertyList);
	DEFINE_HASH(ability);
	DEFINE_HASH(AIDefinition);
	DEFINE_HASH(cAINode);
	DEFINE_HASH(Phase);
	DEFINE_HASH(Condition);
	DEFINE_HASH(cGambitDefinition);
	DEFINE_HASH(cAICondition);
	DEFINE_HASH(MagicNumbers);
	DEFINE_HASH(cWaterData);
	DEFINE_HASH(TeleporterDef);
	DEFINE_HASH(SpaceshipSpawnPointDef);
	DEFINE_HASH(SharedComponentData);
	DEFINE_HASH(cPointLightData);
	DEFINE_HASH(cSpotLightData);
	DEFINE_HASH(cLineLightData);
	DEFINE_HASH(cParallelLightData);
	DEFINE_HASH(cSplineCameraNodeBaseData);
	DEFINE_HASH(cSplineCameraNodeData);
	DEFINE_HASH(cOccluderData);
	DEFINE_HASH(EventListenerData);
	DEFINE_HASH(EventListenerDef);
	DEFINE_HASH(cDecalData);
	DEFINE_HASH(cCameraComponentData);
	DEFINE_HASH(cCameraComponent);
	DEFINE_HASH(AudioTriggerDef);
	DEFINE_HASH(objective);
	DEFINE_HASH(affix);
	DEFINE_HASH(AffixTuning);
	DEFINE_HASH(GravityForce);
	DEFINE_HASH(CollisionVolumeDef);
	DEFINE_HASH(ProjectileDef);
	DEFINE_HASH(OrbitDef);
	DEFINE_HASH(TriggerVolumeComponentDef);

	DEFINE_HASH(Bfx);
	DEFINE_HASH(Bin);
}
#undef DEFINE_HASH

#define DEFINE_TYPE_HASH(x) constexpr auto type_##x = utils::hash_id(#x)
namespace Hash {
	DEFINE_TYPE_HASH(int);
}
#undef DEFINE_TYPE_HASH

// reflection_serializer | Fields = max amount of fields
template<uint8_t FieldCount>
class reflection_serializer {
	static_assert(FieldCount > 0x00 && FieldCount <= 0xFF, "Fields must be above 0");

	public:
		reflection_serializer(RakNet::BitStream& stream)
			: mStream(stream), mStartOffset(std::numeric_limits<BitSize_t>::max()), mWriteBits(0) {};

		void begin() {
			if (mStartOffset != std::numeric_limits<BitSize_t>::max()) {
				return;
			}

			mStartOffset = mStream.GetWriteOffset();
			mWriteBits = 0;

			if constexpr (FieldCount <= 8) {
				mStream.SetWriteOffset(mStartOffset + RakNet::bytes_to_bits<BitSize_t>(sizeof(uint8_t)));
			} else if constexpr (FieldCount <= 16) {
				mStream.SetWriteOffset(mStartOffset + RakNet::bytes_to_bits<BitSize_t>(sizeof(uint16_t)));
			}
		}

		void end() {
			if constexpr (FieldCount > 16) {
				Write<uint8_t>(mStream, 0xFF);
			} else {
				auto offset = mStream.GetWriteOffset();
				mStream.SetWriteOffset(mStartOffset);
				if constexpr (FieldCount <= 8) {
					Write<uint8_t>(mStream, static_cast<uint8_t>(mWriteBits));
				} else {
					Write<uint16_t>(mStream, mWriteBits);
				}
				mStream.SetWriteOffset(offset);
			}

			mStartOffset = std::numeric_limits<BitSize_t>::max();
		}

		template<uint8_t Field, typename T, size_t S>
		void write(const std::array<T, S>& value) {
			static_assert(Field >= 0 && Field < FieldCount, "Field must be within range of FieldCount.");
			if constexpr (FieldCount > 16) {
				Write<uint8_t>(mStream, Field);
			} else {
				mWriteBits |= 1 << Field;
			}

			for (const auto& value : value) {
				if constexpr (std::is_class_v<T>) {
					value.WriteTo(mStream);
				} else {
					Write<T>(mStream, value);
				}
			}
		}

		template<uint8_t Field, typename T>
		void write(const T& value) {
			static_assert(Field >= 0 && Field < FieldCount, "Field must be within range of FieldCount.");
			if constexpr (FieldCount > 16) {
				Write<uint8_t>(mStream, Field);
			} else {
				mWriteBits |= 1 << Field;
			}

			if constexpr (std::is_class_v<T>) {
				value.WriteTo(mStream);
			} else {
				Write<T>(mStream, value);
			}
		}

		template<typename T, size_t S>
		void write(uint8_t field, const std::array<T, S>& value) {
			if constexpr (FieldCount > 16) {
				Write<uint8_t>(mStream, field);
			} else {
				mWriteBits |= 1 << field;
			}

			for (const auto& value : value) {
				if constexpr (std::is_class_v<T>) {
					value.WriteTo(mStream);
				} else {
					Write<T>(mStream, value);
				}
			}
		}

		template<typename T>
		void write(uint8_t field, const T& value) {
			if constexpr (FieldCount > 16) {
				Write<uint8_t>(mStream, field);
			} else {
				mWriteBits |= 1 << field;
			}

			if constexpr (std::is_class_v<T>) {
				value.WriteTo(mStream);
			} else {
				Write<T>(mStream, value);
			}
		}

	private:
		RakNet::BitStream& mStream;

		BitSize_t mStartOffset;

		uint16_t mWriteBits;
};

class reflection {
	public:
		template<size_t FieldsInClass, typename Tuple>
		static void write(RakNet::BitStream& stream, const Tuple& tuple) {
			constexpr auto Size = std::tuple_size<Tuple>::value;
			if constexpr (FieldsInClass > 16) {
				write_field_by_field<Tuple, Size>(stream, tuple);
			} else {
				write_fields<FieldsInClass, Tuple, Size>(stream, tuple);
			}
		}
		
		template<size_t FieldsInClass, typename T, typename... Args>
		static void write_some(RakNet::BitStream& stream, Args&&... args) {
			if constexpr (FieldsInClass > 16) {
				write_some_field_by_field(stream, args...);
			} else {
				write_some_fields<FieldsInClass>(stream, args...);
			}
		}
		
	private:
		template<typename T, size_t S>
		static void write_type(RakNet::BitStream& stream, const std::array<T, S>& value) {
			for (const T& arrValue : value) {
				write_type(stream, arrValue);
			}
		}

		template<typename T>
		static void write_type(RakNet::BitStream& stream, T value) {
			if constexpr (std::is_class_v<T>) {
				value.WriteTo(stream);
			} else {
				Write<T>(stream, value);
			}
		}

		template<typename Tuple, size_t Size, size_t Idx = 0>
		static void write_field_by_field(RakNet::BitStream& stream, const Tuple& tuple) {
			if constexpr (Idx >= Size) {
				Write<uint8_t>(stream, 0xFF);
			} else {
				Write<uint8_t>(stream, Idx);
				write_type(stream, std::get<Idx>(tuple));
				write_field_by_field<Tuple, Size, Idx + 1>(stream, tuple);
			}
		}

		template<size_t FieldsInClass, typename Tuple, size_t Size, size_t Idx = 0>
		static void write_fields(RakNet::BitStream& stream, const Tuple& tuple) {
			if constexpr (Idx == 0) {
				if (FieldsInClass > 8) {
					Write<uint16_t>(stream, RakNet::get_bits<uint16_t, Size>());
				} else {
					Write<uint8_t>(stream, RakNet::get_bits<uint8_t, Size>());
				}
			}

			if constexpr (Idx < Size) {
				write_type(stream, std::get<Idx>(tuple));
				write_fields<FieldsInClass, Tuple, Size, Idx + 1>(stream, tuple);
			}
		}

		// write_some_field_by_field
		static void write_some_field_by_field(RakNet::BitStream& stream) {
			Write<uint8_t>(stream, 0xFF);
		}

		template<typename T, typename... Args>
		static void write_some_field_by_field(RakNet::BitStream& stream, uint8_t idx, const T& value, Args&&... args) {
			Write<uint8_t>(stream, idx);
			write_type(stream, value);
			write_some_field_by_field(stream, args...);
		}

		// write_some_fields
		static void internal_write_some_fields(RakNet::BitStream& stream, uint16_t& bits) {}

		template<typename T, typename... Args>
		static void internal_write_some_fields(RakNet::BitStream& stream, uint16_t& bits, uint8_t idx, const T& value, Args&& ... args) {
			bits |= (1 << idx);
			write_type(stream, value);
			internal_write_some_fields(stream, bits, args...);
		}

		template<size_t FieldsInClass, typename... Args>
		static void write_some_fields(RakNet::BitStream& stream, Args&& ... args) {
			size_t streamPosition = stream.GetWriteOffset();
			if constexpr (FieldsInClass > 8) {
				Write<uint16_t>(stream, 0);
			} else {
				Write<uint8_t>(stream, 0);
			}

			uint16_t bits = 0;
			internal_write_some_fields(stream, bits, args...);

			size_t currentPosition = stream.GetWriteOffset();
			stream.SetWriteOffset(streamPosition);
			if constexpr (FieldsInClass > 8) {
				Write<uint16_t>(stream, bits);
			} else {
				Write<uint8_t>(stream, bits);
			}

			stream.SetWriteOffset(currentPosition);
		}
};

// Test classes
namespace Hash {
	constexpr auto Zero = utils::hash_id("0");
	// constexpr auto Noun = utils::hash_id("Noun");
}

namespace RakNet {
	// cSPVector2
	cSPVector2::cSPVector2(float _x, float _y)
		: x(_x), y(_y) {}

	void cSPVector2::ReadFrom(BitStream& stream) {
		Read<float>(stream, x);
		Read<float>(stream, y);
	}

	void cSPVector2::WriteTo(BitStream& stream) const {
		Write<float>(stream, x);
		Write<float>(stream, y);
	}

	// cSPVector3
	cSPVector3::cSPVector3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z) {}

	void cSPVector3::ReadFrom(BitStream& stream) {
		Read<float>(stream, x);
		Read<float>(stream, y);
		Read<float>(stream, z);
	}

	void cSPVector3::WriteTo(BitStream& stream) const {
		Write<float>(stream, x);
		Write<float>(stream, y);
		Write<float>(stream, z);
	}

	// cSPQuaternion
	void cSPQuaternion::ReadFrom(BitStream& stream) {
		Read<float>(stream, x);
		Read<float>(stream, y);
		Read<float>(stream, z);
		Read<float>(stream, w);
	}

	void cSPQuaternion::WriteTo(BitStream& stream) const {
		// do we read/write w, x, y, z?
		Write<float>(stream, x);
		Write<float>(stream, y);
		Write<float>(stream, z);
		Write<float>(stream, w);
	}

	// cAIDirector
	void cAIDirector::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x4D0);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

	void labsCharacter::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x620);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x008));
		Write<uint64_t>(stream, assetID);
		Write<int32_t>(stream, version);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B4));
		Write<asset>(stream, nounDef);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B8));
		for (size_t i = 0; i < 0x4A; ++i) {
			Write<float>(stream, partsAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1E4));
		Write<float>(stream, partsAttributes[0x4B]);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1EC));
		for (size_t i = 0x4C; i < 0x63; ++i) {
			Write<float>(stream, partsAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x24C));
		for (size_t i = 0x64; i < 0x6F; ++i) {
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
		/*
		uint8_t field = 13;
		for (auto value : partsAttributes) {
			reflector.write(field++, value);
		}
		*/
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
		mReflectionBits.set();
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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, noun);
		position.WriteTo(stream);
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
			0x2CC = is dead
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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x010));
		Write<float>(stream, mScale);
		Write<float>(stream, mMarkerScale);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x018));
		mPosition.WriteTo(stream);
		mOrientation.WriteTo(stream);
		mLinearVelocity.WriteTo(stream);
		mAngularVelocity.WriteTo(stream);

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
		Write<uint8_t>(stream, mMovementType);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x088));
		Write(stream, sourceMarkerKey_markerId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0AC));
		Write(stream, mLastAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x0B8));
		Write(stream, mLastAnimationPlayTimeMs);
		Write(stream, mOverrideMoveIdleAnimationState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x258));
		Write(stream, mGraphicsState);
		Write(stream, mGraphicsStateStartTimeMs);
		Write(stream, mNewGraphicsStateStartTimeMs);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x284));
		Write(stream, mDisableRepulsion);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x288));
		Write(stream, mInteractableState);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x2CC));
		Write(stream, false);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x18));
		lobUpDir.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x30));
		Write(stream, bounceNum);
		Write(stream, bounceRestitution);
		Write(stream, groundCollisionOnly);
		Write(stream, stopBounceOnCreatures);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3C));
		planeDir.WriteTo(stream);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, mSpeed);
		Write(stream, mAcceleration);
		Write(stream, mJinkInfo);
		Write(stream, mRange);
		Write(stream, mSpinRate);
		mDirection.WriteTo(stream);
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

	// cLocomotionData
	void cLocomotionData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x290);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, reflectedLastUpdate);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x44));
		mProjectileParams.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x84));
		mExpectedGeoCollision.WriteTo(stream);
		Write(stream, mTargetObjectId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x9C));
		mInitialDirection.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0xD8));
		Write(stream, lobStartTime);
		Write(stream, lobPrevSpeedModifier);
		lobParams.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x138));
		mOffset.WriteTo(stream);
		Write(stream, mGoalFlags);
		mGoalPosition.WriteTo(stream);
		mPartialGoalPosition.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x178));
		mFacing.WriteTo(stream);
		mExternalLinearVelocity.WriteTo(stream);
		mExternalForce.WriteTo(stream);
		Write(stream, mAllowedStopDistance);
		Write(stream, mDesiredStopDistance);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1AC));
		mTargetPosition.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cLocomotionData::WriteReflection(BitStream& stream) const {
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

	// cAttributeData
	void cAttributeData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x634);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x26C));
		for (size_t i = 0; i < 0x63; ++i) {
			Write(stream, mAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x400));
		for (size_t i = 0x63; i < mAttributes.size(); ++i) {
			Write(stream, mAttributes[i]);
		}

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x43C));
		Write(stream, mMinWeaponDamage);
		Write(stream, mMaxWeaponDamage);

		stream.SetWriteOffset(writeOffset + size);
	}

	void cAttributeData::WriteReflection(BitStream& stream) const {
		reflection_serializer<113> reflector(stream);
		reflector.begin();
		for (uint32_t i = 0; i < mAttributes.size(); ++i) {
			reflector.write(i, mAttributes[i]);
		}
		reflector.write<111>(mMinWeaponDamage);
		reflector.write<112>(mMaxWeaponDamage);
		reflector.end();
	}

	// cCombatantData
	void cCombatantData::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x70);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, flags);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x04));
		Write(stream, deltaHealth);
		Write(stream, absorbedAmount);
		Write(stream, targetID);
		Write(stream, sourceID);
		Write(stream, abilityID);
		damageDirection.WriteTo(stream);
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

	// ServerEvent
	void ServerEvent::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x98);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x04));
		Write(stream, simpleSwarmEffectID);
		Write(stream, objectFxIndex);
		Write(stream, bRemove);
		Write(stream, bHardStop);
		Write(stream, bForceAttach);
		Write(stream, bCritical);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x10));
		Write(stream, ServerEventDef);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x18));
		Write(stream, objectId);
		Write(stream, secondaryObjectId);
		Write(stream, attackerId);
		position.WriteTo(stream);
		facing.WriteTo(stream);
		orientation.WriteTo(stream);
		targetPoint.WriteTo(stream);
		Write(stream, textValue);
		Write(stream, clientEventID);
		Write(stream, clientIgnoreFlags);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x68));
		Write(stream, lootReferenceId);
		Write(stream, lootInstanceId);
		Write(stream, lootRigblockId);
		Write(stream, lootSuffixAssetId);
		Write(stream, lootPrefixAssetId1);
		Write(stream, lootPrefixAssetId2);
		Write(stream, lootItemLevel);
		Write(stream, lootRarity);
		Write(stream, lootCreationTime);

		stream.SetWriteOffset(writeOffset + size);
	}

	void ServerEvent::WriteReflection(BitStream& stream) const {
		reflection_serializer<26> reflector(stream);
		reflector.begin();
		reflector.write<0>(simpleSwarmEffectID);
		reflector.write<1>(objectFxIndex);
		reflector.write<2>(bRemove);
		reflector.write<3>(bHardStop);
		reflector.write<4>(bForceAttach);
		reflector.write<5>(bCritical);
		reflector.write<6>(ServerEventDef);
		reflector.write<7>(objectId);
		reflector.write<8>(secondaryObjectId);
		reflector.write<9>(attackerId);
		reflector.write<10>(position);
		reflector.write<11>(facing);
		reflector.write<12>(orientation);
		reflector.write<13>(targetPoint);
		reflector.write<14>(textValue);
		reflector.write<15>(clientEventID);
		reflector.write<16>(clientIgnoreFlags);
		reflector.write<17>(lootReferenceId);
		reflector.write<18>(lootInstanceId);
		reflector.write<19>(lootRigblockId);
		reflector.write<20>(lootSuffixAssetId);
		reflector.write<21>(lootPrefixAssetId1);
		reflector.write<22>(lootPrefixAssetId2);
		reflector.write<23>(lootItemLevel);
		reflector.write<24>(lootRarity);
		reflector.write<25>(lootCreationTime);
		reflector.end();
	}

	// DifficultyTuning
	void DifficultyTuning::WriteTo(BitStream& stream) const {
		constexpr auto size = bytes_to_bits(0x48);

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x00));
		Write<float>(stream, HealthPercentIncrease[0]);
		Write<float>(stream, HealthPercentIncrease[1]);

		Write<float>(stream, DamagePercentIncrease[0]);
		Write<float>(stream, DamagePercentIncrease[1]);

		Write<float>(stream, ItemLevelRange[0]);
		Write<float>(stream, ItemLevelRange[1]);

		GearScoreRange.WriteTo(stream);
		GearScoreMax.WriteTo(stream);

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
		// mEnemyNouns.fill(0);
		// mLevelNouns.fill(0);
		mEnemyNouns.fill(utils::hash_id("nct_minn_su_drainer.noun"));
		mLevelNouns[0] = 0x12345678;
		mLevelNouns[1] = 0xFEDCBA09;
	}

	float ChainVoteData::GetTimeRemaining() const {
		return mTimeRemaining;
	}

	void ChainVoteData::SetTimeRemaining(float milliseconds) {
		mTimeRemaining = milliseconds;
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
		if (index >= std::extent_v<decltype(levelNames)>) {
			return;
		}

		std::string levelName = levelNames[index];
		levelName += ".Level";

		mLevel = utils::hash_id(levelName);
		mLevelIndex = index;
	}

	uint32_t ChainVoteData::GetMarkerSet() const {
		std::string markerSet = "levelshop";
		// std::string markerSet = "default";

		std::string levelName = levelNames[mLevelIndex];
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

		auto writeOffset = stream.GetWriteOffset();
		stream.AddBitsAndReallocate(size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x00));
		if (mCompletedLevel) {
			// this is different somehow.
			Write<uint32_t>(stream, minorDifficulty);
			Write<uint32_t>(stream, majorDifficulty);
			Write<uint32_t>(stream, mStarLevel);
			Write<float>(stream, 30 * 60 * 1000.f); // time remaining?
		} else {
			Write<uint32_t>(stream, mLevel);
			Write<uint32_t>(stream, majorDifficulty);
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
}
