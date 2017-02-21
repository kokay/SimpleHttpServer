//
// Created by koji on 2/13/17.
//

#ifndef SIMPLEHTTPSERVER_DATABASEHANDLER_HPP
#define SIMPLEHTTPSERVER_DATABASEHANDLER_HPP


#include <cppconn/driver.h>
#include <cppconn/resultset.h>
using namespace std;

class HttpHandler;
class DatabaseHandler {
public:
    static string databasePassword;

    DatabaseHandler(const string& databaseName, const string& tableName);
    ~DatabaseHandler();

    void insertLog(HttpHandler* handler);
    int getQueryResults(const vector<pair<string, string>>& query, vector<vector<string>>* rows);

private:
    void createDatabase(const string& databaseName);
    void useDatabase(const string& databaseName);
    void createTable(const string& tableName);

    sql::Driver* driver;
    sql::Connection* con;

    string databaseName;
    string tableName;
};


#endif //SIMPLEHTTPSERVER_DATABASEHANDLER_HPP
