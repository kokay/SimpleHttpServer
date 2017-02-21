//
// Created by koji on 2/9/17.
//

#ifndef SIMPLEHTTPSERVER_SIMPLEHTTPSERVER_HPP
#define SIMPLEHTTPSERVER_SIMPLEHTTPSERVER_HPP

#include <boost/asio.hpp>
#include <thread>
//#include "HttpHandler.hpp"
#include "DatabaseHandler.hpp"


using namespace std;
using namespace boost;

class HttpHandler;

class SimpleHttpServer {
public:
    SimpleHttpServer(const unsigned short portNum, const unsigned int threadPoolSize, const string& rootDir);
    void Start();
    void Stop();
private:
    void InitAccept();
    void onAccept(const system::error_code& ec, std::shared_ptr<asio::ip::tcp::socket> socket);

    asio::io_service ioService;
    asio::ip::tcp::acceptor acceptor;

    unique_ptr<asio::io_service::work> work;
    vector<unique_ptr<thread>> threadPool;

    unsigned short portNum;
    unsigned int threadPoolSize;

    string rootDir;
};

#endif //SIMPLEHTTPSERVER_SIMPLEHTTPSERVER_HPP
