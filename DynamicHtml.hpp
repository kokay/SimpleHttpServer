//
// Created by koji on 2/16/17.
//

#ifndef SIMPLEHTTPSERVER_DYNAMICHTML_HPP
#define SIMPLEHTTPSERVER_DYNAMICHTML_HPP


#include <string>
#include <vector>
#include <map>

using namespace std;

class HttpHandler;
class DatabaseHandler;
class DynamicHtml {
public:
    static string getDynamicHtmlPage(HttpHandler* handler);
    static bool createDirectory(const string& rootDir, const string& dirName);
    static bool isValidFileName(const string& name);
private:
    static vector<pair<string, string>> parseQuery(const string& queryString);

    static string getHtmlPage(const string& pageTitle, const string& headExtra, const string& main);

    static string getDirsPage(const string& queryString, const string& rootDir);
    static string getFilesPage(const string& rootDir, const string& dirName);
    static string getFileTable(const string& rootDir, const string& dirName);

    static string getRequestLogPage(const string& queryString, DatabaseHandler* databaseHandler);
    static string getRequestLogTable(const string& queryString, DatabaseHandler* databaseHandler);

    static const string FILE_DIR;
};


#endif //SIMPLEHTTPSERVER_DYNAMICHTML_HPP
