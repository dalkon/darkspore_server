
// Include
#include "authcomponent.h"
#include "usersessioncomponent.h"
#include "../client.h"
#include "../../utils/functions.h"
#include <iostream>

/*
	Request Packet IDs
		0x0A = CreateAccount
		0x14 = UpdateAccount
		0x1C = UpdateParentalEmail
		0x1D = ListUserEntitlements2
		0x1E = GetAccount
		0x1F = GrantEntitlement
		0x20 = ListEntitlements
		0x21 = HasEntitlement
		0x22 = GetUseCount
		0x23 = DecrementUseCount
		0x24 = GetAuthToken
		0x25 = GetHandoffToken
		0x26 = GetPasswordRules
		0x27 = GrantEntitlement2
		0x28 = Login
		0x29 = AcceptTOS
		0x2A = GetTOSInfo
		0x2B = ModifyEntitlements2
		0x2C = ConsumeCode
		0x2D = ForgottenPassword
		0x2E = GetTermsAndConditions
		0x2F = GetPrivacyPolicy
		0x30 = ListPersonaEntitlements2
		0x32 = SilentLogin
		0x33 = CheckAgeRequirement
		0x3C = ExpressLogin
		0x46 = Logout
		0x50 = CreatePersona
		0x5A = GetPersona
		0x64 = ListPersonas
		0x6E = LoginPersona
		0x78 = LogoutPersona
		0x8C = DeletePersona
		0x8D = DisablePersona
		0x8F = ListDeviceAccounts
		0x96 = XboxCreateAccount
		0xA0 = XboxAssociateAccount
		0xAA = XboxLogin
		0xB4 = PS3CreateAccount
		0xBE = PS3AssociateAccount
		0xC8 = PS3Login
		0xD2 = ValidateSessionKey
		0xE6 = WalUserSession

	Response Packet IDs

	BlazeValues meanings
		PDTL = PersonaDetails
		SESS = SessionInfo
		NTOS = Need Terms Of Service
		PID = Persona ID
		MAIL = Client Email
		UID = Unique ID
		PCTK = Packet Key

	BlazeValues
		PLST
			List of PDTL

		PDTL
			DSNM
			LAST
			PID
			STAS
			XREF
			XTYP

		SESS
			BUID
			FRST
			KEY
			LLOG
			MAIL
			PDTL
			UID

		(User details)
			MAIL
			PLST

		(User profile details)
			CITY
			CTRY
			GNDR
			STAT
			STRT
			ZIP

		(Account defails)
			ASRC
			CO
			DOB
			DTCR
			GOPT
			LATH
			LN
			MAIL
			PML
			RC
			STAS
			STAT
			TOSV
			TPOT
			UID

	Request Packets
		ConsumeCode
			CDKY
			DEID
			GNAM
			KEY
			PID
			PNID

		Login | Yes this is actually what the client sends, completely static
			53 8B 5C 24 10 8B 53 20  55 8B 6C 24 10 56 57 8B
			7C 24 14 8B 07 6A 00 6A  00 8B F1 8B 4B 24 51 52
			8B 50 30 8D 4E 20 51 68  00 64 6A 93 56 55 8B CF
			FF D2 8B 07 8B 40 24 68  01 01 00 00 68 A4 D5 FC
			00 8D 4B 08 51 8D 56 08  52 68 00 6C

		ExpressLogin
			MAIL
			PASS
			PNAM

		LoginPersona
			PNAM

	Response Packets
		ConsumeCode
			EXRF
			KEY
			MCNT
			MFLG
			MLMT
			PRID
			PRMN
			STAT
			UID

		Login
			NTOS
			PCTK
			PLST
			PRIV
			SKEY
			SPAM
			THST
			TURI
			UID

		FullLogin
			AGUP
			NTOS
			PCTK
			PRIV
			SESS
			SPAM
			THST
			TURI
*/

// Blaze
namespace Blaze {
	// AuthComponent
	void AuthComponent::Parse(Client* client, const Header& header) {
		switch (header.command) {
			case 0x1D:
				ListUserEntitlements(client, header);
				break;

			case 0x24:
				SendAuthToken(client, header);
				break;

			case 0x28:
				Login(client, header);
				break;

			case 0x32:
				SilentLogin(client, header);
				break;

			case 0x46:
				std::cout << "Logout" << std::endl;
				break;

			case 0x6E:
				LoginPersona(client, header);
				break;

			default:
				std::cout << "Unknown auth command: 0x" << std::hex << header.command << std::dec << std::endl;
				break;
		}
	}

