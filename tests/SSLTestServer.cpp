#include <iostream>
#include <thread>
#include <chrono>
#include <Poco/SharedPtr.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/Session.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SecureStreamSocketImpl.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/KeyFileHandler.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char **argv)
{
    auto _logger = spdlog::stdout_color_mt("Client");

    _logger->info("Initializing SSL");
    Poco::Net::initializeSSL();

    try
    {
        Poco::Net::Context::Ptr context = new Poco::Net::Context(
            Poco::Net::Context::Usage::SERVER_USE,
            "/workspaces/certs/server_key_test.pem",
            "/workspaces/certs/server_certificate_test.pem",
            "/workspaces/certs/ca_certificate_test.pem",
            Poco::Net::Context::VERIFY_NONE
        );

        Poco::Net::SSLManager::InvalidCertificateHandlerPtr ptrHandler(new Poco::Net::AcceptCertificateHandler(false));
        Poco::Net::SSLManager::instance().initializeServer(0, ptrHandler, context);

        Poco::Net::SecureServerSocket socket = Poco::Net::SecureServerSocket(
            8000,
            64,
            context
        );
        //Poco::Net::ServerSocket socket(8000);

        while (1)
        {
            auto conn = socket.acceptConnection();
            auto sconn = static_cast<Poco::Net::SecureStreamSocket>(conn);
            sconn.completeHandshake();
            Poco::Net::SocketStream str(sconn);
            //Poco::Net::SocketStream str(conn);
            str << "This is a message" << std::flush;
        }
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::minutes(10));

    _logger->info("Cleaning up SSL");
    Poco::Net::uninitializeSSL();
}
