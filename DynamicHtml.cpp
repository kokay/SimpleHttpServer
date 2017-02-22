//
// Created by koji on 2/16/17.
//
#include <boost/filesystem.hpp>
#include "DynamicHtml.hpp"
#include "HttpHandler.hpp"
#include "DatabaseHandler.hpp"

using namespace boost;

vector<pair<string, string>> DynamicHtml::parseQuery(const string& queryString) {
    vector<pair<string, string>> queries;
    istringstream iss(queryString);
    string query;
    while(getline(iss, query, '&')){
        int sep = query.find("=");
        if (sep != string::npos && sep != query.size() - 1)
            queries.push_back({query.substr(0, sep), query.substr(sep + 1)});
    }
    return queries;
};

string DynamicHtml::getDynamicHtmlPage(HttpHandler* handler) {
    const string& requestUri = handler->getRequestUri();
    int idx = requestUri.find("?");

    string pageName, queryString;
    if (idx != string::npos && idx + 1 <= requestUri.size()) {
        pageName = requestUri.substr(0, idx);
        queryString = requestUri.substr(idx + 1);
    } else {
        pageName = requestUri;
    }

    if (pageName == "/files.html") {
        return getFilesPage(queryString, handler->getRootDir());
    } else if (pageName == "/log.html"){
        return getRequestLogPage(queryString, handler->getDatabaseHandler());
    }
    return "";
}