	void AuthComponent::ListUserEntitlements(Client* client, Header header) {
		std::cout << "List user entitlements" << std::endl;
		/*
		Log.Info(string.Format("Client {0} requested user entitlements", request.Client.ID));

		var etag = (TdfString)request.Data["ETAG"];
		bool onlineAccess = (etag.Value == "ONLINE_ACCESS");

		if (!onlineAccess)
		{
			var nlst = new TdfList("NLST", TdfBaseType.Struct, new ArrayList
			{
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2011-11-02T11:2Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1234632478),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "303566"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "DR:224766400"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "ONLINE_ACCESS"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 1),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2011-11-02T11:2Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1294632417),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "303566"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "303566"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "PROJECT10_CODE_CONSUMED_LE1"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 1),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2013-02-22T14:40Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 5674749135),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "306678"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:50401"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PREMIUM_ACCESS"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2014-05-29T6:15Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1005150961807),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "305060"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "DR:235665900"),
					new TdfInteger("STAT", 2),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "ONLINE_ACCESS"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 1),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2013-02-22T14:40Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1002134961807),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "305061"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "DR:235663400"),
					new TdfInteger("STAT", 2),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "ONLINE_ACCESS"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 1),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2012-06-04T21:13Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1771457489),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "306678"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:50400"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PREMIUM_ACCESS"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// DLC 1 - Back 2 Karkand
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2012-06-04T21:13Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1771457490),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "302777"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:50400"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PC:B2K_PURCHASE"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// DLC 2
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2012-06-04T21:13Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1771457491),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "302776"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:48215"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PC:XPACK2_PURCHASE"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// DLC 3
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2014-02-07T20:15Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1004743136441),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "302778"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:51080"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PC:XPACK3_PURCHASE"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// DLC 4
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2012-11-26T9:4Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1000808118611),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "303129"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:55171"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PC:XPACK4_PURCHASE"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// DLC 5
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2013-03-07T2:21Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1002246118611),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "306409"),
					new TdfInteger("PRCA", 2),
					new TdfString("PRID", "OFB-EAST:109546437"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:PC:XPACK5_PURCHASE"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				},

				// COOP shortcut
				new List<Tdf>
				{
					new TdfString("DEVI", ""),
					new TdfString("GDAY", "2012-04-17T15:57Z"),
					new TdfString("GNAM", "BF3PC"),
					new TdfInteger("ID", 1684196754),
					new TdfInteger("ISCO", 0),
					new TdfInteger("PID", 0),
					new TdfString("PJID", "306215"),
					new TdfInteger("PRCA", 1),
					new TdfString("PRID", "OFB-EAST:48642"),
					new TdfInteger("STAT", 1),
					new TdfInteger("STRC", 0),
					new TdfString("TAG", "BF3:SHORTCUT:COOP"),
					new TdfString("TDAY", ""),
					new TdfInteger("TYPE", 5),
					new TdfInteger("UCNT", 0),
					new TdfInteger("VER", 0)
				}
			});

			request.Reply(0, new List<Tdf> { nlst });
		}
		else
		{
			request.Reply();
		}
		*/
	}

	void AuthComponent::SendAuthToken(Client* client, Header header) {
		std::cout << "Send auth token" << std::endl;

		TDF::Packet packet;
		packet.PutString(nullptr, "AUTH", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.error_code = 0;
		client->reply(header, outBuffer);
	}

	void AuthComponent::Login(Client* client, Header header) {
		std::cout << "Login" << std::endl;

		auto& request = client->get_request();

		std::string email = request["MAIL"].GetString();
		if (email != "dalkon@live.se") {
			std::cout << "User with email " << email << " not found." << std::endl;
			return;
		}

		SendLogin(client, std::move(header));

		// UserSessionComponent::NotifyUserAdded(client, 0, "Dalkon");
		// UserSessionComponent::NotifyUserUpdated(client, 0);
	}

	void AuthComponent::SilentLogin(Client* client, Header header) {
		std::cout << "Silent Login" << std::endl;

		auto& request = client->get_request();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "AGUP", 0);
		packet.PutString(nullptr, "LDHT", "");
		packet.PutInteger(nullptr, "NTOS", 0);
		packet.PutString(nullptr, "PCTK", "PlayerTicket_1337");
		packet.PutString(nullptr, "PRIV", "");
		{
			auto& sessStruct = packet.CreateStruct(nullptr, "SESS");
			packet.PutInteger(&sessStruct, "BUID", 0);
			packet.PutInteger(&sessStruct, "FRST", 0);
			packet.PutString(&sessStruct, "KEY", "SessionKey_1337");
			packet.PutInteger(&sessStruct, "LLOG", 0);
			packet.PutString(&sessStruct, "MAIL", request["MAIL"].GetString());
			{
				auto& pdtlStruct = packet.CreateStruct(nullptr, "PDTL");
				packet.PutString(&pdtlStruct, "DSNM", "Dalkon");
				packet.PutInteger(&pdtlStruct, "LAST", 0);
				packet.PutInteger(&pdtlStruct, "PID", 0);
				packet.PutInteger(&pdtlStruct, "STAS", 0);
				packet.PutInteger(&pdtlStruct, "XREF", 0);
				packet.PutInteger(&pdtlStruct, "XTYP", static_cast<uint64_t>(ExternalRefType::Unknown));
			}
			packet.PutInteger(&sessStruct, "UID", 0);
		}
		packet.PutInteger(nullptr, "SPAM", 0);
		packet.PutString(nullptr, "THST", "");
		packet.PutString(nullptr, "TSUI", "");
		packet.PutString(nullptr, "TURI", "");

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.error_code = 0;
		client->reply(header, outBuffer);
	}

