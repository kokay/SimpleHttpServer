//
// Created by koji on 2/10/17.
//

#include <iostream>
#include <boost/filesystem.hpp>
#include <fstream>
#include "SimpleHttpServer.hpp"
#include "HttpHandler.hpp"
#include "DynamicHtml.hpp"

HttpHandler::HttpHandler(std::shared_ptr<asio::ip::tcp::socket> socket, const string& rootDir)
    : socket(socket)
    , requestBuf(104857600)
    , responseStatusCode(200)
    , resourceSizeBytes(0)
    , rootDir(rootDir)
    , databaseHandler(new DatabaseHandler("simple_http_server", "request_log"))
{}

void HttpHandler::Start() {
    asio::async_read_until(*socket.get(), requestBuf, "\r\n",
                    [this] (const system::error_code& ec, size_t byteTransferred)
                    { onRequestLineReceived(ec, byteTransferred); });
}


void HttpHandler::onRequestLineReceived(const system::error_code& ec, size_t byteTransferred) {
    if (ec != 0) {
        cout << "Error occurred - HttpHandler::onRequestLineReceived" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;

        if (ec == asio::error::not_found) {
            responseStatusCode = 413;
            sendResponse();
            return;
        } else {
            onFinish();
            return;
        }
    }

    string requestLine;
    istream requestStream(&requestBuf);
    getline(requestStream, requestLine, '\r');
    requestStream.get();

    istringstream requestLineStream(requestLine);

    requestLineStream >> method;
    if (method != "GET" && method != "POST") {
        responseStatusCode = 501;
        sendResponse();
        return;
    }

    requestLineStream >> requestUri;

    requestLineStream >> httpVersion;
    if (httpVersion != "HTTP/1.1") {
        responseStatusCode = 505;
        sendResponse();
        return;
    }

    asio::async_read_until(*socket.get(), requestBuf, "\r\n\r\n",
                   [this] (const system::error_code& ec, size_t byteTransferred)
                   { onRequestHeadersReceived(ec, byteTransferred); });
}

void HttpHandler::onRequestHeadersReceived(const system::error_code& ec, size_t byteTransferred) {
    if (ec != 0) {
        cout << "Error occurred - HttpHandler::onRequestHeadersReceived" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;

        if (ec == asio::error::not_found) {
            responseStatusCode = 413;
            sendResponse();
            return;
        } else {
            onFinish();
            return;
        }
    }

    string headerLine, headerField, headerValue;
    istream requestStream(&requestBuf);
    while(getline(requestStream, headerLine, '\r')) {
        requestStream.get();
        if (headerLine.empty()) break;
        istringstream iss(headerLine);
        if (!getline(iss, headerField, ':')) break;
        if (!getline(iss, headerValue)) break;

        requestHeaders[headerField] = headerValue;
    }

    if (method == "POST") {
        long long size = stoll(requestHeaders["Content-Length"]);
        asio::async_read(*socket.get(), requestBuf, asio::transfer_exactly(size - requestBuf.in_avail()),
                         [this] (const system::error_code& ec, size_t byteTransferred)
                         { onRequestPostBodyReceived(ec, byteTransferred); });
    } else {
        processRequest();
    }
}

void HttpHandler::parsePostBody() {
    istream requestStream(&requestBuf);
    string subDir, fileName;
    string headerField, headerValue;
    string line, preLine;

    int idx = requestHeaders["Content-Type"].find_last_of("-");
    string boundary = requestHeaders["Content-Type"].substr(idx + 1);
    while(getline(requestStream, line)) if (line.find(boundary) != string::npos) break;
    while(getline(requestStream, line, '\r')) {
        requestStream.get();
        if (line.empty()) break;
    }
    requestStream >> subDir;
    if (subDir.empty() || subDir.front() != '/' || subDir.back() != '/') {
        return;
    }

    string filename = "filename=\"";
    while(getline(requestStream, line)) if (line.find(boundary) != string::npos) break;
    while(getline(requestStream, line, '\r')) {
        requestStream.get();
        if (line.empty()) break;

        istringstream iss(line);
        if (!getline(iss, headerField, ':')) break;
        if (!getline(iss, headerValue)) break;

        if (headerField == "Content-Disposition") {
            int idx = headerValue.find(filename);
            fileName = headerValue.substr(idx + filename.size());
            fileName.pop_back();
        }
    }
    if (fileName.empty()) {
        return;
    }

    long long size = requestBuf.in_avail();
    unique_ptr<char[]> buf (new char[size]);
    requestStream.read(buf.get(), size);

    size = removeBoundary(boundary, buf.get(), size);
    if (size == -1) {
        return;
    }

    std::ofstream ofs(rootDir + subDir + fileName, std::ofstream::binary);
    if (!ofs.is_open()) {
        return;
    }
    ofs.write(buf.get(), size);
    ofs.close();
}

