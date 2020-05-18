
// Include
#include "instance.h"
#include "creature.h"
#include "user.h"
#include "room.h"

// SporeNet
namespace SporeNet {
	// Instance
	Instance::Instance() {
		mTemplateDatabase = std::make_unique<TemplateDatabase>();
		mTemplateDatabase->Load();

		mUserManager = std::make_unique<UserManager>();
		mRoomManager = std::make_unique<RoomManager>();
	}

	Instance::~Instance() {
		mRoomManager.reset();
		mUserManager.reset();
		mTemplateDatabase.reset();
	}

	TemplateDatabase& Instance::GetTemplateDatabase() const {
		return *mTemplateDatabase;
	}

	UserManager& Instance::GetUserManager() const {
		return *mUserManager;
	}

	RoomManager& Instance::GetRoomManager() const {
		return *mRoomManager;
	}
}
