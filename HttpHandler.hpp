//
// Created by koji on 2/10/17.
//

#ifndef SIMPLEHTTPSERVER_HTTPHANDLER_HPP
#define SIMPLEHTTPSERVER_HTTPHANDLER_HPP

#include <boost/asio.hpp>

using namespace std;
using namespace boost;

class SimpleHttpServer;
class HttpHandler {
public:
    HttpHandler(std::shared_ptr<asio::ip::tcp::socket> socket, const string& rootDir);
    void Start();
    const string&  getRootDir();
    const string  getRemoteIpAddress();
    const string& getMethod();
    const string& getRequestUri();
    const string& getHttpVersion();
    string getRequestHeaderValue(const string& headerName);
    DatabaseHandler* getDatabaseHandler();

private:
    void onRequestLineReceived(const system::error_code& ec, size_t byteTransferred);
    void onRequestHeadersReceived(const system::error_code& ec, size_t byteTransferred);
    void onRequestPostBodyReceived(const system::error_code& ec, size_t bytesTransferred);
    void parsePostBody();
    long long removeBoundary(const string& boundary, const char* buf, long long size);

    void processRequest();
    void sendResponse();
    void onResponseSent(const system::error_code& ec, size_t bytesTransferred);
    void onFinish();

    std::shared_ptr<asio::ip::tcp::socket> socket;

    asio::streambuf requestBuf;
    string method, requestUri, httpVersion;
    map<string, string> requestHeaders;

    unsigned int responseStatusCode;
    string responseStatusLine;
    string responseHeaders;

    size_t resourceSizeBytes;
    std::unique_ptr<char[]> resourceBuf;

    const string& rootDir;
    std::unique_ptr<DatabaseHandler> databaseHandler;
    static const map<unsigned int, string> responseStatuses;
};

#endif //SIMPLEHTTPSERVER_HTTPHANDLER_HPP
