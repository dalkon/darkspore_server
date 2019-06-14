
#ifndef _BLAZE_COMPONENT_ROOMS_HEADER
#define _BLAZE_COMPONENT_ROOMS_HEADER

// Include
#include "../tdf.h"

// Blaze
namespace Blaze {
	class Client;

	// RoomsComponent
	class RoomsComponent {
		public:
			static void Parse(Client* client, const Header& header);

			static void SelectViewUpdates(Client* client, Header header);
			static void SelectCategoryUpdates(Client* client, Header header);
	};
}

#endif
