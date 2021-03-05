
// Include
#include "noun.h"
#include "config.h"

#include "utils/functions.h"
#include "utils/log.h"

#include <filesystem>

// Game
namespace Game {
	template<typename T>
	void ReadListNode(pugi::xml_node node, std::string_view nodeName, std::vector<T>& list) {
		for (const auto& child : node.child(nodeName.data())) {
			if (!utils::string_iequals(child.name(), "entry")) {
				continue;
			}

			decltype(auto) data = list.emplace_back();
			if constexpr (std::is_same_v<T, std::string>) {
				data = node.text().get();
			} else if constexpr (std::is_class_v<T>) {
				data.Read(child);
			} else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>) {
				std::string_view nodeText = node.text().get();
				data = utils::to_number<T>(nodeText);
			}
		}
	}

	// BoundingBox
	BoundingBox::BoundingBox() {
		mCenter = glm::zero<glm::vec3>();
		mExtent = glm::zero<glm::vec3>();
	}

	BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) {
		constexpr auto half = glm::vec3(0.5f);
		mCenter = (max + min) * half;
		mExtent = (max - min) * half;
	}

	bool BoundingBox::IsPoint() const {
		return mExtent == glm::zero<glm::vec3>();
	}

	bool BoundingBox::Contains(const glm::vec3& point) const {
		auto distance = glm::abs(mCenter - point);
		return glm::all(glm::lessThan(distance, mExtent));
	}

	bool BoundingBox::Contains(const BoundingBox& boundingBox) const {
		auto delta = mExtent - boundingBox.mExtent;
		if (glm::any(glm::lessThan(delta, glm::zero<glm::vec3>()))) {
			return false;
		}
		return Contains(boundingBox.mCenter);
	}

	bool BoundingBox::Intersects(const BoundingBox& boundingBox) const {
		auto distance = glm::abs(mCenter - boundingBox.mCenter);
		auto distanceRequired = mExtent + boundingBox.mExtent;
		return glm::all(glm::lessThanEqual(distance, distanceRequired));
	}

	bool BoundingBox::Touches(const BoundingBox& boundingBox) const {
		auto distance = glm::abs(mCenter - boundingBox.mCenter);
		auto distanceRequired = mExtent + boundingBox.mExtent;

		auto results = glm::equal(distance, distanceRequired);
		return (results.x + results.y + results.z) >= 2;
	}

	void BoundingBox::Transform(const glm::mat4& matrix) {
		glm::vec3 min = GetMin();
		glm::vec3 max = GetMax();

		glm::vec3 xa = glm::vec3(matrix[0] * min.x);
		glm::vec3 xb = glm::vec3(matrix[0] * max.x);
		glm::vec3 xMin = glm::min(xa, xb);
		glm::vec3 xMax = glm::max(xa, xb);

		glm::vec3 ya = glm::vec3(matrix[1] * min.y);
		glm::vec3 yb = glm::vec3(matrix[1] * max.y);
		glm::vec3 yMin = glm::min(ya, yb);
		glm::vec3 yMax = glm::max(ya, yb);

		glm::vec3 za = glm::vec3(matrix[2] * min.z);
		glm::vec3 zb = glm::vec3(matrix[2] * max.z);
		glm::vec3 zMin = glm::min(za, zb);
		glm::vec3 zMax = glm::max(za, zb);

		glm::vec3 newMin = xMin + yMin + zMin + glm::vec3(matrix[3]);
		glm::vec3 newMax = xMax + yMax + zMax + glm::vec3(matrix[3]);

		*this = BoundingBox(newMin, newMax);
	}

	glm::vec3 BoundingBox::GetMin() const {
		return mCenter - mExtent;
	}

	glm::vec3 BoundingBox::GetMax() const {
		return mCenter + mExtent;
	}

	glm::vec3 BoundingBox::GetSize() const {
		return mExtent * 2.0f;
	}

	// ClassAttributes
	void ClassAttributes::Read(pugi::xml_node node) {
		mBaseAttributes[ClassAttribute::Health] = utils::xml_get_text_node<float>(node, "baseHealth");
		mBaseAttributes[ClassAttribute::Mana] = utils::xml_get_text_node<float>(node, "baseMana");
		mBaseAttributes[ClassAttribute::Strength] = utils::xml_get_text_node<float>(node, "baseStrength");
		mBaseAttributes[ClassAttribute::Dexterity] = utils::xml_get_text_node<float>(node, "baseDexterity");
		mBaseAttributes[ClassAttribute::Mind] = utils::xml_get_text_node<float>(node, "baseMind");
		mBaseAttributes[ClassAttribute::PhysicalDefense] = utils::xml_get_text_node<float>(node, "basePhysicalDefense");
		mBaseAttributes[ClassAttribute::MagicalDefense] = utils::xml_get_text_node<float>(node, "baseMagicalDefense");
		mBaseAttributes[ClassAttribute::EnergyDefense] = utils::xml_get_text_node<float>(node, "baseEnergyDefense");
		mBaseAttributes[ClassAttribute::Critical] = utils::xml_get_text_node<float>(node, "baseCritical");
		mBaseAttributes[ClassAttribute::CombatSpeed] = utils::xml_get_text_node<float>(node, "baseCombatSpeed");
		mBaseAttributes[ClassAttribute::NonCombatSpeed] = utils::xml_get_text_node<float>(node, "baseNonCombatSpeed");
		mBaseAttributes[ClassAttribute::StealthDetection] = utils::xml_get_text_node<float>(node, "baseStealthDetection");
		mBaseAttributes[ClassAttribute::MovementSpeedBuff] = utils::xml_get_text_node<float>(node, "baseMovementSpeedBuff");

		mMaxAttributes[ClassAttribute::Health] = utils::xml_get_text_node<float>(node, "maxHealth");
		mMaxAttributes[ClassAttribute::Mana] = utils::xml_get_text_node<float>(node, "maxMana");
		mMaxAttributes[ClassAttribute::Strength] = utils::xml_get_text_node<float>(node, "maxStrength");
		mMaxAttributes[ClassAttribute::Dexterity] = utils::xml_get_text_node<float>(node, "maxDexterity");
		mMaxAttributes[ClassAttribute::Mind] = utils::xml_get_text_node<float>(node, "maxMind");
		mMaxAttributes[ClassAttribute::PhysicalDefense] = utils::xml_get_text_node<float>(node, "maxPhysicalDefense");
		mMaxAttributes[ClassAttribute::MagicalDefense] = utils::xml_get_text_node<float>(node, "maxMagicalDefense");
		mMaxAttributes[ClassAttribute::EnergyDefense] = utils::xml_get_text_node<float>(node, "maxEnergyDefense");
		mMaxAttributes[ClassAttribute::Critical] = utils::xml_get_text_node<float>(node, "maxCritical");
	}

	float ClassAttributes::GetBaseAttribute(ClassAttribute::Key key) const {
		if (key < mBaseAttributes.size()) {
			return mBaseAttributes[key];
		}
		return 0.f;
	}

	float ClassAttributes::GetMaxAttribute(ClassAttribute::Key key) const {
		if (key < mMaxAttributes.size()) {
			return mMaxAttributes[key];
		}
		return 0.f;
	}

	// NpcAffix
	void NpcAffix::Read(pugi::xml_node node) {
		mChildName = utils::xml_get_text_node(node, "mpChildAffix");
		mParentName = utils::xml_get_text_node(node, "mpParentAffix");
		mModifier = utils::xml_get_text_node(node, "modifier");
	}

	const std::shared_ptr<NpcAffix>& NpcAffix::GetChild() const {
		return mChild;
	}

	const std::shared_ptr<NpcAffix>& NpcAffix::GetParent() const {
		return mParent;
	}

	// EliteAffix
	void EliteAffix::Read(pugi::xml_node node) {
		mMinDifficulty = utils::xml_get_text_node<int32_t>(node, "minDifficulty");
		mMaxDifficulty = utils::xml_get_text_node<int32_t>(node, "maxDifficulty");

		auto affixName = utils::xml_get_text_node(node, "mpNPCAffix");
		if (!affixName.empty()) {
			mNpcAffix = NounDatabase::Instance().GetNpcAffix(utils::hash_id(affixName));
		}
	}

	// PlayerClass
	void PlayerClass::Read(pugi::xml_node node) {
		mName = utils::xml_get_text_node(node, "speciesName");
		mEffect = utils::xml_get_text_node(node, "mpClassEffect");

		mAbilities[0] = utils::xml_get_text_node(node, "basicAbility");
		mAbilities[1] = utils::xml_get_text_node(node, "specialAbility1");
		mAbilities[2] = utils::xml_get_text_node(node, "specialAbility2");
		mAbilities[3] = utils::xml_get_text_node(node, "specialAbility3");
		mAbilities[4] = utils::xml_get_text_node(node, "passiveAbility");

		mCreatureType = utils::xml_get_text_node<uint32_t>(node, "creatureType");
		mCreatureClass = utils::xml_get_text_node<uint32_t>(node, "creatureClass");
		mPrimaryAttribute = utils::xml_get_text_node<uint32_t>(node, "primaryAttribute");
		mUnlockLevel = utils::xml_get_text_node<int32_t>(node, "unlockLevel");

		mHomeworld = utils::xml_get_text_node<Homeworld>(node, "homeworld");

		mWeaponMinDamage = utils::xml_get_text_node<float>(node, "weaponMinDamage");
		mWeaponMaxDamage = utils::xml_get_text_node<float>(node, "weaponMaxDamage");

		mNoHands = utils::xml_get_text_node<bool>(node, "noHands");
		mNoFeet = utils::xml_get_text_node<bool>(node, "noFeet");

		auto classAttributesName = utils::xml_get_text_node(node, "mpClassAttributes");
		if (!classAttributesName.empty()) {
			mAttributes = NounDatabase::Instance().GetClassAttributes(utils::hash_id(classAttributesName));
		}
	}

	const std::shared_ptr<ClassAttributes>& PlayerClass::GetAttributes() const {
		return mAttributes;
	}

	// NonPlayerClass
	void NonPlayerClass::Read(pugi::xml_node node) {
		const auto& database = NounDatabase::Instance();
		if (auto nameNode = node.child("name")) {
			mName = utils::xml_get_text_node(nameNode, "str");
		}

		mEffect = utils::xml_get_text_node(node, "mpClassEffect");

		mCreatureType = utils::xml_get_text_node<uint32_t>(node, "creatureType");
		mNpcType = utils::xml_get_text_node<NpcType>(node, "mNPCType");

		mChallengeValue = utils::xml_get_text_node<int32_t>(node, "challengeValue");
		mNpcRank = utils::xml_get_text_node<int32_t>(node, "npcRank");

		mAggroRange = utils::xml_get_text_node<float>(node, "aggroRange");
		mAlertRange = utils::xml_get_text_node<float>(node, "alertRange");
		mDropAggroRange = utils::xml_get_text_node<float>(node, "dropAggroRange");
		mDropDelay = utils::xml_get_text_node<float>(node, "dropDelay");
		mPlayerCountHealthScale = utils::xml_get_text_node<float>(node, "playerCountHealthScale");

		mTargetable = utils::xml_get_text_node<bool>(node, "targetable");
		mIsPet = utils::xml_get_text_node<bool>(node, "playerPet");

		ReadListNode(node, "dropType", mDropTypes);
		ReadListNode(node, "eliteAffix", mEliteAffixes);

		auto classAttributesName = utils::xml_get_text_node(node, "mpClassAttributes");
		if (!classAttributesName.empty()) {
			mAttributes = database.GetClassAttributes(utils::hash_id(classAttributesName));
		}
	}

	const std::shared_ptr<ClassAttributes>& NonPlayerClass::GetAttributes() const {
		return mAttributes;
	}

	bool NonPlayerClass::IsTargetable() const {
		return mTargetable;
	}

	bool NonPlayerClass::IsPet() const {
		return mIsPet;
	}

	/*
		noun->add("gfxStates", nullable_type, std::tuple(0x84, cGameObjectGfxStates));
		noun->add("doorDef", nullable_type, std::tuple(0x88, cDoorDef));
		noun->add("switchDef", nullable_type, std::tuple(0x8C, cSwitchDef));
		noun->add("pressureSwitchDef", nullable_type, std::tuple(0x90, cPressureSwitchDef));
		noun->add("crystalDef", nullable_type, std::tuple(0x94, CrystalDef));
		noun->add("triggerVolume", nullable_type, std::tuple(0x124, TriggerVolumeDef));
		noun->add("projectile", nullable_type, std::tuple(0x128, ProjectileDef));
		noun->add("orbit", nullable_type, std::tuple(0x12C, OrbitDef));
		noun->add("locomotionTuning", nullable_type, std::tuple(0x130, LocomotionTuning));
	*/

	// AINode
	void AINode::Read(pugi::xml_node node) {
		ReadListNode(node, "output", mOutput);

		mPhaseData = utils::xml_get_text_node(node, "mpPhaseData");
		mConditionData = utils::xml_get_text_node(node, "mpConditionData");

		mX = utils::xml_get_text_node<int32_t>(node, "nodeX");
		mY = utils::xml_get_text_node<int32_t>(node, "nodeY");
	}

	// AIDefinition
	void AIDefinition::Read(pugi::xml_node node) {
		ReadListNode(node, "ainode", mNodes);

		mDeathAbility = utils::xml_get_text_node(node, "deathAbility");
		mDeathCondition = utils::xml_get_text_node(node, "deathCondition");
		mFirstAggroAbility = utils::xml_get_text_node(node, "firstAggroAbility");
		mSecondaryFirstAggroAbility = utils::xml_get_text_node(node, "firstAggroAbility2");
		mFirstAlertAbility = utils::xml_get_text_node(node, "firstAlertAbility");
		mSubsequentAggroAbility = utils::xml_get_text_node(node, "subsequentAggroAbility");
		mPassiveAbility = utils::xml_get_text_node(node, "passiveAbility");
		mCombatIdle = utils::xml_get_text_node(node, "combatIdle");
		mSecondaryCombatIdle = utils::xml_get_text_node(node, "combatIdle2");
		mSecondaryCombatIdleCondition = utils::xml_get_text_node(node, "combatIdle2Condition");
		mPassiveIdle = utils::xml_get_text_node(node, "passiveIdle");
		mPreAggroIdle = utils::xml_get_text_node(node, "preAggroIdle");
		mSecondaryPreAggroIdle = utils::xml_get_text_node(node, "preAggroIdle2");
		mTargetTooFar = utils::xml_get_text_node(node, "targetTooFar");

		mAggroType = utils::xml_get_text_node<uint32_t>(node, "aggroType");
		mCombatIdleCooldown = utils::xml_get_text_node<uint32_t>(node, "combatIdleCooldown");
		mSecondaryCombatIdleCooldown = utils::xml_get_text_node<uint32_t>(node, "combatIdle2Cooldown");
		mTargetTooFarCooldown = utils::xml_get_text_node<uint32_t>(node, "targetTooFarCooldown");

		mUseSecondaryStart = utils::xml_get_text_node<float>(node, "useSecondaryStart");

		mFaceTarget = utils::xml_get_text_node<bool>(node, "faceTarget");
		mAlwaysRun = utils::xml_get_text_node<bool>(node, "alwaysRunAI");
		mRandomizeCooldowns = utils::xml_get_text_node<bool>(node, "randomizeCooldowns");
	}

	const std::string& AIDefinition::GetPassiveAbility() const {
		return mPassiveAbility;
	}

	// Noun
	void Noun::Read(pugi::xml_node node) {
		auto get_vec3 = [](pugi::xml_node node) {
			glm::vec3 data;
			data.x = utils::xml_get_text_node<float>(node, "x");
			data.y = utils::xml_get_text_node<float>(node, "y");
			data.z = utils::xml_get_text_node<float>(node, "z");
			return data;
		};

		const auto& database = NounDatabase::Instance();
		if (node.child("nounType")) {
			// Directly parsed from .Noun files
			mType = utils::xml_get_text_node<NounType>(node, "nounType");
			mPresetExtents = utils::xml_get_text_node<PresetExtents>(node, "presetExtents");

			if (auto bboxNode = node.child("bbox")) {
				mBoundingBox = BoundingBox(
					get_vec3(bboxNode.child("min")),
					get_vec3(bboxNode.child("max"))
				);
			}

			auto nonPlayerClassName = utils::xml_get_text_node(node, "npcClassData");
			if (!nonPlayerClassName.empty()) {
				mNpcClassData = database.GetNonPlayerClass(utils::hash_id(nonPlayerClassName));
			}

			auto playerClassName = utils::xml_get_text_node(node, "playerClassData");
			if (!playerClassName.empty()) {
				mPlayerClassData = database.GetPlayerClass(utils::hash_id(playerClassName));
			}

			auto aidefinitionName = utils::xml_get_text_node(node, "aiDefinition");
			if (!aidefinitionName.empty()) {
				mAIDefinition = database.GetAIDefinition(utils::hash_id(aidefinitionName));
			}

			mCharacterAnimationData = utils::xml_get_text_node(node, "characterAnimationData");

			mAssetId = utils::xml_get_text_node<uint64_t>(node, "assetId");
			ReadListNode(node, "eliteAssetIds", mEliteAssetIds);

			mIsFlora = utils::xml_get_text_node<bool>(node, "isFlora");
			mIsMineral = utils::xml_get_text_node<bool>(node, "isMineral");
			mIsCreature = utils::xml_get_text_node<bool>(node, "isCreature");
			mIsPlayer = utils::xml_get_text_node<bool>(node, "isPlayer");
			mIsSpawned = utils::xml_get_text_node<bool>(node, "isSpawned");
		} else {
			// Shortened versions
		}
	}

	const BoundingBox& Noun::GetBoundingBox() const {
		return mBoundingBox;
	}

	const std::shared_ptr<NonPlayerClass>& Noun::GetNonPlayerClassData() const {
		return mNpcClassData;
	}

	const std::shared_ptr<PlayerClass>& Noun::GetPlayerClassData() const {
		return mPlayerClassData;
	}

	const std::shared_ptr<AIDefinition>& Noun::GetAIDefinition() const {
		return mAIDefinition;
	}

	const std::string& Noun::GetName() const {
		return mName;
	}

	uint64_t Noun::GetAssetId() const {
		return mAssetId;
	}

	uint32_t Noun::GetId() const {
		return mId;
	}

	NounType Noun::GetType() const {
		return mType;
	}

	PresetExtents Noun::GetPresetExtents() const {
		return mPresetExtents;
	}

	bool Noun::IsCreature() const {
		return mIsCreature;
	}

	bool Noun::IsPlayer() const {
		return mIsPlayer;
	}

	// NounDatabase
	NounDatabase& NounDatabase::Instance() {
		static NounDatabase instance;
		if (!instance.mLoaded) {
			instance.Load();
		}
		return instance;
	}

	bool NounDatabase::Load() {
		mLoaded = true;
		if (!LoadAIDefinitions()) { return false; }
		if (!LoadClassAttributes()) { return false; }
		if (!LoadNpcAffixes()) { return false; }
		if (!LoadPlayerClasses()) { return false; }
		if (!LoadNonPlayerClasses()) { return false; }
		if (!LoadNouns()) { return false; }
		return true;
	}

	NounPtr NounDatabase::Get(uint32_t id) const {
		if (auto it = mNouns.find(id); it != mNouns.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<NonPlayerClass> NounDatabase::GetNonPlayerClass(uint32_t id) const {
		if (auto it = mNonPlayerClasses.find(id); it != mNonPlayerClasses.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<PlayerClass> NounDatabase::GetPlayerClass(uint32_t id) const {
		if (auto it = mPlayerClasses.find(id); it != mPlayerClasses.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<NpcAffix> NounDatabase::GetNpcAffix(uint32_t id) const {
		if (auto it = mNpcAffixes.find(id); it != mNpcAffixes.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<ClassAttributes> NounDatabase::GetClassAttributes(uint32_t id) const {
		if (auto it = mClassAttributes.find(id); it != mClassAttributes.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<AIDefinition> NounDatabase::GetAIDefinition(uint32_t id) const {
		if (auto it = mAIDefinitions.find(id); it != mAIDefinitions.end()) {
			return it->second;
		}
		return nullptr;
	}

	bool NounDatabase::LoadNouns() {
		std::cout << "Loading nouns...";

		size_t failed = 0;

		const auto& dataPath = "data/noun/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					if (auto rootNode = document.child("noun")) {
						auto noun = std::make_shared<Noun>();
						noun->Read(rootNode);

						auto name = path.stem().string();
						noun->mId = utils::hash_id(name);
						noun->mName = name;

						mNouns.try_emplace(noun->mId, noun);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mNouns.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			std::cout << " done. (loaded: " << mNouns.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}

	bool NounDatabase::LoadNonPlayerClasses() {
		std::cout << "Loading nonplayerclasses...";

		size_t failed = 0;

		const auto& dataPath = "data/nonplayerclass/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto data = std::make_shared<NonPlayerClass>();
					if (auto rootNode = document.child("nonplayerclass")) {
						data->Read(rootNode);

						auto name = path.stem().string();
						data->mId = utils::hash_id(name);
						data->mPath = name;

						mNonPlayerClasses.try_emplace(data->mId, data);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mNonPlayerClasses.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			std::cout << " done. (loaded: " << mNonPlayerClasses.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}

	bool NounDatabase::LoadPlayerClasses() {
		std::cout << "Loading playerclasses...";

		size_t failed = 0;

		const auto& dataPath = "data/playerclass/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto data = std::make_shared<PlayerClass>();
					if (auto rootNode = document.child("playerclass")) {
						data->Read(rootNode);

						auto name = path.stem().string();
						data->mId = utils::hash_id(name);
						data->mPath = name;

						mPlayerClasses.try_emplace(data->mId, data);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mPlayerClasses.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			std::cout << " done. (loaded: " << mPlayerClasses.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}

	bool NounDatabase::LoadNpcAffixes() {
		std::cout << "Loading npcaffixes...";

		size_t failed = 0;

		const auto& dataPath = "data/npcaffix/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto data = std::make_shared<NpcAffix>();
					if (auto rootNode = document.child("npcaffix")) {
						data->Read(rootNode);

						auto name = path.stem().string();
						data->mId = utils::hash_id(name);
						data->mPath = name;

						mNpcAffixes.try_emplace(data->mId, data);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mNpcAffixes.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			for (const auto& [_, affix] : mNpcAffixes) {
				affix->mChild = GetNpcAffix(utils::hash_id(affix->mChildName));
				affix->mParent = GetNpcAffix(utils::hash_id(affix->mParentName));
			}
			std::cout << " done. (loaded: " << mNpcAffixes.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}

	bool NounDatabase::LoadClassAttributes() {
		std::cout << "Loading classattributes...";

		size_t failed = 0;

		const auto& dataPath = "data/classattributes/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto data = std::make_shared<ClassAttributes>();
					if (auto rootNode = document.child("classattributes")) {
						data->Read(rootNode);

						auto name = path.stem().string();
						data->mId = utils::hash_id(name);
						data->mPath = name;

						mClassAttributes.try_emplace(data->mId, data);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mClassAttributes.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			std::cout << " done. (loaded: " << mClassAttributes.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}

	bool NounDatabase::LoadAIDefinitions() {
		std::cout << "Loading aidefinitions...";

		size_t failed = 0;

		const auto& dataPath = "data/aidefinition/";
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".xml") {
				pugi::xml_document document;
				if (auto parse_result = document.load_file(path.c_str())) {
					auto data = std::make_shared<AIDefinition>();
					if (auto rootNode = document.child("aidefinition")) {
						data->Read(rootNode);

						auto name = path.stem().string();
						data->mId = utils::hash_id(name);
						data->mPath = name;

						mAIDefinitions.try_emplace(data->mId, data);
					}
				} else {
					std::cout << "NounDatabase: Could not load '" << path << "'." << std::endl;
					failed++;
				}
			}
		}

		if (mAIDefinitions.empty()) {
			std::cout << " failed." << std::endl;
			return false;
		} else {
			std::cout << " done. (loaded: " << mAIDefinitions.size() << ", failed: " << failed << ")" << std::endl;
			return true;
		}
	}
}
