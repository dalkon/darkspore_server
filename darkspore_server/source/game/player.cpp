
// Include
#include "player.h"
#include "instance.h"

#include "sporenet/user.h"

#include "utils/functions.h"

// Game
namespace Game {
	// Player
	uint32_t Player::GetXPForLevel(uint32_t level) {
		constexpr std::array<uint32_t, 100> xpTable {
			// 1-20
			100, 200, 3000, 6000, 9000,
			12000, 15000, 18000, 21000, 24500,
			28500, 33000, 38000, 43000, 48000,
			53000, 58000, 63000, 68000, 73000,

			// 21-40
			78000, 83000, 88000, 93500, 99500,
			106000, 113000, 120500, 128500, 137000,
			146000, 155500, 165500, 176000, 187000,
			198500, 210500, 223000, 236000, 249500,

			// 41-60
			263500, 278000, 293000, 308500, 324500,
			341000, 358000, 375500, 393500, 412000,
			431000, 451000, 471500, 492500, 514000,
			536000, 558500, 581500, 605000, 629000,

			// 61-80
			653500, 678500, 704500, 731500, 759500,
			788500, 818500, 849500, 881500, 914500,
			948500, 983500, 1019500, 1056500, 1094500,
			1133500, 1173500, 1214500, 1256500, 1299500,

			// 81-100
			1343500, 1388500, 1434500, 1481500, 1529500,
			1578500, 1628500, 1679500, 1731500, 1784500,
			1839500, 1896500, 1955500, 2016500, 2080500,
			2149500, 2225500, 2310500, 2410500, 2510500
		};

		if (level == 0 || level > xpTable.size()) {
			return 0;
		}

		return xpTable[level];
	}

	Player::Player(Instance& instance, const SporeNet::UserPtr& user, uint8_t playerIndex) : mInstance(instance), mUser(user) {
		mData.mPlayerIndex = playerIndex;
		mData.SetUpdateBits(4);
		Setup();
	}

	const SporeNet::UserPtr& Player::GetUser() const {
		return mUser;
	}

	RakNet::labsPlayer& Player::GetData() {
		SetUpdateBits(RakNet::labsPlayerBits::PlayerBits);
		return mData;
	}

	const RakNet::labsPlayer& Player::GetData() const {
		return mData;
	}

	uint8_t Player::GetId() const {
		return mData.mPlayerIndex;
	}

	uint32_t Player::GetAbilityId(uint8_t creatureIndex, uint8_t abilityIndex) const {
		// TODO: rewrite creature stuff
		switch (abilityIndex) {
			case 0: case 2: case 3: {
				if (creatureIndex >= 3) {
					creatureIndex = 0;
				}

				const auto& creatureIds = mSquad->GetCreatureIds();
				const auto& creature = mUser->GetCreatureById(creatureIds[creatureIndex]);
				if (!creature) {
					return 0;
				}

				switch (abilityIndex) {
					case 0: return creature->GetAbility(0);
					case 2: return creature->GetAbility(1);
					case 3: return creature->GetAbility(3);
				}

				break;
			}

			case 6: case 7: case 8: {
				const auto& creatureIds = mSquad->GetCreatureIds();
				const auto& creature = mUser->GetCreatureById(creatureIds[abilityIndex - 6]);
				if (creature) {
					return creature->GetAbility(2);
				}
				break;
			}
		}

		return 0;
	}

	void Player::Setup() {
		SetStatus(0, 0.f);

		mData.mbDataSetup = false;
		mData.mTeam = 1;
		mData.mPlayerOnlineId = mUser->get_id();
		mData.mLockCamera = false;
		mData.mLockedAbilityMin = 0xFF; // num abilities shown in hud (including squad abilities)
		mData.mLockedDeckIndexMin = 0xFF; // num creatures shown in squad
		// mData.mbIsCharged = true;

		const auto& accountData = mUser->get_account();
		mData.mAvatarLevel = accountData.level;
		mData.mDNA = accountData.dna;

		auto prevXP = GetXPForLevel(accountData.level);
		auto nextXP = GetXPForLevel(accountData.level + 1);
		mData.mAvatarXP = (accountData.xp - prevXP) / static_cast<float>(std::max<uint32_t>(1, nextXP - prevXP));

		mData.SetUpdateBits({
			0, 5, 6, 12,
			15, 16, 18, 21, 22
		});
	}

	void Player::GetStatus(uint32_t& status, float& progress) const {
		status = mData.mStatus;
		progress = mData.mStatusProgress;
	}

	void Player::SetStatus(uint32_t status, float progress) {
		mData.mStatus = status;
		mData.mStatusProgress = progress;
		mData.SetUpdateBits({ RakNet::labsPlayer::Status, RakNet::labsPlayer::StatusProgress });
		SetUpdateBits(RakNet::labsPlayerBits::PlayerBits);
	}

	ObjectPtr Player::GetCharacterObject(uint32_t index) const {
		if (index >= mCharacterObjects.size()) {
			return nullptr;
		}
		return mCharacterObjects[index];
	}

	SporeNet::SquadPtr Player::GetSquad() const {
		return mSquad;
	}

