#include <iostream>
#include <thread>
#include <chrono>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/NetException.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char **argv)
{
    auto _logger = spdlog::stdout_color_mt("Client");

    _logger->info("Initializing SSL");
    Poco::Net::initializeSSL();

    Poco::Net::Context::Ptr context = new Poco::Net::Context(
        Poco::Net::Context::Usage::CLIENT_USE,
        "/workspaces/certs/client_key.pem",
        "/workspaces/certs/client_certificate.pem",
        "/workspaces/certs/ca_certificate.pem",
        Poco::Net::Context::VERIFY_NONE,
        9,
        false,
        "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
    );

    _logger->info("Opening socket");
    auto socketAddress = Poco::Net::SocketAddress("172.18.0.2", 5671);
    Poco::Net::SecureStreamSocket socket = Poco::Net::SecureStreamSocket(
        socketAddress,
        context
    );

    //int res = socket.sendBytes("Something", 10);
    //std::this_thread::sleep_for(std::chrono::minutes(2));

    _logger->info("Available to read: {}", socket.available());

    _logger->info("Cleaning up SSL");
    Poco::Net::uninitializeSSL();
}
