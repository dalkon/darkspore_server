
#ifndef _RAKNET_TYPES_HEADER
#define _RAKNET_TYPES_HEADER

// Include
#include "blaze/types.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <BitStream.h>
#include <vector>
#include <array>
#include <tuple>
#include <bitset>

// TODO: find somewhere to place this
namespace Attribute {
	enum Type : uint8_t {
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
		// ImmuneToShocked,
		ImmuneToSleep,
		// ImmuneToTaunted,
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
		// DistributeDamageAmongSquad,
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
		// DefenseBoostPhysicalDamage,
		Count
	};
}

// RakNet
namespace RakNet {
	// types
	using tObjID = uint32_t;
	using asset = uint32_t;

	// labsPlayerBits
	enum labsPlayerBits {
		// Characters (creatures)
		CharacterBits = 1 << 0,
		CharacterLeft = CharacterBits << 0,
		CharacterCenter = CharacterBits << 1,
		CharacterRight = CharacterBits << 2,
		CharacterMask = CharacterLeft | CharacterCenter | CharacterRight,

		// Crystals (catalysts)
		CrystalBits = 1 << 3,
		CrystalTopLeft = CrystalBits << 0,
		CrystalTopCenter = CrystalBits << 1,
		CrystalTopRight = CrystalBits << 2,
		CrystalMidLeft = CrystalBits << 3,
		CrystalMidCenter = CrystalBits << 4,
		CrystalMidRight = CrystalBits << 5,
		CrystalBottomLeft = CrystalBits << 6,
		CrystalBottomCenter = CrystalBits << 7,
		CrystalBottomRight = CrystalBits << 8,
		CrystalMask = CrystalTopLeft | CrystalTopCenter | CrystalTopRight |
			CrystalMidLeft | CrystalMidCenter | CrystalMidRight |
			CrystalBottomLeft | CrystalBottomCenter | CrystalBottomRight,

		// Player data
		PlayerBits = 1 << 12
	};

	// ObjectiveMedal
	enum class ObjectiveMedal {
		InProgress = 0,
		Failed,
		Bronze,
		Silver,
		Gold,
		Unknown
	};

	//
	template<typename T>
	std::enable_if_t<std::is_integral_v<T>, T> bswap(T t) {
		constexpr auto size = sizeof(T);
		if constexpr (size == 2) {
			t = (t >> 8) |
				(t << 8);
		} else if constexpr (size == 4) {
			t = (t << 24) |
				((t & 0x0000FF00U) << 8) |
				((t & 0x00FF0000U) >> 8) |
				(t >> 24);
		} else if constexpr (size == 8) {
			t = (t << 56) |
				((t & 0x000000000000FF00ULL) << 40) |
				((t & 0x0000000000FF0000ULL) << 24) |
				((t & 0x00000000FF000000ULL) << 8) |
				((t & 0x000000FF00000000ULL) >> 8) |
				((t & 0x0000FF0000000000ULL) >> 24) |
				((t & 0x00FF000000000000ULL) >> 40) |
				(t >> 56);
		}
		return t;
	}

	template<typename T>
	std::enable_if_t<std::is_floating_point_v<T>, T> bswap(T t) {
		T value;

		auto pValue = reinterpret_cast<uint8_t*>(&t);
		auto pNewValue = reinterpret_cast<uint8_t*>(&value);

		constexpr auto size = sizeof(T);
		for (size_t i = 0; i < size; i++) {
			pNewValue[size - 1 - i] = pValue[i];
		}

		return value;
	}

	template<typename T>
	std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, void> Read(BitStream& stream, T& value) {
		if constexpr (std::is_same_v<T, bool>) {
			stream.Read<uint8_t>(reinterpret_cast<uint8_t&>(value));
		} else {
			stream.Read<T>(value);
			value = bswap<T>(value);
		}
	}

