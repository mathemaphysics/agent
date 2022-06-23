#include <iostream>
#include <thread>
#include <chrono>
#include <Poco/SharedPtr.h>
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

class EchoConnection: public Poco::Net::TCPServerConnection
{
public:
   EchoConnection(const Poco::Net::StreamSocket& s): Poco::Net::TCPServerConnection(s){}

   void run()
   {
       Poco::Net::SecureStreamSocket ss = (Poco::Net::SecureStreamSocket)dynamic_cast<const Poco::Net::StreamSocket&>(socket());
       ss.completeHandshake();
       if (ss.secure())
          std::cout << "Connection is secure" << std::endl;
       std::cout << "Connection from client: " << ss.address() << std::endl;
       try
       {
           std::cout << "Here's something" << std::endl;
       }
       catch (Poco::Exception& exc)
       {
           std::cerr << "--> EchoConnection: " << exc.displayText() << std::endl;
       }

       Poco::Buffer<char> tempData(1024*1024);

        int i = 0;
       while (i < 3)
       {
        std::cout << "Ret: " << ss.receiveBytes(tempData) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++i;
       }
       ss.close();
   }
};

class MyPassphraseHandler : public Poco::Net::PrivateKeyPassphraseHandler
{
public:
    MyPassphraseHandler() : Poco::Net::PrivateKeyPassphraseHandler(true) {}

    void onPrivateKeyRequested(const void* pSender, std::string& privateKey)
    {
        std::cout << "Private key is requested" << std::endl;
    }
};

int main(int argc, char **argv)
{
    auto _logger = spdlog::stdout_color_mt("Client");

    _logger->info("Initializing SSL");
    Poco::Net::initializeSSL();

    Poco::Net::Context::Ptr context = new Poco::Net::Context(
        Poco::Net::Context::Usage::SERVER_USE,
        "/workspaces/certs/server_key.pem",
        "/workspaces/certs/server_certificate.pem",
        "/workspaces/certs/ca_certificate.pem",
        Poco::Net::Context::VERIFY_RELAXED
    );

    Poco::Net::SSLManager::PrivateKeyPassphraseHandlerPtr qtrHandler(new MyPassphraseHandler());
    Poco::Net::SSLManager::InvalidCertificateHandlerPtr ptrHandler(new Poco::Net::AcceptCertificateHandler(false));
    Poco::Net::SSLManager::instance().initializeServer(0, ptrHandler, context);

    auto serverAddress = Poco::Net::SocketAddress("dcdfdef02f23", 8000);
    Poco::Net::SecureServerSocket socket = Poco::Net::SecureServerSocket(
        serverAddress,
        64,
        context
    );



    Poco::Net::TCPServer *server;
    try
    {
        /* code */
        server = new Poco::Net::TCPServer(new Poco::Net::TCPServerConnectionFactoryImpl<EchoConnection>(), socket);
    }
    catch(const Poco::Exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    server->start();
    std::this_thread::sleep_for(std::chrono::minutes(10));

    _logger->info("Cleaning up SSL");
    Poco::Net::uninitializeSSL();
}
