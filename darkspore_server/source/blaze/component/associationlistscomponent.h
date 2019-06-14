
#ifndef _BLAZE_COMPONENT_ASSOCIATIONLISTS_HEADER
#define _BLAZE_COMPONENT_ASSOCIATIONLISTS_HEADER

// Include
#include "../tdf.h"

// Blaze
namespace Blaze {
	class Client;

	// AssociationListsComponent
	class AssociationListsComponent {
		public:
			static void Parse(Client* client, const Header& header);

			static void SendLists(Client* client);

		private:
			static void GetLists(Client* client, Header header);
	};
}

#endif