	void Player::SetSquad(const SporeNet::SquadPtr& squad) {
		mSquad = squad;

		mData.mCurrentDeckIndex = 0;
		mData.mQueuedDeckIndex = 0;

		if (mSquad) {
			// mData.mDeckScore = mSquad->GetScore();

			uint32_t creatureIndex = 0;
			for (uint32_t creatureId : squad->GetCreatureIds()) {
				RakNet::labsCharacter character;

				// TODO: save/calculate health and "power"
				character.mHealthPoints = 50.f * (creatureIndex + 1);
				character.mMaxHealthPoints = 200.0f;
				character.mManaPoints = 25.f * (creatureIndex + 1);
				character.mMaxManaPoints = 200.0f;

				const auto& creature = mUser->GetCreatureById(creatureId);
				if (creature) {
					character.nounDef = creature->GetNoun();
					character.version = creature->GetVersion();
					character.mCreatureType = static_cast<uint32_t>(creature->GetType());
					character.mGearScore = creature->GetGearScore();
					character.mGearScoreFlattened = creature->GetGearScoreFlattened();
				} else {
					character.nounDef = 0;
					character.version = 0;
					character.mCreatureType = static_cast<uint32_t>(SporeNet::CreatureType::Unknown);
					character.mGearScore = 0.f;
					character.mGearScoreFlattened = 0.f;
				}

				SetCharacter(std::move(character), creatureIndex);
				creatureIndex++;
			}
		} else {
			mData.mDeckScore = 0;
		}

		mData.SetUpdateBits({ 1, 2, 23 });
		SetUpdateBits(RakNet::labsPlayerBits::PlayerBits);
	}

	void Player::SetCharacter(RakNet::labsCharacter&& character, uint32_t index) {
		auto& characters = mData.mCharacters;
		if (index >= characters.size()) {
			return;
		}

		auto& characterRef = characters[index];
		characterRef = std::move(character);

		auto& characterObject = mCharacterObjects[index];
		if (characterRef.nounDef != 0) {
			characterObject = mInstance.GetObjectManager().Create(characterRef.nounDef);
		} else {
			characterObject = nullptr;
		}

		if (characterObject) {
			characterRef.assetID = characterObject->GetId();
			characterRef.mAbilityPoints = 10;
			characterRef.mAbilityRanks.fill(1);

			auto& characterObjectData = characterObject->GetData();
			characterObjectData.mOwnerID = 0;
			characterObjectData.mPlayerIdx = mData.mPlayerIndex;
			characterObjectData.mTeam = mData.mTeam;
			characterObjectData.mbHasCollision = true;
			characterObjectData.mbPlayerControlled = true;

			if (mData.mCurrentCreatureId == 0) {
				mData.mCurrentCreatureId = 0;
			}
		} else {
			characterRef.assetID = 0;
		}

		mData.SetUpdateBits(RakNet::labsPlayer::Characters);
		SetUpdateBits(RakNet::labsPlayerBits::CharacterBits << index);
	}

	const RakNet::labsCrystal& Player::GetCrystal(uint32_t index) const {
		const auto& crystals = mData.mCrystals;
		if (crystals.size() < index) {
			static RakNet::labsCrystal defaultCrystal;
			return defaultCrystal;
		}
		return crystals[index];
	}

	void Player::SetCrystal(RakNet::labsCrystal&& crystal, uint32_t index) {
		auto& crystals = mData.mCrystals;
		if (index < crystals.size()) {
			crystals[index] = std::move(crystal);

			mData.SetUpdateBits(13);
			SetUpdateBits(RakNet::labsPlayerBits::CrystalBits << index);

			UpdateCrystalBonuses();
		}
	}

	uint16_t Player::GetUpdateBits() const {
		return mUpdateBits;
	}

	void Player::SetUpdateBits(uint16_t bits) {
		mUpdateBits |= bits;
	}

	void Player::ResetUpdateBits() {
		mUpdateBits = 0;
	}

	void Player::UpdateCrystalBonuses() {
		// TODO: match catalyst colors

		const auto& crystals = mData.mCrystals;
		auto& crystalBonuses = mData.mCrystalBonuses;

		constexpr size_t gridSize = 3;
		for (size_t i = 0; i < gridSize; ++i) {
			size_t index = i * gridSize;
			
			auto horizontalCrystals =
				(crystals[index + 0].crystalNoun != 0) +
				(crystals[index + 1].crystalNoun != 0) +
				(crystals[index + 2].crystalNoun != 0);

			auto verticalCrystals =
				(crystals[i + (gridSize * 0)].crystalNoun != 0) +
				(crystals[i + (gridSize * 1)].crystalNoun != 0) +
				(crystals[i + (gridSize * 2)].crystalNoun != 0);

			crystalBonuses[i] = horizontalCrystals == 3;
			crystalBonuses[i + gridSize] = verticalCrystals == 3;
		}

		// Diagonals
		crystalBonuses[6] = (
			(crystals[0].crystalNoun != 0) +
			(crystals[4].crystalNoun != 0) +
			(crystals[8].crystalNoun != 0)
		) == 3;

		crystalBonuses[7] = (
			(crystals[2].crystalNoun != 0) +
			(crystals[4].crystalNoun != 0) +
			(crystals[6].crystalNoun != 0)
		) == 3;

		mData.SetUpdateBits(14);
	}
}