long long HttpHandler::removeBoundary(const string& boundary, const char* buf, long long size) {
    size -= boundary.size();

    bool found = true;
    for(int i = 0; size >= boundary.size(), i < 30; ++i) {
        found = true;
        for (long long i = 0; i < boundary.size(); ++i ) {
            if (buf[size + i] != boundary[i]) {
                found = false; break;
            }
        }
        if (found) break;
        size--;
    }
    if (!found) return -1;

    size--;
    while(buf[size] == '-') size--;

    if (buf[size] == '\n') size--;
    else return -1;
    if (buf[size] == '\r') return size;
    else return -1;
}



void HttpHandler::onRequestPostBodyReceived(const system::error_code& ec, size_t bytesTransferred) {
    if (ec != 0) {
        cout << "Error occurred - HttpHandler::onRequestPostBodyReceived" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;
    }

    parsePostBody();
    processRequest();
}


void HttpHandler::processRequest() {

    string dynamicHtmlPage = DynamicHtml::getDynamicHtmlPage(this);
    if (!dynamicHtmlPage.empty()) {
        resourceSizeBytes = dynamicHtmlPage.size();
        resourceBuf.reset(new char[resourceSizeBytes]);
        strncpy(resourceBuf.get(), dynamicHtmlPage.c_str(), resourceSizeBytes);
        sendResponse();
        return;
    }

    string resourceUri = rootDir + requestUri;
    if (!filesystem::exists(resourceUri)) {
        responseStatusCode = 404;
        sendResponse();
        return;
    }

    resourceUri = filesystem::canonical(resourceUri).generic_string();
    if (resourceUri.substr(0, rootDir.size()) != rootDir || !filesystem::is_regular_file(resourceUri)) {
        responseStatusCode = 404;
        sendResponse();
        return;
    }

    ifstream ifs(resourceUri, ifstream::binary);
    if (!ifs.is_open()) {
        responseStatusCode = 500;
        sendResponse();
        return;
    }

    resourceSizeBytes = filesystem::file_size(resourceUri);
    resourceBuf.reset(new char[resourceSizeBytes]);
    ifs.read(resourceBuf.get(), resourceSizeBytes);
    sendResponse();
}


void HttpHandler::sendResponse() {
    socket->shutdown(asio::ip::tcp::socket::shutdown_receive);

    responseStatusLine = "HTTP/1.1 " + responseStatuses.at(responseStatusCode) + "\r\n";
    if (resourceSizeBytes != 0)
        responseHeaders += "content-length: " + to_string(resourceSizeBytes) + "\r\n";
    responseHeaders += "\r\n";

    vector<asio::const_buffer> responseBuf {
            asio::buffer(responseStatusLine),
            asio::buffer(responseHeaders),
            asio::buffer(resourceBuf.get(), resourceSizeBytes)
    };

    asio::async_write(*socket.get(), responseBuf,
                      [this](const system::error_code& ec, size_t bytesTransferred)
                      { onResponseSent(ec, bytesTransferred); });
}

void HttpHandler::onResponseSent(const system::error_code& ec, size_t bytesTransferred) {
    if (ec != 0) {
        cout << "Error occurred - HttpHandler::onResponseSent" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;
    }

    databaseHandler->insertLog(this);
    socket->shutdown(asio::ip::tcp::socket::shutdown_both);
    onFinish();
}

void HttpHandler::onFinish() {
    delete this;
}

const map<unsigned int, string> HttpHandler::responseStatuses = {
        { 200, "200 OK" },
        { 404, "404 Not Found" },
        { 413, "413 Request Entity Too Large" },
        { 500, "500 Server Error" },
        { 501, "501 Not Implemented" },
        { 505, "505 HTTP Version Not Supported" }
};

const string  HttpHandler::getRemoteIpAddress() {
    system::error_code ec;
    string remoteIpAddress = socket->remote_endpoint(ec).address().to_string(ec);
    if (ec != 0) {
        cout << "Error occurred - HttpHandler::getRemoteIpAddress" << endl;
        cout << "  Error code - " << ec.value() << endl;
        cout << "  Message    - " << ec.message() << endl;
        cout << endl;
    }
    return remoteIpAddress;
}

const string& HttpHandler::getMethod() {
    return method;
}

const string& HttpHandler::getRequestUri() {
    return requestUri;
}

const string& HttpHandler::getHttpVersion() {
    return httpVersion;
}

string HttpHandler::getRequestHeaderValue(const string& headerField) {
    auto it = requestHeaders.find(headerField);
    if (it != requestHeaders.end()) {
        return it->second;
    } else {
        return "";
    }
}

const string& HttpHandler::getRootDir() {
    return rootDir;
}

DatabaseHandler* HttpHandler::getDatabaseHandler() {
    return databaseHandler.get();
}
