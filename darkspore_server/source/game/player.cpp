
// Include
#include "player.h"
#include "instance.h"

#include "sporenet/user.h"

#include "utils/functions.h"

/* exp table?
1	100
2	200
3	3000
4	6000
5	9000
6	12000
7	15000
8	18000
9	21000
10	24500
11	28500
12	33000
13	38000
14	43000
15	48000
16	53000
17	58000
18	63000
19	68000
20	73000
21	78000
22	83000
23	88000
24	93500
25	99500
26	106000
27	113000
28	120500
29	128500
30	137000
31	146000
32	155500
33	165500
34	176000
35	187000
36	198500
37	210500
38	223000
39	236000
40	249500
41	263500
42	278000
43	293000
44	308500
45	324500
46	341000
47	358000
48	375500
49	393500
50	412000
51	431000
52	451000
53	471500
54	492500
55	514000
56	536000
57	558500
58	581500
59	605000
60	629000
61	653500
62	678500
63	704500
64	731500
65	759500
66	788500
67	818500
68	849500
69	881500
70	914500
71	948500
72	983500
73	1019500
74	1056500
75	1094500
76	1133500
77	1173500
78	1214500
79	1256500
80	1299500
81	1343500
82	1388500
83	1434500
84	1481500
85	1529500
86	1578500
87	1628500
88	1679500
89	1731500
90	1784500
91	1839500
92	1896500
93	1955500
94	2016500
95	2080500
96	2149500
97	2225500
98	2310500
99	2410500
100	2510500
*/

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

	Player::Player(Instance& instance, uint8_t playerIndex, uint64_t blazeId) : mInstance(instance) {
		mData.mPlayerIndex = playerIndex;
		mData.mPlayerOnlineId = blazeId;
		mData.mTeam = 1;
		SetStatus(0, 0.f);
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

	void Player::Setup(const SporeNet::UserPtr& user) {
		if (!user) {
			return;
		}

		mData.mbDataSetup = false;
		mData.mLockCamera = false;
		mData.mCurrentDeckIndex = 0;
		mData.mQueuedDeckIndex = 0;
		mData.mCurrentCreatureId = 0;

		const auto& accountData = user->get_account();
		mData.mAvatarLevel = accountData.level;
		mData.mDNA = accountData.dna;

		auto prevXP = GetXPForLevel(accountData.level);
		auto nextXP = GetXPForLevel(accountData.level + 1);
		mData.mAvatarXP = (accountData.xp - prevXP) / static_cast<float>(std::max<uint32_t>(1, nextXP - prevXP));
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

	const RakNet::labsCharacter& Player::GetCharacter(uint32_t index) const {
		const auto& characters = mData.mCharacters;
		if (index >= characters.size()) {
			static RakNet::labsCharacter defaultCharacter;
			return defaultCharacter;
		}
		return characters[index];
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
			characterObject = mInstance.GetObjectManager().CreateObject(characterRef.nounDef);
		} else {
			characterObject = nullptr;
		}

		if (characterObject) {
			characterRef.assetID = characterObject->GetId();
			characterRef.mAbilityPoints = 0;
			characterRef.mAbilityRanks.fill(1);
			characterRef.partsAttributes.fill(1.0f);

			auto& characterObjectData = characterObject->GetData();
			// characterObjectData.mOwnerID = 0;
			characterObjectData.mPlayerIdx = mData.mPlayerIndex;
			characterObjectData.mTeam = 1;
			characterObjectData.mbHasCollision = true;
			characterObjectData.mbPlayerControlled = true;
		} else {
			characterRef.assetID = 0;
		}

		// mData.SetUpdateBits(RakNet::labsPlayer::Characters);
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
		const auto& crystals = mData.mCrystals;
		auto& crystalBonuses = mData.mCrystalBonuses;

		constexpr size_t gridSize = 3;
		for (size_t i = 0; i < gridSize; ++i) {
			size_t index = i * gridSize;
			crystalBonuses[i] = (crystals[index + 0].crystalNoun + crystals[index + 1].crystalNoun + crystals[index + 2].crystalNoun) != 0;
		}
	}
}
