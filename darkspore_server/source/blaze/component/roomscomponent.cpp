
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

	Notification Packet IDs
		0x0A = NotifyRoomViewUpdated
		0x0B = NotifyRoomViewAdded
		0x0C = NotifyRoomViewRemoved
		0x14 = NotifyRoomCategoryUpdated
		0x15 = NotifyRoomCategoryAdded
		0x16 = NotifyRoomCategoryRemoved
		0x1E = NotifyRoomUpdated
		0x1F = NotifyRoomAdded
		0x20 = NotifyRoomRemoved
		0x28 = NotifyRoomPopulationUpdated
		0x32 = NotifyRoomMemberJoined
		0x33 = NotifyRoomMemberLeft
		0x34 = NotifyRoomMemberUpdated
		0x3C = NotifyRoomKick
		0x46 = NotifyRoomHostTransfer
		0x50 = NotifyRoomAttributesSet


	Blaze fields
		RoomReplicationContext
			SEID = 0x38
			UPRE = 0x1C
			USID = 0x34
			VWID = 0x38

		RoomViewData
			DISP = 0x24
			GMET = 0x54
			META = 0x54
			MXRM = 0x38
			NAME = 0x24
			USRM = 0x38
			VWID = 0x38
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

	void RoomsComponent::SendSelectCategoryUpdates(Client* client, uint32_t viewId) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "VWID", viewId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::Rooms;
		header.command = 0x0B;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void RoomsComponent::NotifyRoomViewUpdated(Client* client, uint32_t viewId) {
		TDF::Packet packet;
		packet.PutInteger(nullptr, "VWID", viewId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::Rooms;
		header.command = 0x0A;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}
	
	void RoomsComponent::NotifyRoomViewAdded(Client* client, uint32_t viewId) {
		TDF::Packet packet;
		packet.PutString(nullptr, "DISP", "");
		{
			auto& gmetMap = packet.CreateMap(nullptr, "GMET", TDF::Type::String, TDF::Type::Struct);
		} {
			auto& metaMap = packet.CreateMap(nullptr, "META", TDF::Type::String, TDF::Type::Struct);
		}
		packet.PutInteger(nullptr, "MXRM", viewId);
		packet.PutString(nullptr, "NAME", "Dalkon's Room");
		packet.PutInteger(nullptr, "USRM", viewId);
		packet.PutInteger(nullptr, "VWID", viewId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::Rooms;
		header.command = 0x0B;
		header.error_code = 0;

		client->notify(std::move(header), outBuffer);
	}

	void RoomsComponent::SelectViewUpdates(Client* client, Header header) {
		auto& request = client->get_request();

		bool update = request["UPDT"].GetUint() != 0;
		if (update) {
			// What do we send here?
			TDF::Packet packet;
			packet.PutInteger(nullptr, "SEID", 1);
			packet.PutInteger(nullptr, "UPRE", 0);
			packet.PutInteger(nullptr, "USID", 1);
			packet.PutInteger(nullptr, "VWID", 1);

			DataBuffer outBuffer;
			packet.Write(outBuffer);

			header.component = Component::Rooms;
			header.command = 0x0A;
			header.error_code = 0;

			client->reply(std::move(header), outBuffer);
			
			NotifyRoomViewUpdated(client, 1);
		}
	}

	void RoomsComponent::SelectCategoryUpdates(Client* client, Header header) {
		auto& request = client->get_request();

		uint32_t viewId = request["VWID"].GetUint();
		SendSelectCategoryUpdates(client, viewId);
	}
}
