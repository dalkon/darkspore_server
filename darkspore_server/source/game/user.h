
#ifndef _GAME_USER_HEADER
#define _GAME_USER_HEADER

// Include
#include "../databuffer.h"
#include <boost/asio.hpp>

// Game
namespace Game {
	// User
	class User {
		public:
			User();
			~User();

			static std::shared_ptr<User> GetUserByEmail(const std::string& email);

		private:
			std::string mEmail;
			std::string mName;

			uint64_t mId;
	};
}

#endif
