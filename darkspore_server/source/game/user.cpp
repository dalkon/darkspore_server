
// Include
#include "user.h"

// Game
namespace Game {
	// User
	User::User() {

	}

	User::~User() {

	}

	std::shared_ptr<User> User::GetUserByEmail(const std::string& email) {
		if (email != "dalkon@live.se") {
			return nullptr;
		}

		auto user = std::make_shared<User>();
		user->mEmail = email;
		user->mName = "Dalkon";
		user->mId = 0;

		return user;
	}
}
