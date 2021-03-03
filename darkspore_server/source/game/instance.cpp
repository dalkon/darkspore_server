
// Include
#include "instance.h"

#include "objectmanager.h"
#include "lua.h"
#include "serverevent.h"
#include "scheduler.h"

#include "raknet/server.h"
#include "sporenet/instance.h"
#include "sporenet/user.h"
#include "utils/functions.h"

#include <iostream>
#include <array>

/*
	eeh... maybe not

	TODO: Rename Game namespace to SporeNet? Same for RakNet?
		Game -> SporeNet
		RakNet -> SporeNet
*/

// Game
namespace Game {
	// Instance
	InstancePtr Instance::Create(uint32_t id) {
		return InstancePtr(new Instance(id));
	}

	Instance::Instance(uint32_t id) {
		mData.id = id;

		decltype(auto) objective = mObjectives.emplace_back();
		objective.id = utils::hash_id("FinishLevelQuickly");
		objective.value = 0;
		objective.description = "Do some stuff bruh";
	}

	Instance::~Instance() {
		Stop();
	}

	bool Instance::Start() {
		Stop();

		mObjectManager = std::make_unique<ObjectManager>(*this);
		mLua = std::make_unique<Lua>(*this);
		mServer = std::make_unique<RakNet::Server>(*this);

		mServer->start(mData.hostNetwork.exip.port);
		mGameStartTime = utils::get_milliseconds();

		mServer->add_task([this] {
			mLua->Initialize();
		});
		return true;
	}

	void Instance::Stop() {
		for (uint32_t id : mEvents) {
			Scheduler::CancelTask(id);
		}

		mEvents.clear();
		if (mServer && mServer->is_running()) {
			mServer->stop();
			// wait until it stops
		}

		mServer.reset();
		mLua.reset();
		mObjectManager.reset();
	}

	bool Instance::LoadLevel() {
		if (!mLevelLoaded) {
			std::string levelName = mChainData.GetName().data();
			mLevelLoaded = mLevel.Load(mChainData.GetDifficultyName(), levelName);
			if (mLevelLoaded) {
				LevelConfig config;
				if (mChainData.IsCompleted()) {
					config = mLevel.GetConfig();
				} else {
					config = mLevel.GetFirstTimeConfig();
				}

				mChainData.SetEnemyNoun(config.GetMinion(0).GetNounName(), 0);
				mChainData.SetEnemyNoun(config.GetMinion(1).GetNounName(), 1);
				mChainData.SetEnemyNoun(config.GetMinion(2).GetNounName(), 2);
				mChainData.SetEnemyNoun(config.GetSpecial(0).GetNounName(), 3);
				mChainData.SetEnemyNoun(config.GetSpecial(1).GetNounName(), 4);
				mChainData.SetEnemyNoun(config.GetSpecial(2).GetNounName(), 5);

				Markerset markerset;
				if (mLevel.GetMarkerset(levelName + "_design.Markerset", markerset)) {
					const auto& markers = markerset.GetMarkersByType(utils::hash_id("CameraSpawnPoint.Noun"));
					for (const auto& marker : markers) {
						mPlayerSpawnpoints.push_back(marker->GetPosition());
					}
				}
			}
		}

		return mLevelLoaded;
	}

	MarkerPtr Instance::GetMarker(uint32_t id) const {
		auto it = mMarkers.find(id);
		return (it != mMarkers.end()) ? it->second : nullptr;
	}

	PlayerPtr Instance::GetPlayer(int64_t id) const {
		auto it = mPlayers.find(id);
		return (it != mPlayers.end()) ? it->second : nullptr;
	}

	PlayerPtr Instance::GetPlayerByIndex(uint8_t index) const {
		for (const auto& [_, player] : mPlayers) {
			if (player->GetId() == index) {
				return player;
			}
		}
		return nullptr;
	}

	PlayerPtr Instance::AddPlayer(const SporeNet::UserPtr& user, uint8_t index) {
		if (!user) {
			return nullptr;
		}

		auto it = mPlayers.try_emplace(user->get_id());
		if (it.second) {
			it.first->second = std::make_shared<Player>(*this, user, index);
		}

		return it.first->second;
	}

	void Instance::RemovePlayer(int64_t id) {
		mPlayers.erase(id);
	}

