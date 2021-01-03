
// Include
#include "instance.h"
#include "raknet/server.h"
#include "sporenet/instance.h"
#include "sporenet/user.h"
#include "utils/functions.h"

#include <iostream>

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
	}

	Instance::~Instance() {
		Stop();
	}

	bool Instance::Start() {
		Stop();

		mObjectManager = std::make_unique<ObjectManager>();
		mServer = std::make_unique<RakNet::Server>(*this);

		mServer->start(mData.hostNetwork.exip.port);
		mGameStartTime = utils::get_unix_time();

		return true;
	}

	void Instance::Stop() {
		if (mServer && mServer->is_running()) {
			mServer->stop();
			// wait until it stops
		}

		mServer.reset();
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

	const RakNet::ChainVoteData& Instance::GetChainData() const {
		return mChainData;
	}

	RakNet::ChainVoteData& Instance::GetChainData() {
		return mChainData;
	}

	const RakNet::GameStateData& Instance::GetStateData() const {
		return mStateData;
	}

	RakNet::GameStateData& Instance::GetStateData() {
		return mStateData;
	}

	const Blaze::ReplicatedGameData& Instance::GetInfo() const {
		return mData;
	}

	Blaze::ReplicatedGameData& Instance::GetInfo() {
		return mData;
	}

	const ObjectManager& Instance::GetObjectManager() const {
		return *mObjectManager;
	}

	ObjectManager& Instance::GetObjectManager() {
		return *mObjectManager;
	}

	uint64_t Instance::GetTime() const {
		return mGameTime;
	}

	uint64_t Instance::GetTimeElapsed() const {
		return utils::get_unix_time() - mGameStartTime;
	}
	
	void Instance::AddServerTask(std::function<void(void)> task) {
		mServer->add_task(std::move(task));
	}

	void Instance::AddClientTask(uint8_t id, MessageID packet) {
		mServer->add_client_task(id, packet);
	}

	bool Instance::Update() {
		auto newGameTime = utils::get_unix_time();
		auto diff = newGameTime - mGameTime;

		if (diff >= 1) {
			mGameTime = newGameTime;
			return true;
		}

		return false;
	}
}
