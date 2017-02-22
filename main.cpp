#include <iostream>
#include "SimpleHttpServer.hpp"

int main() {
    unsigned short portNum = 5555;
    unsigned int threadPoolSize = std::thread::hardware_concurrency();
    if (threadPoolSize == 0) threadPoolSize = 2;
    string rootDir = "../www";

    try {
        cout << "Enter database password:";
        cin >> DatabaseHandler::databasePassword;

        SimpleHttpServer simpleHttpServer(portNum, threadPoolSize, rootDir);
        cout << "Simple HTTP Serer starts." << endl;
        simpleHttpServer.Start();

        cout << "Please type stop when it needs to be stopped." << endl;
        string stop;
        while(true) {
            cin >> stop;
            if (stop == "stop") break;
        }
        cout << "Simple HTTP Serer stops." << endl;
        simpleHttpServer.Stop();
    } catch (system::system_error& ec) {
        cout << "Error occurred." << endl;
        cout << "  Error code - " << ec.code() << endl;
        cout << "  Message    - " << ec.what() << endl;
        cout << endl;
    }
    return 0;
}