	template<typename T>
	std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, void> Write(BitStream& stream, const T& value) {
		if constexpr (std::is_same_v<T, bool>) {
			stream.Write<uint8_t>(value);
		} else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
			using Tu = typename std::make_unsigned_t<T>;
			stream.Write<Tu>(bswap<Tu>(static_cast<Tu>(value)));
		} else {
			stream.Write<T>(bswap<T>(value));
		}
	}

	template<typename T>
	std::enable_if_t<std::is_enum_v<T>, void> Write(BitStream& stream, const T& value) {
		Write(stream, static_cast<std::underlying_type_t<T>>(value));
	}

	template<typename T>
	std::enable_if_t<std::is_same_v<std::remove_cvref_t<T>, uint24_t>, void> Write(BitStream& stream, const T& value) {
		uint8_t* pVal = reinterpret_cast<uint8_t*>(&value.val);
		stream.Write<uint8_t>(pVal[0]);
		stream.Write<uint8_t>(pVal[1]);
		stream.Write<uint8_t>(pVal[2]);
	}

	template<typename T>
	std::enable_if_t<std::is_class_v<T>, void> Read(BitStream& stream, T& value) {
		if constexpr (std::is_same_v<T, glm::vec2>) {
			Read<float>(stream, value.x);
			Read<float>(stream, value.y);
		} else if constexpr (std::is_same_v<T, glm::vec3>) {
			Read<float>(stream, value.x);
			Read<float>(stream, value.y);
			Read<float>(stream, value.z);
		} else if constexpr (std::is_same_v<T, glm::quat>) {
			Read<float>(stream, value.x);
			Read<float>(stream, value.y);
			Read<float>(stream, value.z);
			Read<float>(stream, value.w);
		}
	}

	template<typename T>
	std::enable_if_t<std::is_class_v<T>, void> Write(BitStream& stream, const T& value) {
		if constexpr (std::is_same_v<T, glm::vec2>) {
			Write<float>(stream, value.x);
			Write<float>(stream, value.y);
		} else if constexpr (std::is_same_v<T, glm::vec3>) {
			Write<float>(stream, value.x);
			Write<float>(stream, value.y);
			Write<float>(stream, value.z);
		} else if constexpr (std::is_same_v<T, glm::quat>) {
			Write<float>(stream, value.x);
			Write<float>(stream, value.y);
			Write<float>(stream, value.z);
			Write<float>(stream, value.w);
		}
	}

	template<typename T>
	constexpr std::enable_if_t<std::is_integral_v<T>, T> bits_to_bytes(T bits) {
		return (bits + 7) >> 3;
	}

	template<typename T>
	constexpr std::enable_if_t<std::is_integral_v<T>, T> bytes_to_bits(T bytes) {
		return bytes << 3;
	}

#pragma warning(push)
#pragma warning(disable:4309)
	template<typename T, int Bits>
	constexpr T get_bits() {
		return static_cast<T>((1 << Bits) - 1);
	}
