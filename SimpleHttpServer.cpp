//
// Created by koji on 2/10/17.
//

#include <boost/filesystem/operations.hpp>
#include <iostream>

#include "SimpleHttpServer.hpp"
#include "HttpHandler.hpp"

SimpleHttpServer::SimpleHttpServer(const unsigned short portNum,
                                   const unsigned int threadPoolSize, const string& rootDir)
    : acceptor(ioService, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), portNum))
    , context(asio::ssl::context::sslv23)
    , portNum(portNum)
    , threadPoolSize(threadPoolSize)
    , rootDir(rootDir)
{
    work.reset(new asio::io_service::work(ioService));
    context.set_options(
            asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::no_tlsv1_1 |
            asio::ssl::context::single_dh_use
    );
    context.set_password_callback(bind(&SimpleHttpServer::getPassword, this));
    context.use_certificate_chain_file("server.pem");
    context.use_private_key_file("server.pem", asio::ssl::context::pem);
    context.use_tmp_dh_file("dh2048.pem");
}

void SimpleHttpServer::Start() {
    system::error_code ec;
    rootDir = filesystem::canonical(rootDir, ec).generic_string();
    if (ec != 0) {
        cout << "Error occurred - SimpleHttpServer::Start" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << rootDir << " " << ec.message() << endl;
        cout << endl;
        return;
    } else if (!filesystem::is_directory(rootDir)) {
        cout << "Error occurred - SimpleHttpServer::Start" << endl;
        cout << "  Error code - " << "NONE" << endl;
        cout << "  Message    - " << rootDir << " is not directory" << endl;
        cout << endl;
        return;
    }

    acceptor.listen();
    InitAccept();

    for (unsigned int i = 0; i < threadPoolSize; ++i) {
        std::unique_ptr<std::thread> thread(new std::thread([this]() {
            for(;;) {
                try {
                    ioService.run();
                } catch(const std::exception& ec) {
                    cout << "Error occurred - SimpleHttpServer::Start" << endl;
                    cout << "  Error code - " << ec.what() << endl;
                    cout << endl;
                }
            }
        }));
        threadPool.push_back(move(thread));
    }

    // check this
    for (auto& thread : threadPool) {
        thread->join();
    }
}

string SimpleHttpServer::getPassword() {
    return "tmp";
}


void SimpleHttpServer::Stop() {
    acceptor.cancel();
    ioService.stop();
    for (auto& thread : threadPool) {
        thread->join();
    }
}

void SimpleHttpServer::InitAccept() {
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>
            socket(new asio::ssl::stream<asio::ip::tcp::socket>(ioService, context));
    acceptor.async_accept(socket.get()->lowest_layer(),
                          [this, socket](const system::error_code& ec) { onAccept(ec, socket); });
}

void SimpleHttpServer::onAccept(const system::error_code& ec, std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> socket) {
    if (ec != 0) {
        cout << "Error occurred - Acceptor::onAccept" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;
    } else {
        (new HttpHandler(socket, rootDir))->Start();
    }

    InitAccept();
}