	void AuthComponent::LoginPersona(Client* client, Header header) {
		std::cout << "Login persona" << std::endl;

		SendLoginPersona(client, std::move(header));

		// UserSessionComponent::NotifyUserAdded(client, 0, "Dalkon");
		// UserSessionComponent::NotifyUserUpdated(client, 0);
	}

	void AuthComponent::SendLogin(Client* client, Header header) {
		auto& request = client->get_request();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "NTOS", 0);
		packet.PutString(nullptr, "PCTK", "");
		{
			auto& plstList = packet.CreateList(nullptr, "PLST", TDF::Type::Struct);
			{
				auto& pdtlStruct = packet.CreateStruct(&plstList, "");
				packet.PutString(&pdtlStruct, "DSNM", "Dalkon");
				packet.PutInteger(&pdtlStruct, "LAST", 0);
				packet.PutInteger(&pdtlStruct, "PID", 0);
				packet.PutInteger(&pdtlStruct, "STAS", 2);
				packet.PutInteger(&pdtlStruct, "XREF", 0);
				packet.PutInteger(&pdtlStruct, "XTYP", static_cast<uint64_t>(ExternalRefType::Unknown));
			}
		}
		packet.PutString(nullptr, "PRIV", "");
		packet.PutString(nullptr, "SKEY", "");
		packet.PutInteger(nullptr, "SPAM", 1);
		packet.PutString(nullptr, "THST", "");
		packet.PutString(nullptr, "TURI", "");
		packet.PutInteger(nullptr, "UID", 0);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.component = Component::Authentication;
		header.command = 0x28;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void AuthComponent::SendLoginPersona(Client* client, Header header) {
		auto& request = client->get_request();
		uint64_t currentTime = utils::get_unix_time();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "BUID", 0);
		packet.PutInteger(nullptr, "FRST", 0);
		packet.PutString(nullptr, "KEY", "");
		packet.PutInteger(nullptr, "LLOG", currentTime);
		packet.PutString(nullptr, "MAIL", request["MAIL"].GetString());
		{
			auto& pdtlStruct = packet.CreateStruct(nullptr, "PDTL");
			packet.PutString(&pdtlStruct, "DSNM", "Dalkon");
			packet.PutInteger(&pdtlStruct, "LAST", currentTime);
			packet.PutInteger(&pdtlStruct, "PID", 0);
			packet.PutInteger(&pdtlStruct, "STAS", 2);
			packet.PutInteger(&pdtlStruct, "XREF", 0);
			packet.PutInteger(&pdtlStruct, "XTYP", static_cast<uint64_t>(ExternalRefType::Unknown));
		}
		packet.PutInteger(nullptr, "UID", 0);

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.component = Component::Authentication;
		header.command = 0x6E;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}

	void AuthComponent::SendFullLogin(Client* client, Header header) {
		auto& request = client->get_request();
		uint64_t currentTime = utils::get_unix_time();

		TDF::Packet packet;
		packet.PutInteger(nullptr, "AGUP", 0);
		packet.PutInteger(nullptr, "NTOS", 0);
		packet.PutString(nullptr, "PCTK", "");
		{
			auto& sessStruct = packet.CreateStruct(nullptr, "SESS");
			packet.PutInteger(&sessStruct, "BUID", 0);
			packet.PutInteger(&sessStruct, "FRST", 0);
			packet.PutString(&sessStruct, "KEY", "");
			packet.PutInteger(&sessStruct, "LLOG", currentTime);
			packet.PutString(&sessStruct, "MAIL", request["MAIL"].GetString());
			{
				auto& pdtlStruct = packet.CreateStruct(&sessStruct, "PDTL");
				packet.PutString(&pdtlStruct, "DSNM", "Dalkon");
				packet.PutInteger(&pdtlStruct, "LAST", currentTime);
				packet.PutInteger(&pdtlStruct, "PID", 0);
				packet.PutInteger(&pdtlStruct, "STAS", 2);
				packet.PutInteger(&pdtlStruct, "XREF", 0);
				packet.PutInteger(&pdtlStruct, "XTYP", static_cast<uint64_t>(ExternalRefType::Unknown));
			}
			packet.PutInteger(&sessStruct, "UID", 0);
		}
		packet.PutInteger(nullptr, "SPAM", 0);
		packet.PutString(nullptr, "THST", "");
		packet.PutString(nullptr, "TURI", "");

		DataBuffer outBuffer;
		packet.Write(outBuffer);

		header.component = Component::Authentication;
		header.command = 0x3C;
		header.error_code = 0;

		client->reply(std::move(header), outBuffer);
	}
}
