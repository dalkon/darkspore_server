
// Include
#include "roomscomponent.h"
#include "../client.h"
#include "../../utils/functions.h"
#include <iostream>

/*
	Packet IDs
		0x0A = SelectViewUpdates
		0x0B = SelectCategoryUpdates
		0x14 = JoinRoom
		0x15 = LeaveRoom
		0x1F = KickUser
		0x28 = TransferRoomHost
		0x64 = CreateRoomCategory
		0x65 = RemoveRoomCategory
		0x66 = CreateRoom
		0x67 = RemoveRoom
		0x68 = ClearBannedUsers
		0x69 = UnbanUser
		0x6D = GetViews
		0x6E = CreateScheduledCategory
		0x6F = DeleteScheduledCategory
		0x70 = GetSchedulesCategories
		0x78 = LookupRoomData
		0x7A = ListBannedUsers
		0x82 = SetRoomAttributes
		0x8C = CheckEntryCriteria
		0x96 = ToggleJoinedRoomNotifications
		0xA0 = SelectPseudoRoomUpdates
*/

// Blaze
namespace Blaze {
	// RoomsComponent
	void RoomsComponent::Parse(Client* client, const Header& header) {
		switch (header.command) {
			case 0x0A:
				SelectViewUpdates(client, header);
				break;

			case 0x0B:
				SelectCategoryUpdates(client, header);
				break;

			default:
				std::cout << "Unknown rooms command: 0x" << std::hex << header.command << std::dec << std::endl;
				break;
		}
	}

	void RoomsComponent::SelectViewUpdates(Client* client, Header header) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "VWID", 1);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.component = Component::Rooms;
		header.command = 0x0A;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void RoomsComponent::SelectCategoryUpdates(Client* client, Header header) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "VWID", 1);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.component = Component::Rooms;
		header.command = 0x0B;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}
}