	uint32_t Instance::GetId() const {
		return mData.id;
	}

	uint64_t Instance::GetTime() const {
		return mGameTime;
	}

	uint64_t Instance::GetTimeElapsed() const {
		return utils::get_milliseconds() - mGameStartTime;
	}

	ObjectManager& Instance::GetObjectManager() {
		return *mObjectManager;
	}

	const ObjectManager& Instance::GetObjectManager() const {
		return *mObjectManager;
	}

	Lua& Instance::GetLua() {
		return *mLua;
	}

	const Lua& Instance::GetLua() const {
		return *mLua;
	}
	
	void Instance::AddServerTask(std::function<void(void)> task) {
		mServer->add_task(std::move(task));
	}

	void Instance::AddClientTask(uint8_t id, MessageID packet) {
		mServer->add_client_task(id, packet);
	}

	void Instance::OnPlayerStart(const PlayerPtr& player) {
		mLua->PreloadAbilities();
		mLevelLoaded = LoadLevel();

		if (mLevelLoaded) {
			std::string levelName = mChainData.GetName().data();

			// create level objects and whatnot
			Markerset markerset;
			if (mLevel.GetMarkerset(levelName + "_obelisk_1.Markerset", markerset)) {
				const auto& healthObelisks = markerset.GetMarkersByType(utils::hash_id("prefab_health_obelisk.Noun"));
				for (const auto& marker : healthObelisks) {
					const auto& object = mObjectManager->Create(marker);
					if (object) {
						SendObjectCreate(object);
					}
				}

				const auto& bossObelisks = markerset.GetMarkersByType(utils::hash_id("prefab_boss_obelisk.Noun"));
				for (const auto& marker : bossObelisks) {
					const auto& object = mObjectManager->Create(marker);
					if (object) {
						SendObjectCreate(object);
					}
				}
			}

			if (mLevel.GetMarkerset(levelName + "_design.Markerset", markerset)) {
				for (const auto& marker : markerset.GetMarkers()) {
					mMarkers[marker->GetId()] = marker;

					const auto& teleporterData = marker->GetTeleporterData();
					if (teleporterData) {
						const auto& object = mObjectManager->Create(marker);
						if (object) {
							SendObjectCreate(object);
						}
					}
				}
			}

			const auto TestEnemy = [this](const auto& markerset) {
				for (const auto& marker : markerset.GetMarkers()) {
					mMarkers[marker->GetId()] = marker;

					switch (marker->GetNoun()) {
						case utils::hash_id("SpawnPoint_Director.Noun"):
						case utils::hash_id("SpawnPoint_DirectorA.Noun"):
						case utils::hash_id("SpawnPoint_DirectorB.Noun"):
						case utils::hash_id("SpawnPoint_DirectorWanderer.Noun"):
						{
							const auto& object = mObjectManager->Create(mChainData.GetEnemyNoun(utils::random::get<uint32_t>(0, 5)));
							if (object) {
								object->SetPosition(marker->GetPosition());
								object->SetOrientation(marker->GetRotation());
								SendObjectCreate(object);
							}
							break;
						}
					}
				}
			};

			if (mLevel.GetMarkerset(levelName + "_AI_WandererA.Markerset", markerset)) {
				TestEnemy(markerset);
			}

			if (mLevel.GetMarkerset(levelName + "_AI_WandererB.Markerset", markerset)) {
				TestEnemy(markerset);
			}

			if (mLevel.GetMarkerset(levelName + "_AI_WandererC.Markerset", markerset)) {
				TestEnemy(markerset);
			}
		}

		mLua->LoadFile("data/lua/player_start.lua");

		// Create creatures
		glm::vec3 spawnpoint;
		if (player->GetId() < mPlayerSpawnpoints.size()) {
			spawnpoint = mPlayerSpawnpoints[player->GetId()];
		}

		for (uint32_t i = 0; i < 3; ++i) {
			const auto& characterObject = player->GetCharacterObject(i);
			if (characterObject) {
				characterObject->SetPosition(spawnpoint);
				characterObject->SetVisible(true); // i == creatureIndex

				characterObject->SetAttributeValue(Attribute::InvisibleToSecurityTeleporters, 1);
				characterObject->SetAttributeValue(Attribute::AttackSpeedScale, 1);
				characterObject->SetAttributeValue(Attribute::CooldownScale, 1);

				SendObjectCreate(characterObject);
				SendObjectUpdate(characterObject);
			}
		}
	}

