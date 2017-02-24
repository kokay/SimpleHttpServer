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

    if (pageName == "/files") {
        return getFilesPage(queryString, handler->getRootDir());
    } else if (pageName == "/log"){
        return getRequestLogPage(queryString, handler->getDatabaseHandler());
    }
    return "";
}

string DynamicHtml::getHtmlPage(const string& pageTitle, const string& headExtra, const string& main) {
    return { u8""
        "<html>"
        "<head>"
            "<meta charset='UTF-8'>"
            "<title>" + pageTitle + "</title>"
            + headExtra +
            "<link href='./layout.css' rel='stylesheet' />"
        "</head>"
        "<body>"
        "<header>"
            "<nav>"
                "<h1>" + pageTitle + "</h1>"
                "<a href='./files'>Files</a>"
                "<a href='./log'>Request Log</a>"
                "<a href='./projects.html'>Personal Projects</a>"
            "</nav>"
        "</header>"
        "<main>" + main + "</main>"
        "</body>"
        "</html>"
    };
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

    string status = "";
    if (!successCreatingDir) {
        status += "<p class='error'>Making Folder fails.</p>";
        status += "<p class='error'>Note : A-Z, a-z, 0-9, _, - can be only used for the folder name. "
                "If the folder already exists, then new folder can not be made.</p>";
    }

    string radioButtons = "";
    vector<filesystem::directory_entry> directories{ filesystem::directory_entry(filesystem::path(rootDir)) };
    for (auto& de : filesystem::recursive_directory_iterator(rootDir))
        if (filesystem::is_directory(de)) directories.push_back(de);

    string dirPath = directories[0].path().generic_string().substr(rootDir.size()) + "/";
    radioButtons += "<p><input type=radio name='dir' value='" + dirPath + "' checked>in " + dirPath + "</p>";
    for (int i = 1; i < directories.size(); ++i) {
        string dirPath = directories[i].path().generic_string().substr(rootDir.size()) + "/";
        radioButtons += "<p><input type=radio name='dir' value='" + dirPath + "'>in " + dirPath + "</p>";

    }

    string dirAndFiles = "";
    for (auto& dir : directories) {
        string dirPath = dir.path().generic_string().substr(rootDir.size()) + "/";
        dirAndFiles += "<article><h2>" + dirPath + "</h2>";
        for (auto& de : filesystem::directory_iterator(dir)) {
            if (filesystem::is_regular_file(de)) {
                string path = de.path().filename().string();
                dirAndFiles += "<p><a href='." + dirPath + path + "'>" + path + "</a></p>";
            }
        }
        dirAndFiles += "</article>";
    }

    string main = { u8""
        "<aside>"
            "<form action='./files'>"
                "<h4>Make Folder</h4>"
                "<input type='text' name='make_folder' placeholder='Folder Name ( A-Z, a-z, 0-9, -, _ )'"
                "pattern='[A-Za-z0-9_-]+' title='Only A-Z, a-z, 0-9, -, _ can be used.'>"
                "<input type='submit' value='Make Folder'/>"
                    + status +
            "</form>"

            "<form action='./files' method='post' enctype='multipart/form-data'>"
                "<h4>Upload File</h4>"
                "<p>Cheese a file below</p>"
                    + radioButtons +
                "<input type='file' name='file' value='Choose File'/>"
                "<input type='submit' value='Upload File'/>"
            "</form>"
        "</aside>"
        "<section>"
                    + dirAndFiles +
        "</section>"
    };
    return getHtmlPage("Files", "", main);
}


string DynamicHtml::getRequestLogPage(const string& queryString, DatabaseHandler* databaseHandler) {
    string headExtra = {
            "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>"
            "<script src='./log.js'></script>"
    };

    string checkboxes = {
        "<p><input id='allCheckbox' type='checkbox'> All</p>"
        "<p><input id='requestLineCheckbox' type='checkbox'> Request Line</p>"
    };

    vector<pair<string, string>> queries = parseQuery(queryString);

    vector<vector<string>> res;
    for (const string& field : {"method", "request_uri", "http_version"}) {
        auto it = find_if(queries.begin(), queries.end(), [&field](const pair<string, string>& query){return query.first == field;});
        if (it != queries.end()) {
            checkboxes += "<p><input class='requestLine' type='checkbox' name='" + field + "' checked>"  + field + "</p>";
        } else {
            checkboxes += "<p><input class='requestLine' type='checkbox' name='" + field + "'>"  + field + "</p>";
        }
    }

    checkboxes += "<p><input id='headersCheckbox' type='checkbox'> Headers</p>";
    for (const string& field : {"accept", "accept_encoding", "accept_language", "connection", "host", "user_agent", "access_time"}) {
        auto it = find_if(queries.begin(), queries.end(), [&field](const pair<string, string>& query){return query.first == field;});
        if (it != queries.end()) {
            checkboxes += "<p><input class='headers' type='checkbox' name='" + field + "' checked>" + field + "</p>";
        } else {
            checkboxes += "<p><input class='headers' type='checkbox' name='" + field + "'>" + field + "</p>";
        }
    }
    int numRows = databaseHandler->getQueryResults(queries, &res);

    string table = "";
    if (numRows == -1) {
        table += "<h2>Search Result - Error Occurred</h2><table></table>";
    } else if (numRows == 0) {
        table += "<h2>Search Result - " + to_string(numRows) + " rows found</h2><table></table>";
    } else {
        table += "<h2>Search Result - " + to_string(numRows) + " rows found</h2>";
        table += "<table><tr>";
        for (string& header : res[0])
            table += "<th>" + header + "</th>";
        table += "</tr>";

        for (int i = 1; i < res.size(); ++i) {
            table += "<tr>";
            for (string& rowData : res[i]) {
                table += "<td>" + rowData + "</td>";
            }
            table += "</tr>";
        }
        table += "</table>";
        table += "<p>";

        int idx = queryString.find("&row=");
        int i = 0;
        for (; i < numRows - 100; i += 100) {
            table += "<a href='./log?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
                       + to_string(i + 1) + "-" + to_string(i + 100) + "</a>";
        }
        table += "<a href='./log?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
                   + to_string(i + 1) + "-" + to_string(numRows) + "</a>";
        table += "</p>";
    }

    string main = {
            "<aside>"
                "<form action='./log' method='get'>"
                    "<h4>Search</h4>"
                    "<input type='text' name='search' placeholder='Search Word'>"
                        + checkboxes +
                    "<input type='reset' value='Reset'/>"
                    "<input type='submit' value='Search'/>"
                "</form>"
            "</aside>"

            "<section>"
                "<article>"
                        + table +
                "</article>"
            "</section>"
    };

    return getHtmlPage("Request Log", headExtra, main);
}
