
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

			// Responses
			static void SendSelectCategoryUpdates(Client* client, uint32_t viewId);

			// Notifications
			static void NotifyRoomViewUpdated(Client* client, uint32_t viewId);
			static void NotifyRoomViewAdded(Client* client, uint32_t viewId);

		private:
			static void SelectViewUpdates(Client* client, Header header);
			static void SelectCategoryUpdates(Client* client, Header header);
	};
}

#endif
