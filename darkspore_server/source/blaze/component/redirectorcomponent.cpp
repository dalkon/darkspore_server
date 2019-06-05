
// Include
#include "redirectorcomponent.h"
#include "../client.h"
#include <iostream>

// Blaze
namespace Blaze {
	// RedirectorComponent
	void RedirectorComponent::Parse(Client* client, const Header& header) {
		switch (header.command) {
			case 1:
				SendServerInstanceInfo(client, header);
				break;

			default:
				std::cout << "Unknown redirector command: 0x" << std::hex << header.command << std::dec << std::endl;
				break;
		}
	}

	void RedirectorComponent::SendServerInstanceInfo(Client* client, Header header) {
		TDF::Packet packet;

		auto& addrUnion = packet.CreateUnion(nullptr, "ADDR", NetworkAddressMember::XboxClientAddress);
		{
			auto& valuStruct = packet.CreateStruct(&addrUnion, "VALU");
			packet.PutString(&valuStruct, "HOST", "127.0.0.1");
			packet.PutInteger(&valuStruct, "IP", 0);
			packet.PutInteger(&valuStruct, "PORT", 10041);
		}
		packet.PutInteger(nullptr, "SECU", 1);
		packet.PutInteger(nullptr, "XDNS", 0);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.error_code = 0;
		client->reply(header, outBuffer);
	}
}
