
// Include
#include "server.h"

#include <boost/bind.hpp>
#include <iostream>

// Blaze
namespace Blaze {
	// Server
	Server::Server(boost::asio::io_context& io_service, uint16_t port) :
		mIoService(io_service),
		mAcceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		mContext(boost::asio::ssl::context::sslv3)
	{
		const std::string certificatePath = "crt/old/cacert.pem";
		const std::string privateKeyPath = "crt/old/privkey.pem";

		auto nativeHandle = mContext.native_handle();

		SSL_CTX_set_security_level(nativeHandle, 0);
		// SSL_CTX_set_cipher_list(nativeHandle, "AES:ALL:!aNULL:!eNULL:+RC4:@STRENGTH");
		SSL_CTX_set_cipher_list(nativeHandle, "RC4-MD5:RC4-SHA");

		if (SSL_CTX_load_verify_locations(nativeHandle, certificatePath.c_str(), privateKeyPath.c_str()) != 1) {
			std::cout << "SSL_CTX: Could not set verify locations" << std::endl;
		}

		if (SSL_CTX_set_default_verify_paths(nativeHandle) != 1) {
			std::cout << "SSL_CTX: Could not set default verify paths" << std::endl;
		}

		mContext.set_verify_callback(&Server::verify_callback);
		mContext.use_certificate_file(certificatePath, boost::asio::ssl::context::pem);
		mContext.use_private_key_file(privateKeyPath, boost::asio::ssl::context::pem);

		if (!SSL_CTX_check_private_key(nativeHandle)) {
			std::cout << "SSL_CTX: Private key does not match the public certificate" << std::endl;
			abort();
		}

		Client* client = new Client(mIoService, mContext);
		mAcceptor.async_accept(client->get_socket(),
			boost::bind(&Server::handle_accept, this, client, boost::asio::placeholders::error));
	}

	Server::~Server() {

	}

	void Server::handle_accept(Client* client, const boost::system::error_code& error) {
		if (!error) {
			client->start();
			client = new Client(mIoService, mContext);
			mAcceptor.async_accept(client->get_socket(),
				boost::bind(&Server::handle_accept, this, client, boost::asio::placeholders::error));
		} else {
			std::cout << error.message() << std::endl;
			delete client;
		}
	}

	bool Server::verify_callback(bool preverified, boost::asio::ssl::verify_context& context) {
		char subject_name[256];

		X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

		bool verified = boost::asio::ssl::rfc2818_verification("gosredirector.ea.com")(preverified, context);
		std::cout << "Verifying: " << subject_name << "\nVerified: " << verified << std::endl;

		return verified;
	}
}