#pragma warning(pop)

	// Reflection stuff
	BitSize_t ReallocateStream(RakNet::BitStream& stream, BitSize_t sizeInBits);

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
					if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::quat>) {
						Write(mStream, value);
					} else if constexpr (std::is_class_v<T>) {
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

				if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::quat>) {
					Write(mStream, value);
				} else if constexpr (std::is_class_v<T>) {
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
					if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::quat>) {
						Write(mStream, value);
					} else if constexpr (std::is_class_v<T>) {
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

				if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::quat>) {
					Write(mStream, value);
				} else if constexpr (std::is_class_v<T>) {
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

	/*
		FYI: All class names are what darkspore calls them, not me.
			The same goes for the variable names, they are stupid, they will change later on.
	*/

	// MovementType
	enum class MovementType : uint8_t {
		Default = 0,
		Pathfinding = Default,
		Projectile,
		Ballistic,
		HomingProjectile,
		Lobbed,
		OrbitOwner,
		None,
		GroundRoll
	};

	// cAIDirector
	struct cAIDirector {
		tObjID mBossId = 0;

		int32_t mActiveHordeWaves = 0;

		bool mbBossSpawned = false;
		bool mbBossHorde = false;
		bool mbCaptainSpawned = false;
		bool mbBossComplete = false;
		bool mbHordeSpawned = false;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// labsCharacter
	class labsCharacter {
		public:
			labsCharacter();

			void WriteTo(BitStream& stream) const;
			void WriteReflection(BitStream& stream) const;
			
		public:
			int32_t version = 0;
			asset nounDef = 0;
			uint64_t assetID = 0;
			uint32_t mCreatureType = 0;
			uint64_t mDeployCooldownMs = 0;
			uint32_t mAbilityPoints = 0;
			std::array<uint32_t, 9> mAbilityRanks;
			float mHealthPoints = 0;
			float mMaxHealthPoints = 0;
			float mManaPoints = 0;
			float mMaxManaPoints = 0;
			float mGearScore = 300.f;
			float mGearScoreFlattened = 300.f;

			// make some sort of map?
			std::array<float, Attribute::Count> partsAttributes { 0.f };
	};

	// labsCrystal
	struct labsCrystal {
		enum Type {
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

		uint32_t crystalNoun = 0;
		uint16_t level = 0;

		labsCrystal() = default;
		labsCrystal(Type type, uint16_t rarity, bool prismatic);

		uint32_t GetType() const;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// labsPlayer
	class labsPlayer {
		public:
			enum {
				Characters = 3,

				Status = 7,
				StatusProgress = 8,

				FieldCount = 24,
			};

			labsPlayer();

			void SetUpdateBits(uint8_t bitIndex);
			void SetUpdateBits(std::initializer_list<uint8_t>&& bitIndexes);
			void ResetUpdateBits();

			void WriteTo(BitStream& stream) const;
			void WriteReflection(BitStream& stream) const;

		public:
			bool mbDataSetup = false; // if true then game thinks your lvl/xp is capped (demo mode?)
			int32_t mCurrentDeckIndex = 0;
			int32_t mQueuedDeckIndex = 0;
			std::array<labsCharacter, 3> mCharacters {};
			uint8_t mPlayerIndex = 0;
			uint8_t mTeam = 0;
			uint64_t mPlayerOnlineId = 0;
			uint32_t mStatus = 0;
			float mStatusProgress = 0.f;
			tObjID mCurrentCreatureId = 0;
			float mEnergyPoints = 0.f;
			bool mbIsCharged = false;
			int32_t mDNA = 0;

			std::array<labsCrystal, 9> mCrystals {};
			std::array<bool, 8> mCrystalBonuses { false };

			uint32_t mAvatarLevel = 0;
			float mAvatarXP = 0.f;
			uint32_t mChainProgression = 0;
			bool mLockCamera = false;
			bool mbLockedOverdrive = false;
			bool mbLockedCrystals = false;
			uint32_t mLockedAbilityMin = 0;
			uint32_t mLockedDeckIndexMin = 0;
			uint32_t mDeckScore = 0;

		private:
			std::bitset<FieldCount> mReflectionBits;
	};

	// cGameObjectCreateData
	struct cGameObjectCreateData {
		uint32_t noun;
		glm::vec3 position;
		float rotXDegrees;
		float rotYDegrees;
		float rotZDegrees;
		uint64_t assetId;
		float scale;
		uint8_t team;
		bool hasCollision;
		bool playerControlled;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cAgentBlackboard
	struct cAgentBlackboard {
		tObjID targetId = 0;
		bool mbInCombat;
		uint8_t mStealthType;
		bool mbTargetable;
		uint32_t mNumAttackers;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cLobParams
	struct cLobParams {
		float planeDirLinearParam;
		float upLinearParam;
		float upQuadraticParam;
		glm::vec3 lobUpDir;
		glm::vec3 planeDir;
		int32_t bounceNum;
		float bounceRestitution;
		bool groundCollisionOnly;
		bool stopBounceOnCreatures;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cProjectileParams
	struct cProjectileParams {
		float mSpeed;
		float mAcceleration;
		uint32_t mJinkInfo;
		float mRange;
		float mSpinRate;
		glm::vec3 mDirection;
		uint8_t mProjectileFlags;
		float mHomingDelay;
		float mTurnRate;
		float mTurnAcceleration;
		float mEccentricity;
		bool mPiercing;
		bool mIgnoreGroundCollide;
		bool mIgnoreCreatureCollide;
		float mCombatantSweepHeight;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cLocomotionData in Darkspore
	class LocomotionData {
		public:
			void SetGoalPosition(const glm::vec3& position);
			void SetGoalPositionWithDistance(const glm::vec3& position, float distance);
			// void SetGoalObject(const Game::ObjectPtr& object, float distance);
			// void SetGoalObjectEx(const Game::ObjectPtr& object, float distance);
			void Stop();
			// void MoveToPointWhileFacingTarget(const glm::vec3& position, const Game::ObjectPtr& object);
			void ApplyExternalVelocity(const glm::vec3& velocity);
			void ClearExternalVelocity(const glm::vec3& velocity);

			void WriteTo(BitStream& stream) const;
			void WriteReflection(BitStream& stream) const;

		public:
			uint64_t lobStartTime;
			float lobPrevSpeedModifier;
			cLobParams lobParams;
			cProjectileParams mProjectileParams;
			uint32_t mGoalFlags;
			glm::vec3 mGoalPosition;
			glm::vec3 mPartialGoalPosition;
			glm::vec3 mFacing;
			glm::vec3 mExternalLinearVelocity;
			glm::vec3 mExternalForce;
			float mAllowedStopDistance;
			float mDesiredStopDistance;
			tObjID mTargetObjectId;
			glm::vec3 mTargetPosition;
			glm::vec3 mExpectedGeoCollision;
			glm::vec3 mInitialDirection;
			glm::vec3 mOffset;
			int32_t reflectedLastUpdate;
	};

	// cAttributeData in Darkspore
	class AttributeData {
		public:
			AttributeData();

			const auto& GetData() const { return mData; }
			const auto& GetErasedData() const { return mErasedData; }
			const auto& GetDataBits() const { return mDataBits; }

			float GetValue(uint8_t idx) const;
			void SetValue(uint8_t idx, float value);

			void WriteTo(BitStream& stream) const;
			void WriteReflection(BitStream& stream) const;

			void ResetReflectionBits();

		private:
			std::vector<std::tuple<uint8_t, float>> mData;
			std::vector<uint8_t> mErasedData;
			std::bitset<Attribute::Count> mDataBits;

			float mMinWeaponDamage;
			float mMaxWeaponDamage;
	};

	// cCombatantData
	struct cCombatantData {
		float mHitPoints = 0;
		float mManaPoints = 0;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cInteractableData
	struct cInteractableData {
		int32_t mNumTimesUsed;
		int32_t mNumUsesAllowed;
		uint32_t mInteractableAbility;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// cLootData
	struct cLootData {
		int32_t crystalLevel;

		// mLootItem
		uint64_t mId;
		asset mRigblockAsset;
		uint32_t mSuffixAssetId;
		uint32_t mPrefixAssetId1;
		uint32_t mPrefixAssetId2;
		int32_t mItemLevel;
		int32_t mRarity;

		uint64_t mLootInstanceId;
		float mDNAAmount;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// sporelabsObject
	struct sporelabsObject {
		uint8_t mTeam;
		bool mbPlayerControlled;
		uint32_t mInputSyncStamp;
		uint8_t mPlayerIdx;
		glm::vec3 mLinearVelocity;
		glm::vec3 mAngularVelocity;
		glm::vec3 mPosition;
		glm::quat mOrientation;
		float mScale;
		float mMarkerScale;
		uint32_t mLastAnimationState;
		uint64_t mLastAnimationPlayTimeMs;
		uint32_t mOverrideMoveIdleAnimationState;
		uint32_t mGraphicsState;
		uint64_t mGraphicsStateStartTimeMs;
		uint64_t mNewGraphicsStateStartTimeMs;
		bool mVisible;
		bool mbHasCollision;
		tObjID mOwnerID = 0;
		uint8_t mMovementType;
		bool mDisableRepulsion;
		uint32_t mInteractableState;
		uint32_t sourceMarkerKey_markerId;

		sporelabsObject();

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// GravityForce
	struct GravityForce {
		float radius;
		float force;
		float forceForMover;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// CombatEvent
	enum class CombatEventFlags : uint32_t {
		None		= 0x000000,
		KillingBlow	= 0x000004,
		Critical	= 0x000008,
		Deflect		= 0x000010,
		Dodge		= 0x000020,
		Immune		= 0x000040,
		Resist		= 0x000080,
		Resurrected	= 0x000100,
		Flag0200	= 0x000200,
		Slowed		= 0x000800,
		Stunned		= 0x001000,
		Taunted		= 0x004000,
		Supressed	= 0x008000,
		Rooted		= 0x010000,
		Terrified	= 0x020000,
		Banished	= 0x040000,
	};

	struct CombatEvent {
		CombatEventFlags flags;
		float deltaHealth;
		float absorbedAmount;
		tObjID targetID;
		tObjID sourceID;
		tObjID abilityID;
		glm::vec3 damageDirection;
		int32_t integerHpChange;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// DifficultyTuning
	struct DifficultyTuning {
		std::array<float, 2> HealthPercentIncrease;
		std::array<float, 2> DamagePercentIncrease;
		std::array<float, 2> ItemLevelRange;
		std::array<float, 2> RatingConversion;

		std::array<int32_t, 2> ExpectedAvatarLevel;

		glm::vec2 GearScoreRange;
		glm::vec2 GearScoreMax;

		float StarModeHealthMult;
		float StarModeDamageMult;
		float StarModeEliteChanceAdd;
		float StarModeSuggestedLevelAdd;

		void WriteTo(BitStream& stream) const;
		void WriteReflection(BitStream& stream) const;
	};

	// ChainVoteData
	class ChainVoteData {
		public:
			ChainVoteData();

			float GetTimeRemaining() const;
			void SetTimeRemaining(float milliseconds);

			std::string GetDifficultyName() const;
			std::string_view GetName() const;

			uint32_t GetLevel() const;
			void SetLevel(uint32_t level);

			uint32_t GetLevelIndex() const;
			void SetLevelByIndex(uint32_t index);
			void SetLevelInstance(uint32_t index);

			uint32_t GetMarkerSet() const;
			uint32_t GetMinorDifficulty() const;
			uint32_t GetMajorDifficulty() const;

			uint32_t GetStarLevel() const;
			void SetStarLevel(uint32_t starLevel);

			bool IsCompleted() const;
			void SetCompleted(bool completed);

			uint32_t GetEnemyNoun(uint32_t index) const;
			void SetEnemyNoun(std::string_view nounStr, uint32_t index);

			void WriteTo(RakNet::BitStream& stream) const;

		private:
			std::array<uint32_t, 6> mEnemyNouns;
			std::array<uint32_t, 2> mLevelNouns;

			float mTimeRemaining = 0.f;

			uint32_t mLevel = 0;
			uint32_t mLevelIndex = 0;
			uint32_t mStarLevel = 0;

			uint32_t mPlayerAsset = 0;

			bool mCompletedLevel = false;
	};

	// GameStateData
	struct GameStateData {
		double var;

		uint32_t state;
		Blaze::GameType type;
	};

	// Objective
	struct Objective {
		std::string description;

		uint32_t id = 0;
		uint32_t value = 0;

		ObjectiveMedal medal = ObjectiveMedal::InProgress;

		void WriteTo(RakNet::BitStream& stream) const;
	};

	// ActionCommand
	struct ActionCommand {
		// TODO: separate this into multiple structs to minimize data use
		uint8_t type;
		uint8_t unk[3];


	};

	// AbilityCommandResponse
	struct AbilityCommandResponse {
		uint64_t cooldown;
		uint64_t timeImmobilized;
		uint32_t abilityId;
		uint32_t userData;
	};

	// CombatData
	struct CombatData {
		glm::vec3 targetPosition;
		glm::vec3 cursorPosition;

		uint32_t targetId;
		uint32_t abilityId;
		uint32_t valueFromActionResponse;
		uint32_t unk[2];
	};
}

#endif
