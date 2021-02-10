
// Include
#include "instance.h"

#include "objectmanager.h"
#include "lua.h"

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
		if (mServer && mServer->is_running()) {
			mServer->stop();
			// wait until it stops
		}

		mServer.reset();
		mLua.reset();
		mObjectManager.reset();
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
		mLua->LoadFile("data/lua/player_start.lua");
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

		// TODO: do we check triggers here instead of at update... or both?

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

	void Instance::PickupLoot(const PlayerPtr& player, uint32_t lootObjectId) {
		if (!player) {
			return;
		}

		const auto& object = mObjectManager->Get(lootObjectId);
		if (!object) {
			return;
		}

		object->MarkForDeletion();
		if (!object->HasLootData()) {
			return;
		}

		const auto& lootData = object->GetLootData();

		RakNet::ServerEvent lootEvent;
		lootEvent.textValue = player->GetId();
		lootEvent.clientEventID = 0x615f3861;
		lootEvent.clientIgnoreFlags = 0;
		lootEvent.lootReferenceId = 0;
		lootEvent.lootInstanceId = lootData.mLootInstanceId;
		lootEvent.lootRigblockId = lootData.mRigblockAsset;
		lootEvent.lootSuffixAssetId = lootData.mSuffixAssetId;
		lootEvent.lootPrefixAssetId1 = lootData.mPrefixAssetId1;
		lootEvent.lootPrefixAssetId2 = lootData.mPrefixAssetId2;
		lootEvent.lootItemLevel = lootData.mItemLevel;
		lootEvent.lootRarity = lootData.mRarity;
		lootEvent.lootCreationTime = 0;

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendServerEvent(client, lootEvent);
			mServer->SendObjectDelete(client, object);
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

	void Instance::PickupCatalyst(const PlayerPtr& player, uint32_t catalystObjectId) {
		if (!player) {
			return;
		}

		const auto& object = mObjectManager->Get(catalystObjectId);
		if (!object) {
			return;
		}

		object->MarkForDeletion();
		if (!object->HasLootData()) {
			return;
		}

		const auto& lootData = object->GetLootData();

		RakNet::ServerEvent lootEvent;
		lootEvent.textValue = player->GetId();
		lootEvent.clientEventID = 0x615f3861;
		lootEvent.clientIgnoreFlags = 0;
		lootEvent.lootReferenceId = 0;
		lootEvent.lootInstanceId = lootData.mLootInstanceId;
		lootEvent.lootRigblockId = lootData.mRigblockAsset;
		lootEvent.lootSuffixAssetId = lootData.mSuffixAssetId;
		lootEvent.lootPrefixAssetId1 = lootData.mPrefixAssetId1;
		lootEvent.lootPrefixAssetId2 = lootData.mPrefixAssetId2;
		lootEvent.lootItemLevel = lootData.mItemLevel;
		lootEvent.lootRarity = lootData.mRarity;
		lootEvent.lootCreationTime = 0;

		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendServerEvent(client, lootEvent);
			mServer->SendObjectDelete(client, object);
		}
	}

	void Instance::DropCatalyst(const PlayerPtr& player, uint32_t catalystSlot) {

	}

	void Instance::DropCatalyst() {
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
				mServer->SendAttributeDataUpdate(client, object, object->GetAttributeData());
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

	void Instance::SendServerEvent(const RakNet::ServerEvent& serverEvent) {
		for (const auto& [_, client] : mServer->GetClients()) {
			mServer->SendServerEvent(client, serverEvent);
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
}
