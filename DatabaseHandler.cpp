//
// Created by koji on 2/13/17.
//
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "DatabaseHandler.hpp"
#include "HttpHandler.hpp"

string DatabaseHandler::databasePassword;

DatabaseHandler::DatabaseHandler(const string &databaseName, const string &tableName) {
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "simpleHttpServer", "1234321");

    DatabaseHandler::databaseName = databaseName;
    DatabaseHandler::tableName = tableName;

    //createDatabase(databaseName);
    useDatabase(databaseName);
    //createTable(tableName);
}

DatabaseHandler::~DatabaseHandler(){
    delete con;
}

void DatabaseHandler::createDatabase(const string& databaseName) {
    sql::Statement *stmt;
    stmt = con->createStatement();
    stmt->execute("CREATE DATABASE IF NOT EXISTS " + databaseName);
    delete stmt;
}

void DatabaseHandler::useDatabase(const string& databaseName) {
    sql::Statement *stmt;
    stmt = con->createStatement();
    stmt->execute("USE " + databaseName);
    delete stmt;
}

void DatabaseHandler::createTable(const string& tableName) {
    sql::Statement *stmt;
    stmt = con->createStatement();
    stmt->execute(
            "CREATE TABLE IF NOT EXISTS " + tableName + " (" +
            "   id INT NOT NULL AUTO_INCREMENT, " +
            "   remote_address VARCHAR(15) NOT NULL, " +
            "   method VARCHAR(7) NOT NULL, " +
            "   request_uri VARCHAR(255) NOT NULL, " +
            "   http_version VARCHAR(8) NOT NULL, " +
            "   accept VARCHAR(100), " +
            "   accept_encoding VARCHAR(100), " +
            "   accept_language VARCHAR(100), " +
            "   connection VARCHAR(12), " +
            "   host VARCHAR(21), " +
            "   user_agent VARCHAR(200), " +
            "   access_time DATETIME NOT NULL, " +
            "   PRIMARY KEY ( id ));"
    );
    delete stmt;
}

vector<string> headerFieldNames = {
        "", // 0 should be empty
        "", // 1 remote_address, should be empty
        "", // 2 method, should be empty
        "", // 3 request_uri, should be empty
        "", // 4 http_version, should be empty
        "Accept", // 5
        "Accept-Encoding", // 6
        "Accept-Language", // 7
        "Connection", // 8
        "Host", // 9
        "User-Agent", // 10
};


int startIdx = 5;

void DatabaseHandler::insertLog(HttpHandler* handler) {
    try {
        sql::PreparedStatement* prep_stmt = con->prepareStatement(
                "INSERT INTO " + tableName +
                "  ( remote_address,  method,  request_uri,  http_version, " +
                "    accept, accept_encoding, accept_language, connection, " +
                "    host, user_agent, access_time )" +
                "  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())"
        );
        prep_stmt->setString(1, handler->getRemoteIpAddress());
        prep_stmt->setString(2, handler->getMethod());
        prep_stmt->setString(3, handler->getRequestUri());
        prep_stmt->setString(4, handler->getHttpVersion());
        for (int i = startIdx; i < headerFieldNames.size(); ++ i) {
            string value = handler->getRequestHeaderValue(headerFieldNames[i]);
            if (value.empty()) value = "null";
            prep_stmt->setString(i, value);
        }
        prep_stmt->execute();
        delete prep_stmt;
    } catch (sql::SQLException& ec) {
        cout << "Error occurred - DatabaseHandler::insertLog" << endl;
        cout << "  Error code - " << ec.getErrorCode() << endl;
        cout << "  Message    - " << ec.what() << endl;
        cout << endl;
    }
}

vector<string> requestLogColumns = {
        "remote_address",
        "method",
        "request_uri",
        "http_version",
        "accept",
        "accept_encoding",
        "accept_language",
        "connection",
        "host",
        "user_agent",
        "access_time"
};

int DatabaseHandler::getQueryResults(const vector<pair<string, string>>& queries, vector<vector<string>>* rows) {
    string search = "";
    int row = 0;
    int totalRows = 0;

    vector<string> headers;
    for (const pair<string, string>& query : queries) {
        if (query.first == "search") {
            search = query.second;
        } else if (query.first == "row") {
            row = stoi(query.second);
        } else if (query.second == "on" &&
                find(requestLogColumns.begin(), requestLogColumns.end(), query.first) != requestLogColumns.end()) {
            headers.push_back(query.first);
        }
    }

    if (headers.size() != 0) {
        try {
            rows->push_back(headers);
            string sqlQuery = "SELECT ";
            for (int i = 0; i < headers.size() - 1; ++i) {
                sqlQuery += headers[i] + ",";
            }
            sqlQuery += headers[headers.size() - 1];
            sqlQuery += " FROM " + tableName;

            if (!search.empty()) {
                sqlQuery += " WHERE ";
                for (int i = 0; i < headers.size() - 1; ++i) {
                    sqlQuery += headers[i] + " LIKE '%" + search + "%' OR ";
                }
                sqlQuery += headers[headers.size() - 1] + " LIKE '%" + search + "%'";

            }

            sql::Statement* stmt = con->createStatement();
            sql::ResultSet* res = stmt->executeQuery(sqlQuery);

            totalRows = res->rowsCount();
            res->relative(row);
            for(int i = 0; i < 100 && res->next(); ++i) {
                vector<string> row;
                for (int i = 1; i <= headers.size(); ++i) {
                    row.push_back(res->getString(i));
                }
                rows->push_back(row);
            }

            delete res;
            delete stmt;
        } catch (sql::SQLException& ec) {
            cout << "Error occurred - DatabaseHandler::getQueryResultTableHtmlCode" << endl;
            cout << "  Error code - " << ec.getErrorCode() << endl;
            cout << "  Message    - " << ec.what() << endl;
            cout << endl;
            return -1;
        }
    }
    return totalRows;
}

