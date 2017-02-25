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
        return getDirsPage(queryString, handler->getRootDir());
    } else if (pageName == "/log"){
        return getRequestLogPage(queryString, handler->getDatabaseHandler());
    }
    return "";
}

string DynamicHtml::getHtmlPage(const string& pageTitle, const string& headExtra, const string& main) {
    string page = { u8""
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
                "<h1><a href='./home.html'><img src='./logo.png' height='65px' alt='kokayasu logo'></a></h1>"
                "<a href='./home.html' class='hNav'>Home</a>"
                "<a href='./projects.html' class='hNav'>Personal Projects</a>"
    };

    if (pageTitle == "Files") {
        page += {
                "<a href='./files' class='hNav' id='here'>Files</a>"
                "<a href='./log' class='hNav'>Request Log</a>"
        };

    } else if (pageTitle == "Request Log") {
        page += {
                "<a href='./files' class='hNav'>Files</a>"
                "<a href='./log' class='hNav'id='here'>Request Log</a>"
        };
    }

    page += {
            "</nav>"
        "</header>"
        "<main>" + main + "</main>"
        "</body>"
        "</html>"
    };

    return page;
}

bool DynamicHtml::isValidFileName(const string &name){
    for (const char c : name) {
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
    if (isValidFileName(dirName))
        return filesystem::create_directories(rootDir + "/" + dirName);

    return false;
}

string DynamicHtml::getFilesPage(const string& rootDir, const string& dirName) {
    string path = rootDir + "/" + dirName;
    string form = "", files = "";
    string status = "";

    //status += "<p class='error'>Making Folder fails.</p>";
    //status += "<p class='error'>Note : A-Z, a-z, 0-9, _, - can be only used for the folder name. "
    //        "If the folder already exists, then new folder can not be made.</p>";

    if (filesystem::exists(path)) {
        form += {
            "<form action='./files?folder=" + dirName + "' method='post' enctype='multipart/form-data'>"
                "<h4>Upload File</h4>"
                "<input type='hidden' name='dir' value='" + dirName +"'>"
                "<input type='file' name='file' value='Choose File'/>"
                "<input type='submit' value='Upload File'/>"
                    + status +
            "</form>"
        };
        files += "<article><h2>" + dirName + "</h2>";
        for (auto& de : filesystem::directory_iterator(path)) {
            if (filesystem::is_regular_file(de)) {
                string path = de.path().filename().string();
                files += "<p><a href='./" + dirName + "/" + path + "'>" + path + "</a></p>";
            }
        }
        files += "</article>";
    } else {
        form += "<form></form>";
        files += "<article><h2>" + dirName + " does not exist.</h2></article>";
    }

    string explanation = {
            "<article>"
                    "<h2>Motivation</h2>"
                    "<p>I wanted to know how POST method of HTTP works and decided to write code for it."
                    "   Upload File uses POST.</p>"
                    "</article>"
    };

    string main = { u8""
        "<aside>" + form + "</aside>"
        "<section>" +  explanation + files + "</section>"
    };
    return getHtmlPage("Files", "", main);
}

string DynamicHtml::getDirsPage(const string& queryString, const string& rootDir) {
    string status = "";
    for (const pair<string, string>& query : parseQuery(queryString)) {
        if (query.first == "make_folder") {
            if (!createDirectory(rootDir, query.second)) {
                status += "<p class='error'>Making Folder fails.</p>";
                status += "<p class='error'>Note : A-Z, a-z, 0-9, _, - can be only used for the folder name. "
                        "If the folder already exists, then new folder can not be made.</p>";
            }
            break;
        }

        if (query.first == "folder") {
            return getFilesPage(rootDir, query.second);
        }
    }

    string dirs = "<article><h2>Folders</h2>";
    for (auto& de : filesystem::directory_iterator(rootDir)) {
        if (filesystem::is_directory(de)) {
            string path = de.path().filename().string();
            dirs += "<p><a href='./files?folder=" + path + "'>" + path + "</a></p>";
        }
    }
    dirs += "</article>";

    string explanation = {
            "<article>"
                    "<h2>Motivation</h2>"
                    "<p>I wanted to know how POST method of HTTP works and decided to write code for it (Make Folder uses GET)."
                    "   By clicking a folder name, go to inside of the folder and then files can be upload (Upload File uses POST).</p>"
                    "</article>"
    };

    string main = { u8""
        "<aside>"
            "<form action='./files'>"
                "<h4>Make Folder</h4>"
                "<input type='text' name='make_folder' placeholder='Folder Name ( A-Z, a-z, 0-9, -, _ )'"
                "pattern='[A-Za-z0-9_-]+' title='Only A-Z, a-z, 0-9, -, _ can be used.'>"
                "<input type='submit' value='Make Folder'/>"
                    + status +
            "</form>"
        "</aside>"
        "<section>" + explanation + dirs + "</section>"
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

    string explanation = {
            "<article>"
                    "<h2>Motivation</h2>"
                    "<p>I wanted to make a program that communicates with SQL. This website uses MySql for the database.</p>"
                    "</article>"
    };

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
                        + explanation +
                "<article>"
                        + table +
                "</article>"
            "</section>"
    };

    return getHtmlPage("Request Log", headExtra, main);
}