bool DynamicHtml::isValidDirectoryName(const string& dirName) {
    for (const char c : dirName) {
        if (!(c >= 'A' && c <= 'Z') &&
            !(c >= 'a' && c <= 'z') &&
            !(c >= '0' && c <= '9') &&
            c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}

bool DynamicHtml::createDirectory(const string& rootDir, const string& dirName) {
    if (isValidDirectoryName(dirName))
        return filesystem::create_directories(rootDir + "/" + dirName);

    return false;
}

string DynamicHtml::getFilesPage(const string& queryString, const string& rootDir) {
    bool successCreatingDir = true;
    for (const pair<string, string>& query : parseQuery(queryString)) {
        if (query.first == "make_dir") {
            successCreatingDir = createDirectory(rootDir, query.second);
        }
    }

    string filesPage = { u8""
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<meta charset='UTF-8'>"
            "<title>Files</title>"
            "<link href='./layout.css' rel='stylesheet' />"
        "</head>"
        "<body>"
            "<header>"
                 "<h2>Files</h2>"
            "</header>"

            "<nav>"
                "<a href='./files.html'>File(s) Page</a>"
                "<a href='./log.html'>Request Log Page</a>"
                "<a href='./me.html'>About Me</a>"
            "</nav>"

            "<aside>"
                "<form action='./files.html'>"
                    "<h4>Make Folder</h4>"
                    "<input type='text' name='make_dir' placeholder=' Folder Name'>"
                    "<input type='submit' value='Make Folder'/>"
    };

    if (!successCreatingDir) {
        filesPage += "<p class='status'>Making Folder fails.</p>";
        filesPage += "<p class='status'>Note : A-Z, a-z, 0-9, _, - can be only used for the folder name. "
                "If the folder already exists, then new folder can not be made.</p>";
    }

    filesPage += {
                "</form>"
                "<form action='./files.html' method='post' enctype='multipart/form-data'>"
                    "<h4>Upload File (Choose Folder)</h4>"
    };

    vector<filesystem::directory_entry> directories{ filesystem::directory_entry(filesystem::path(rootDir)) };
    for (auto& de : filesystem::recursive_directory_iterator(rootDir))
        if (filesystem::is_directory(de)) directories.push_back(de);

    for (auto& dir : directories) {
        string dirPath = dir.path().generic_string().substr(rootDir.size()) + "/";
        filesPage += "<label><input type=radio name='dir' value='" + dirPath + "'>in '" + dirPath + "'</label>";
    }

    filesPage += {
                    "<input id='button' type='file' name='file' value='Choose File'/>"
                    "<input id='button' type='submit' value='Upload File'/>"
                "</form>"
            "</aside>"

            "<main>"

    };

    for (auto& dir : directories) {
        string dirPath = dir.path().generic_string().substr(rootDir.size()) + "/";
        filesPage += "<article><h4>" + dirPath + "</h4>";
        for (auto& de : filesystem::directory_iterator(dir)) {
            if (filesystem::is_regular_file(de)) {
                string path = de.path().filename().string();
                filesPage += "<p><a href='." + dirPath + path + "'>" + path + "</a></p>";
            }
        }
        filesPage += "</article>";
    }

    filesPage += {
            "</article>"
            "</main>"
        "</body>"
        "</html>"
    };
    return filesPage;
}


string DynamicHtml::getRequestLogPage(const string& queryString, DatabaseHandler* databaseHandler) {
    string logPage = {u8""
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
          "<meta charset='UTF-8'>"
          "<title>Request Log</title>"
          "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>"
          "<script src='./log.js'></script>"
          "<link href='./layout.css' rel='stylesheet' />"
      "</head>"
      "<body>"
          "<header>"
          "<h2>Request Log</h2>"
          "</header>"

          "<nav>"
          "<a href='./files.html'>File(s) Page</a>"
          "<a href='./log.html'>Request Log Page</a>"
          "<a href='./me.html'>About Me</a>"
          "</nav>"
                "<aside>"
                    "<form action='./log.html' method='get'>"
                        "<h4>Search</h4>"
                        "<input type='text' name='search' placeholder='Search Word'>"
                        "<label><input id='allCheckbox' type='checkbox'>                  All</label>"
                        "<label><input id='requestLineCheckbox' type='checkbox'>       Request Line</label>"
                        "<label><input class='requestLine' type='checkbox' name='method'>       Method</label>"
                        "<label><input class='requestLine' type='checkbox' name='request_uri'>          URI</label>"
                        "<label><input class='requestLine' type='checkbox' name='http_version'> HTTP Version</label>"
                        "<label><input id='headersCheckbox' type='checkbox'>       Headers</label>"
                        "<label><input class='headers' type='checkbox' name='accept'>               Accpet</label>"
                        "<label><input class='headers' type='checkbox' name='accept_encoding'>      Accept-Encoding</label>"
                        "<label><input class='headers' type='checkbox' name='accept_language'>      Accept-Language</label>"
                        "<label><input class='headers' type='checkbox' name='connection'>           Connection</label>"
                        "<label><input class='headers' type='checkbox' name='host'>                 Host</label>"
                        "<label><input class='headers' type='checkbox' name='user_agent'>           User-Agent</label>"
                        "<label><input class='headers' type='checkbox' name='access_time'>          Access Time</label>"
                        "<input type='reset' value='Reset'/>"
                        "<input type='submit' value='Search'/>"
                    "</form>"
                "</aside>"
          "<main><article>"
    };

    vector<vector<string>> res;
    int numRows = databaseHandler->getQueryResults(parseQuery(queryString), &res);
    if (numRows == -1) {
        logPage += "<h4>Search Result - Error Occurred</h4><table></table>";
    } else if (numRows == 0) {
        logPage += "<h4>Search Result - " + to_string(numRows) + " rows found</h4><table></table>";
    } else {
        logPage += "<h4>Search Result - " + to_string(numRows) + " rows found</h4>";
        logPage += "<table><tr>";
        for (string& header : res[0])
            logPage += "<th>" + header + "</th>";
        logPage += "</tr>";

        for (int i = 1; i < res.size(); ++i) {
            logPage += "<tr>";
            for (string& rowData : res[i]) {
                logPage += "<td>" + rowData + "</td>";
            }
            logPage += "</tr>";
        }
        logPage += "</table>";
        logPage += "<p>";

        int idx = queryString.find("&row=");
        int i = 0;
        for (; i < numRows - 100; i += 100) {
            logPage += "<a href='./log.html?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
                       + to_string(i + 1) + " - " + to_string(i + 100) + "</a>";
        }
        logPage += "<a href='./log.html?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
                   + to_string(i + 1) + " - " + to_string(numRows) + "</a>";
        logPage += "</p>";
    }

    logPage += {
        "</article></main>"
    "</body>"
    "</html>"
    };

    return logPage;
}