	uint32_t Instance::AddTask(uint32_t delay, std::function<void(uint32_t)> task) {
		auto id = Scheduler::AddTask(delay, task);
		mEvents.insert(id);
		return id;
	}

	void Instance::CancelTask(uint32_t id) {
		mEvents.erase(id);
		Scheduler::CancelTask(id);
	}

	bool Instance::ServerUpdate() const {
		return (utils::get_milliseconds() - mGameTime) >= 10;
	}

	bool Instance::Update() {
		// It is safe to send packets in this function.
		auto newGameTime = utils::get_milliseconds();
		auto diff = newGameTime - mGameTime;

		if (diff >= 50) {
			mGameTime = newGameTime;
			mObjectManager->Update();

			const auto& client = mServer->GetClient(static_cast<uint8_t>(0));
			if (client) {
				auto& objective = mObjectives.front();
				if (objective.id == utils::hash_id("FinishLevelQuickly")) {
					objective.value = static_cast<uint32_t>(GetTimeElapsed() / 1000);
					mServer->SendObjectiveUpdate(client, 0);
				}
			}

			return true;
		}

		return false;
	}

	void Instance::MoveObject(const ObjectPtr& object, RakNet::LocomotionData& locomotionData) {
		if (!object) {
			return;
		}

		bool teleport = locomotionData.mGoalFlags & 0x020;
		if (teleport) {
			object->SetPosition(locomotionData.mGoalPosition);
			for (const auto& [_, client] : mServer->GetClients()) {
				mServer->SendObjectTeleport(client, object, object->GetPosition(), locomotionData.mFacing);
			}
		} else {
			object->SetPosition(locomotionData.mPartialGoalPosition);
			for (const auto& [_, client] : mServer->GetClients()) {
				mServer->SendObjectPlayerMove(client, object, locomotionData);
			}
		}
	}

	void Instance::UseAbility(const ObjectPtr& object, const RakNet::CombatData& combatData) {
		if (!object) {
			return;
		}

		auto ability = mLua->GetAbility(combatData.abilityId);
		if (ability != sol::nil) {
			sol::object value = ability["tick"];
			if (value.is<sol::protected_function>()) {
				value.as<sol::protected_function>().call<void>(ability, ObjectPtr(object), mObjectManager->Get(combatData.targetId), combatData.cursorPosition);
			}
		}

		// mLua->LoadFile("ability_test.lua");

		/*
		RakNet::CombatEvent combatEvent;
		combatEvent.flags = RakNet::CombatEventFlags::None;
		combatEvent.deltaHealth = -500;
		combatEvent.absorbedAmount = 0;
		combatEvent.targetID = combatData.targetId;
		combatEvent.sourceID = object->GetId();
		combatEvent.abilityID = combatData.abilityId;
		combatEvent.damageDirection = glm::zero<glm::vec3>();
		combatEvent.integerHpChange = -500;
		
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendCombatEvent(client, combatEvent);
		}
		*/
	}

	void Instance::SwapCharacter(const PlayerPtr& player, uint32_t creatureIndex) {
		if (!player) {
			return;
		}

		// TODO: proper functions for these things
		if (!player->SwapCharacter(creatureIndex)) {
			return;
		}

		// set player current creature
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendPlayerCharacterDeploy(client, player, creatureIndex);
		}

