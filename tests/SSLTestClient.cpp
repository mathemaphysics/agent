#include <iostream>
#include <thread>
#include <chrono>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/NetException.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
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

    Poco::Net::Context::Ptr context = new Poco::Net::Context(
        Poco::Net::Context::Usage::CLIENT_USE,
        "/workspaces/certs/client_key_test.pem",
        "/workspaces/certs/client_certificate_test.pem",
        "/workspaces/certs/ca_certificate_test.pem",
        Poco::Net::Context::VERIFY_NONE
    );

    _logger->info("Opening socket");
    auto socketAddress = Poco::Net::SocketAddress("localhost", 8000);
    Poco::Net::SecureStreamSocket socket = Poco::Net::SecureStreamSocket(
        socketAddress,
        context
    );
    //Poco::Net::StreamSocket socket(socketAddress);
    auto str = Poco::Net::SocketStream(socket);

    char buffer[1024*1024];
    while(1)
    {
        try
        {
            auto in = socket.receiveBytes(buffer, 1024);
            if (in > 0)
            {
                std::cout << buffer << std::endl
                        << std::flush;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        catch(const Poco::Exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    _logger->info("Cleaning up SSL");
    Poco::Net::uninitializeSSL();
}
