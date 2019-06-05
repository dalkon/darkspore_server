
#ifndef _BLAZE_COMPONENT_REDIRECTOR_HEADER
#define _BLAZE_COMPONENT_REDIRECTOR_HEADER

// Include
#include "../tdf.h"

// Blaze
namespace Blaze {
	class Client;

	// RedirectorComponent
	class RedirectorComponent {
		public:
			static void Parse(Client* client, const Header& header);

		private:
			static void SendServerInstanceInfo(Client* client, Header header);
	};
}

#endif