		SendLabsPlayerUpdate(player);
	}

	void Instance::InteractWithObject(const PlayerPtr& player, uint32_t objectId) {
		if (!player) {
			return;
		}

		const auto& object = mObjectManager->Get(objectId);
		if (!object || object->IsMarkedForDeletion()) {
			return;
		}

		const auto& noun = object->GetNoun();
		if (noun) {
			switch (noun->GetType()) {
				case NounType::Loot:
					PickupLoot(player, object);
					break;

				case NounType::Crystal:
					PickupCatalyst(player, object);
					break;

				default:
					const auto& characterObject = player->GetDeployedCharacterObject();
					DropLoot(characterObject, characterObject, utils::enum_helper<DropType>::bor(DropType::Catalysts, DropType::Orbs));
					break;
			}
		} else {
			const auto& characterObject = player->GetDeployedCharacterObject();
			DropLoot(characterObject, characterObject, utils::enum_helper<DropType>::bor(DropType::Catalysts, DropType::Orbs));
		}
	}

	void Instance::DropLoot(const PlayerPtr& player, uint64_t lootInstanceId) {
		if (!player) {
			return;
		}

		const auto& user = player->GetUser();
		if (!user) {
			return;
		}

		// get temporary part and drop
	}

	void Instance::DropLoot() {
		constexpr std::array<std::string_view, 4> dropRarityNouns {
			"loot_container_white.Noun",
			"loot_container_green.Noun",
			"loot_drop.noun",
			"loot_container_purple.Noun"
		};

		const auto& clients = mServer->GetClients();
		if (clients.empty()) {
			return;
		}

		auto firstClient = clients.begin()->second;
		auto player = firstClient->GetPlayer();
		auto characterObject = player->GetCharacterObject(1);

		auto position = characterObject->GetPosition();

		static uint64_t lootId = 0;
		for (uint32_t i = 0; i < 4; ++i) {
			auto part = SporeNet::Part(i);
			part.SetRarity(static_cast<SporeNet::PartRarity>(i));

			RakNet::cLootData lootData;
			lootData.mId = ++lootId;
			lootData.mLootInstanceId = lootId;
			lootData.mDNAAmount = 0;
			lootData.crystalLevel = 0;
			lootData.mRigblockAsset = part.GetRigblockAssetHash();
			lootData.mSuffixAssetId = part.GetSuffixAssetHash();
			lootData.mPrefixAssetId1 = part.GetPrefixAssetHash();
			lootData.mPrefixAssetId2 = part.GetPrefixSecondaryAssetHash();
			lootData.mItemLevel = part.GetLevel();
			lootData.mRarity = static_cast<int32_t>(part.GetRarity());

			//
			auto containerType = lootData.mRarity;
			if (containerType >= dropRarityNouns.size()) {
				containerType = 0;
			}

			auto object = mObjectManager->Create(utils::hash_id(dropRarityNouns[containerType]));
			object->SetPosition(position);
			object->SetTeam(0);
			object->SetMovementType(6);
			object->SetInteractableState(0);
			object->SetLootData(std::move(lootData));

			RakNet::cInteractableData interactableData;
			interactableData.mNumUsesAllowed = 1;
			interactableData.mNumTimesUsed = 0;
			interactableData.mInteractableAbility = utils::hash_id("PickUpLoot");

			for (const auto& [_, client] : clients) {
				mServer->SendObjectCreate(client, object);
				mServer->SendLootDataUpdate(client, object, object->GetLootData());
				mServer->SendInteractableDataUpdate(client, object, interactableData);
			}

			mObjects.push_back(std::move(object));
			position.x += 5;
		}
	}

	void Instance::DropLoot(const ObjectPtr& object, const ObjectPtr& targetObject, DropType dropType) {
		if (!object || !targetObject) {
			return;
		}

		const auto& interactableData = object->GetInteractableData();
		if (interactableData) {

		}

		const auto& position = object->GetPosition(); // client uses "GetClosestPosition" but no way in hell im implementing that.
		const auto& footprintRadius = object->GetFootprintRadius();
		if (utils::enum_helper<DropType>::test(dropType, DropType::Orbs)) {
			float orbDifficultyScale;
			if (mChainData.GetLevelIndex() < 72) {
				orbDifficultyScale = 5.f;
			} else {
				orbDifficultyScale = 1.f;
			}

			uint32_t orbType = 1;
			uint32_t orbServerEvent = 0;
			switch (orbType) {
				case 1:
					orbType = utils::hash_id("HealthOrb.Noun");
					orbServerEvent = utils::hash_id("health_orb_drop.ServerEventDef");
					break;

				case 2:
					orbType = utils::hash_id("ManaOrb.Noun");
					orbServerEvent = utils::hash_id("mana_orb_drop.ServerEventDef");
					break;

				case 3:
					orbType = utils::hash_id("ResurrectOrb.Noun");
					orbServerEvent = utils::hash_id("resurrect_orb_drop.ServerEventDef");
					break;
			}

			if (orbType) {
				ServerEvent serverEvent;
				serverEvent.mServerEventDef = orbServerEvent;
				serverEvent.mTargetPoint = position;

				const auto& object = mObjectManager->Create(orbType);
				object->SetPosition(position);

				SendObjectCreate(object);
				SendServerEvent(serverEvent);
			}
		}

		if (utils::enum_helper<DropType>::test(dropType, DropType::Catalysts)) {
			auto crystalFind = targetObject->GetAttributeValue(Attribute::CrystalFind);
			switch (object->GetTeam()) {
				case 2:
					break;

				default:
					break;
			}

			RakNet::labsCrystal crystal(RakNet::labsCrystal::AttackSpeed, 0, false);

			const auto& object = mObjectManager->Create(crystal.crystalNoun);
			object->SetPosition(position);

			SendObjectCreate(object);
		}

		if (utils::enum_helper<DropType>::test(dropType, DropType::DNA)) {
			//
		}

		if (utils::enum_helper<DropType>::test(dropType, DropType::Loot)) {
			//
		}
	}

	void Instance::DropCatalyst(const PlayerPtr& player, uint32_t catalystSlot) {

	}

	void Instance::DropCatalyst() {
		const auto& clients = mServer->GetClients();
		if (clients.empty()) {
			return;
		}

		const auto& firstClient = clients.begin()->second;
		const auto& player = firstClient->GetPlayer();

		auto characterObject = player->GetDeployedCharacterObject();
		if (!characterObject) {
			return;
		}

		auto position = characterObject->GetPosition();

		static uint64_t lootId = 0;
		for (uint32_t i = 0; i < 4; ++i) {
			RakNet::labsCrystal catalyst(static_cast<RakNet::labsCrystal::Type>(i), i % 3, i & 1);

			RakNet::cLootData lootData;
			lootData.mId = ++lootId;
			lootData.crystalLevel = catalyst.level;

			auto object = mObjectManager->Create(catalyst.crystalNoun);
			object->SetPosition(position);
			object->SetTeam(0);
			object->SetMovementType(6);
			object->SetInteractableState(0);
			object->SetLootData(std::move(lootData));

			RakNet::cInteractableData interactableData;
			interactableData.mNumUsesAllowed = 1;
			interactableData.mNumTimesUsed = 0;
			interactableData.mInteractableAbility = utils::hash_id("PickUpLoot");

			for (const auto& [_, client] : clients) {
				mServer->SendObjectCreate(client, object);
				mServer->SendLootDataUpdate(client, object, object->GetLootData());
				mServer->SendInteractableDataUpdate(client, object, interactableData);
			}

			mObjects.push_back(std::move(object));
			position.x += 5;
		}
	}

	/*
		HealthOrb.noun - Orb_Pickup
		HealthOrbPlaced.noun - Orb_Pickup
		JuggernautOrb.noun - Juggernaut_Pickup
		Loot_drop.noun - PickupLoot
		loot_questCrystal.noun - PickupQuestCrystal
		ManaOrb.noun - Orb_Pickup
		ManaOrbPlaced.noun - Orb_Pickup
		ResurrectOrb.noun - PickupResurrectOrb
		DNA.noun - DNA_Pickup
	*/

	void Instance::SendObjectCreate(const ObjectPtr& object) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendObjectCreate(client, object);
		}
	}

	void Instance::SendObjectDelete(const ObjectPtr& object) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendObjectDelete(client, object);
		}
	}

	void Instance::SendObjectDelete(const std::vector<ObjectPtr>& objects) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendObjectDelete(client, objects);
		}
	}

	void Instance::SendObjectUpdate(const ObjectPtr& object) {
		if (!object) {
			return;
		}

		auto flags = object->GetFlags();
		auto newFlags = flags;

		for (const auto& [_, client] : mServer->GetClients()) {
			const auto& player = client->GetPlayer();
			if (object == player->GetDeployedCharacterObject()) {
				if (player->SyncCharacterData()) {
					SendLabsPlayerUpdate(player);
				}
			}

			if (flags & Object::UpdateCombatant) {
				mServer->SendCombatantDataUpdate(client, object, object->GetCombatantData());
				newFlags &= ~Object::UpdateCombatant;
			}

			if (flags & Object::UpdateAttributes) {
				mServer->SendAttributeDataUpdate(client, object, *object->GetAttributeData());
				newFlags &= ~Object::UpdateAttributes;
			}

			if (flags & Object::UpdateLootData) {
				mServer->SendLootDataUpdate(client, object, object->GetLootData());
				newFlags &= ~Object::UpdateLootData;
			}

			if (flags & Object::UpdateAgentBlackboardData) {
				mServer->SendAgentBlackboardUpdate(client, object, object->GetAgentBlackboardData());
				newFlags &= ~Object::UpdateAgentBlackboardData;
			}

			if (flags & Object::UpdateInteractableData) {
				mServer->SendInteractableDataUpdate(client, object, *object->GetInteractableData());
				newFlags &= ~Object::UpdateInteractableData;
			}
			
			if (object->mDataBits.any()) {
				mServer->SendObjectUpdate(client, object);
			}
		}

		object->SetFlags(newFlags);
		object->ResetUpdateBits();
	}

	void Instance::SendAnimationState(const ObjectPtr& object, uint32_t state, bool overlay) {
		if (!object) {
			return;
		}

		const auto timestamp = GetTime();
		if (!overlay) {
			object->mLastAnimationState = state;
			object->mLastAnimationPlayTime = timestamp;
		}

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendAnimationState(client, object, state, timestamp, overlay);
		}
	}

	void Instance::SendObjectGfxState(const ObjectPtr& object, uint32_t state) {
		if (!object) {
			return;
		}

		const auto timestamp = GetTime();
		object->mGraphicsState = state;
		object->mGraphicsStateStartTime = timestamp;

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendObjectGfxState(client, object, state, timestamp);
		}
	}

	void Instance::SendServerEvent(const ServerEventBase& serverEvent) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendServerEvent(client, serverEvent);
		}
	}

	void Instance::SendServerEvent(const PlayerPtr& player, const ServerEventBase& serverEvent) {
		if (!player) {
			return;
		}

		const auto& client = mServer->GetClient(player->GetId());
		if (client) {
			mServer->SendServerEvent(client, serverEvent);
		}
	}

	void Instance::SendCombatEvent(const CombatEvent& combatEvent) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendCombatEvent(client, combatEvent);
		}
	}

	void Instance::SendCooldownUpdate(const ObjectPtr& object, uint32_t id, int64_t milliseconds) {
		if (!object) {
			return;
		}

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendCooldownUpdate(client, object, id, milliseconds);
		}
	}

	void Instance::SendLabsPlayerUpdate(const PlayerPtr& player) {
		if (!player) {
			return;
		}

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendLabsPlayerUpdate(client, player);
		}

		player->ResetUpdateBits();
	}

	void Instance::PickupLoot(const PlayerPtr& player, const ObjectPtr& object) {
		if (!player || !object || object->IsMarkedForDeletion()) {
			return;
		}

		object->MarkForDeletion();
		if (!object->HasLootData()) {
			return;
		}

		ClientEvent lootEvent;
		lootEvent.SetLootPickup(player->GetId(), object->GetLootData());

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendServerEvent(client, lootEvent);
		}
	}

	void Instance::PickupCatalyst(const PlayerPtr& player, const ObjectPtr& object) {
		if (!player || !object || object->IsMarkedForDeletion()) {
			return;
		}

		uint32_t slot = player->GetOpenCrystalSlot();
		if (slot != 0xFFFFFFFF) {
			// object->MarkForDeletion();

			auto crystal = RakNet::labsCrystal(RakNet::labsCrystal::AttackSpeed, 2, true);

			RakNet::CrystalData crystalData;
			crystalData.unk8 = 0;
			crystalData.unk32[0] = 5;
			crystalData.unk32[1] = 0;
			crystalData.unk32[2] = 0;
			crystalData.unk32[3] = crystal.crystalNoun;
			crystalData.unk32[4] = crystal.level;
			crystalData.unk32[5] = 0;
			crystalData.unk32[6] = 0;

			player->SetCrystal(std::move(crystal), 5);
			for (const auto& [_, client] : mServer->GetClients()) {
				mServer->SendCrystalMessage(client, crystalData);
			}
		} else {
			// TODO: add bounce
			ClientEvent catalystEvent;
			catalystEvent.SetCatalystPickup(player->GetId());

			for (const auto& [_, client] : mServer->GetClients()) {
				mServer->SendServerEvent(client, catalystEvent);
			}
		}
	}
}
