//
// Created by koji on 2/16/17.
//
#include <boost/filesystem.hpp>
#include "DynamicHtml.hpp"
#include "HttpHandler.hpp"
#include "DatabaseHandler.hpp"

using namespace boost;

const string DynamicHtml::FILE_DIR = "/files";

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

    if (pageName.find(FILE_DIR) == 0) {
        if (pageName.size() > FILE_DIR.size()) {
            if (handler->getMethod() == "POST") return getFileTable(handler->getRootDir(), pageName);
            return getFilesPage(handler->getRootDir(), pageName);
        }

        return getDirsPage(queryString, handler->getRootDir());
    } else if (pageName == "/log"){
        return getRequestLogPage(queryString, handler->getDatabaseHandler());
    } else if (pageName == "/logTable") {
        return getRequestLogTable(queryString, handler->getDatabaseHandler());

    }
    return "";
}

string DynamicHtml::getHtmlPage(const string& pageTitle, const string& headExtra, const string& main) {
    string page = { u8""
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<meta charset='UTF-8'>"
            "<title>" + pageTitle + "</title>"
            "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>"
            + headExtra +
            "<link id='css' href='/css/layout.css' rel='stylesheet' />"
        "</head>"
        "<body>"
        "<header>"
            "<nav>"
                "<h1><a href='/home.html'><img src='/img/logo.png' height='65px' alt='kokayasu logo'></a></h1>"
                "<a href='/home.html' class='hNav'>Home</a>"
                "<a href='/projects.html' class='hNav'>Personal Projects</a>"
    };

    if (pageTitle == "Files") {
        page += {
                "<a href='/files' class='hNav' id='here'>Files</a>"
                "<a href='/log' class='hNav'>Request Log</a>"
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
        return filesystem::create_directories(rootDir + FILE_DIR + "/" + dirName);

    return false;
}

string DynamicHtml::getFilesPage(const string& rootDir, const string& dirName) {
    string path = rootDir + dirName;
    string headExtra = {
            "<script src='/js/files.js'></script>"
    };

    string form = "";
    if (filesystem::exists(path)) {
        if (filesystem::is_regular_file(path)) return "";

        form += {
            "<div class='box'>"
                "<h4>Choose Files</h4>"
                "<input type='file' id='file' multiple>"
                "<p class='error'>Please choose less than or equal to 10 files.</p>"
            "</div>"

            "<div id='progressBox'class='box' hidden>"
            "    <input type='button' value='Upload File'>"
            "</div>"
        };
    }

    string explanation = {
            "<article>"
                "<h2>Why I am making this page</h2>"
                "<p>application/x-www-form-urlencoded and multipart/form-data has different structures, so I decided to"
                "   make this page and make this server parse the POST request.</p>"
                "<p>This page also uses AJAX to upload the files and load the result of the page.</p>"
            "</article>"
    };

    string main = { u8""
        "<aside>" + form + "</aside>"
        "<section>"
                                +  explanation +
            "<article>" + getFileTable(rootDir, dirName) + "</article>"
        "</section>"
    };
    return getHtmlPage("Files", headExtra, main);
}

string DynamicHtml::getFileTable(const string& rootDir, const string& dirName) {
    string path = rootDir + dirName;

    string fileTable = "<table>";
    if (filesystem::exists(path)) {
        if (filesystem::is_regular_file(path)) return "";

        fileTable += "<tr><th>File Name</th><th>File Size</th></tr></th>";
        for (const auto& de : filesystem::directory_iterator(path)) {
            if (filesystem::is_regular_file(de)) {
                string fileName = de.path().filename().string();
                fileTable += "<tr><td><p><a href='" + dirName + "/" + fileName + "'>" + fileName + "</a></p></td>";
                fileTable += "<td>" + to_string(filesystem::file_size(de)) + " B </td></tr>";
            }
        }
    } else {
        fileTable += "<h2>" + dirName.substr(FILE_DIR.size()) + " does not exist.</h2>";
    }

    fileTable += "</table>";
    return fileTable;
}

string DynamicHtml::getDirsPage(const string& queryString, const string& rootDir) {
    string status = "";

    string dirs = "<article><h2>Folders</h2><table>";
    dirs += "<tr><th>Folder Name</th><th>Number of Files</th></tr></th>";
    for (auto& de : filesystem::directory_iterator(rootDir + FILE_DIR)) {
        if (filesystem::is_directory(de)) {
            int numFiles = 0;
            string path = de.path().filename().string();
            for (auto& f : filesystem::directory_iterator(de)) {
                if (filesystem::is_regular_file(f)) numFiles++;
            }
            dirs += "<tr><td><p><a href='/files/" + path + "'>" + path + "</a></p></td>";
            dirs += "<td>" + to_string(numFiles) + " </td></tr>";
        }
    }
    dirs += "</table></article>";

    string explanation = {
            "<article>"
                    "<h2>Why I am making this page</h2>"
                    "<p>I wanted to know how POST method of HTTP works and decided to write code for it."
                    "   For making folder, POST method is used and parse the POST request by this server program."
                    "   By clicking a folder name, go to inside of the folder and then files can be upload (Upload File uses POST).</p>"
                    "</article>"
    };

    string main = { u8""
        "<aside>"
            "<form class='box' action='/files' method='post'>"
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
            "<script src='/js/log.js'></script>"
    };

    string checkboxes = {
        "<label><input id='allCheckbox' type='checkbox'> All</label>"
            "<label><input id='requestLineCheckbox' type='checkbox'> Request Line</label>"
                "<label><input class='requestLine' type='checkbox' name='method'> method</label>"
                "<label><input class='requestLine' type='checkbox' name='request_uri'> request_uri</label>"
                "<label><input class='requestLine' type='checkbox' name='http_version'> http_version</label>"
            "<label><input id='headersCheckbox' type='checkbox'> Headers</label>"
                "<label><input class='headers' type='checkbox' name='accept'> accept</label>"
                "<label><input class='headers' type='checkbox' name='accept_encoding'> accept_encoding</label>"
                "<label><input class='headers' type='checkbox' name='accept_language'> accept_language</label>"
                "<label><input class='headers' type='checkbox' name='connection'> connection</label>"
                "<label><input class='headers' type='checkbox' name='host'> host</label>"
                "<label><input class='headers' type='checkbox' name='user_agent'> user_agent</label>"
                "<label><input class='headers' type='checkbox' name='access_time'> access_time</label>"
    };

    string explanation = {
            "<article>"
                "<h2>Why I am making this page</h2>"
                "<p>I wanted to make a program that communicates with SQL. This website uses MySql for the database.</p>"
                "<p>I use jQuery to manipulate the check boxes and AJAX to load the table without loading a whole page.</p>"
            "</article>"
    };

    string main = {
            "<aside>"
                    "<form class='box'>"
                        "<h4>Search</h4>"
                        "<input type='text' name='search' placeholder='Search Word'/>"
                            + checkboxes +
                        "<input type='reset' value='Reset'/>"
                        "<input id='searchButton' type='button' value='Search'/>"
                    "</form>"
                "</form>"
            "</aside>"

            "<section>"
                        + explanation +
                "<article id='tableArticle'>"
                        + getRequestLogTable(queryString, databaseHandler) +
                "</article>"
            "</section>"
    };

    return getHtmlPage("Request Log", headExtra, main);
}

string DynamicHtml::getRequestLogTable(const string& queryString, DatabaseHandler* databaseHandler) {
    vector<vector<string>> res;
    int numRows = databaseHandler->getQueryResults(parseQuery(queryString), &res);

    if (numRows == -1) return "<h2>Search Result - Error Occurred</h2><table></table>";
    if (numRows == 0)  return "<h2>Search Result - " + to_string(numRows) + " rows found</h2><table></table>";

    string table = "";
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

    int idx = queryString.find("&row=");
    int i = 0;
    for (; i < numRows - 100; i += 100) {
        table += "<a href='/logTable?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
                 + to_string(i + 1) + "-" + to_string(i + 100) + "</a>";
    }
    table += "<a href='/logTable?" + queryString.substr(0, idx) + "&row=" + to_string(i) + "'>"
             + to_string(i + 1) + "-" + to_string(numRows) + "</a>";

    return table;
}
