
// Include
#include "usersessioncomponent.h"
#include "../client.h"
#include "../../utils/functions.h"
#include <iostream>

/*
	Request Packet IDs

	Response Packet IDs

	BlazeValues meanings
		USER = UserIdentification

	BlazeValues
		USER
			AID
			ALOC
			EXBB
			EXID
			ID
			NAME
			
		DATA
			ADDR
			BPS
			CMAP
			CTY
			CVAR
			DMAP
			HWFG
			PSLM
			QDAT
			UATT
			ULST

		(User Status)
			FLGS
			ID

		(User Data)
			EDAT
			FLGS
			USER

	Request Packets

	Response Packets
		
*/

// Blaze
namespace Blaze {
	// UserSessionComponent
	void UserSessionComponent::Parse(Client* client, const Header& header) {
		switch (header.command) {
			case 0x14:
				UpdateNetworkInfo(client, header);
				break;

			default:
				std::cout << "Unknown usersession command: 0x" << std::hex << header.command << std::dec << std::endl;
				break;
		}
	}

	void UserSessionComponent::NotifyUserAdded(Client* client, uint64_t userId, const std::string& userName) {
		std::cout << "UserSession: Add user" << std::endl;

		auto& request = client->get_request();

		TDF::Packet packet;
		{
			auto& dataStruct = packet.CreateStruct(nullptr, "DATA");
			packet.CreateUnion(&dataStruct, "ADDR", NetworkAddressMember::Unset);
			packet.PutString(&dataStruct, "BPS", "");
			packet.PutString(&dataStruct, "CTY", "");
			{
				auto& dmapMap = packet.CreateMap(&dataStruct, "DMAP", TDF::Type::Integer, TDF::Type::Integer);
				packet.PutInteger(&dmapMap, "0x70001", 55);
				packet.PutInteger(&dmapMap, "0x70002", 707);
			}
			packet.PutInteger(&dataStruct, "HWFG", 0);
			{
				auto& qdatStruct = packet.CreateStruct(&dataStruct, "QDAT");
				packet.PutInteger(&qdatStruct, "DBPS", 0);
				packet.PutInteger(&qdatStruct, "NATT", static_cast<uint64_t>(NatType::Open));
				packet.PutInteger(&qdatStruct, "UBPS", 0);
			}
			packet.PutInteger(&dataStruct, "UATT", 0);
		} {
			auto& userStruct = packet.CreateStruct(nullptr, "USER");
			packet.PutInteger(&userStruct, "AID", userId);
			packet.PutInteger(&userStruct, "ALOC", request["CINF"]["LOC"].GetUint64());
			packet.PutInteger(&userStruct, "EXBB", 0);
			packet.PutInteger(&userStruct, "EXID", 0);
			packet.PutInteger(&userStruct, "ID", userId);
			packet.PutString(&userStruct, "NAME", userName);
		}

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::UserSessions;
		header.command = 0x02;
		header.error_code = 0;

		client->notify(header, outBuffer);
	}

	void UserSessionComponent::NotifyUserUpdated(Client* client, uint64_t userId) {
		auto& request = client->get_request();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "FLGS", 3);
		packet.PutInteger(nullptr, "ID", userId);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		Header header;
		header.component = Component::UserSessions;
		header.command = 0x05;
		header.error_code = 0;

		client->notify(header, outBuffer);
	}

	void UserSessionComponent::UpdateNetworkInfo(Client* client, Header header) {
		std::cout << "Update network info" << std::endl;
		/*
		Log.Info(string.Format("Client {0} updating network info", request.Client.ID));

        var addr = (TdfUnion)request.Data["ADDR"];
        var valu = (TdfStruct)addr.Data.Find(tdf => tdf.Label == "VALU");

        var inip = (TdfStruct)valu.Data.Find(tdf => tdf.Label == "INIP");
        var ip = (TdfInteger)inip.Data.Find(tdf => tdf.Label == "IP");
        var port = (TdfInteger)inip.Data.Find(tdf => tdf.Label == "PORT");

        request.Client.InternalIP = ip.Value;
        request.Client.InternalPort = (ushort)port.Value;

        request.Client.ExternalIP = ip.Value;
        request.Client.ExternalPort = (ushort)port.Value;

        request.Reply();
		*/
	}
}
