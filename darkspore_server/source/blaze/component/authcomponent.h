
#ifndef _BLAZE_COMPONENT_AUTH_HEADER
#define _BLAZE_COMPONENT_AUTH_HEADER

// Include
#include "../tdf.h"

// Blaze
namespace Blaze {
	class Client;

	// AuthComponent
	class AuthComponent {
		public:
			static void Parse(Client* client, const Header& header);

		private:
			static void ListUserEntitlements(Client* client, Header header);
			static void SendAuthToken(Client* client, Header header);
			static void Login(Client* client, Header header);
			static void SilentLogin(Client* client, Header header);
			static void LoginPersona(Client* client, Header header);

			static void SendLogin(Client* client, Header header);
			static void SendLoginPersona(Client* client, Header header);
			static void SendFullLogin(Client* client, Header header);
	};
}

#endif
