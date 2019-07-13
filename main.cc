#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "FileSortTools.cpp"
#include "html.h"
#include "http.h"
#include "server.h"

namespace fs = std::experimental::filesystem;
#define BUF_SIZE 1024 * 1024
using namespace obsequi;
using namespace std;

#if 0 // EX Running as production
const string homeFilePath = "/var/www/html";
const int port = 8080;
#else // EX Running as development
const string homeFilePath = "/home/calvin/Desktop/Code/WebServer";
const int port = 8000;
#endif

vector<string> getFileDirectory(string path);
string creatRow(string path, string unK);

/* -------------------------------- ------- -------------------------------- **
-- -------------------------------- Methods -------------------------------- --
-- ------------- in a coment EX = Explanation or do not remove ------------- --
** -------------------------------- ------- -------------------------------- */

// TODO find a better func name
// EX puts every file in a vetor with the date it was added
void everyFileVec(string path, vector<FileSort> *everyFile) {
  path += "/";
  DIR *dir;
  struct dirent *ent;
  // EX checks if directery is open
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("");
    return;
  }

  /* EX adds all the files and directories within directory to everyFile*/
  while ((ent = readdir(dir)) != NULL) {
    string unK = string(ent->d_name); // what is this again?
    fs::path p = path + unK;
    auto dSE = fs::last_write_time(p).time_since_epoch();
    printf("%s %ld\n", ent->d_name, dSE.count());

    if (fs::is_directory(path + unK)) {
      // EX "if" checks for the "." ".." so it dose not infa loop
      if ((unK != ".") && (unK != "..")) {
        everyFileVec(path + unK, everyFile);
      }
    } else {
      everyFile->push_back(
          FileSort(dSE, "<tr>" + creatRow(path, unK) + "</tr>"));
    }
  }
  closedir(dir);
}

// EX returns the html table for oldest and newest files
string everyFileSort(string path, int numOfRows) {
  vector<FileSort> everyFile;
  everyFileVec(path, &everyFile);
  string html;

  sort(everyFile.begin(), everyFile.end(),
       [](const FileSort &a, const FileSort &b) {
         return a.getDate() > b.getDate();
       });
  html = "<table>";
  for (size_t i = 0; i < numOfRows; i++) {
    html += everyFile[i].getRow();
  }

  return html + "</table>";
}

// EX sorts the vector indexes that hold the rows & formats for HTML table
string webContentSort(string path) {
  vector<string> webContent = getFileDirectory(path);
  string webContentF;

  sort(webContent.begin(), webContent.end());
  for (size_t i = 0; i < webContent.size(); i++) {
    // TODO should this if move to vec creation to eliminate to itterations?
    if (webContent[i].find("./") != std::string::npos ||
        webContent[i].find("..") != std::string::npos) {
      webContent[i].erase(i);
    } else {
      webContentF = webContentF + webContent[i];
    }
  }
  // EX this row is the parentDirectory Row than added to the rest
  webContentF = R"stop(
        <tr>
          <th><img src="/assets/backArrow_icon_edit.png" alt="[DNE]" width="10"></th>
          <th><a href="../"> &lt;&lt;PD </a></th>
          <th> -- </th>
          <th> -- </th>
        </tr>
      )stop" + webContentF;
  return webContentF;
}

// EX creats the html page
string htmlFormat(string tableRows) {
  return HTML_HEAD + tableRows + HTML_TAIL;
}

// converts bytes to needed unit
string byteConversion(unsigned long size) {
  int unit = 0;
  string units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

  while (size >= 1024) {
    size /= 1024;
    ++unit;
  }
  // cout << "befor:" << size << endl;
  return to_string((int)size) + " " + units[unit];
}

// TODO unK = file name
// TODO needs the last time a file was accsesed
// EX returns the elements of a table row
string creatRow(string path, string unK) {
  // EX this method is called only from getFiledirectery
  string row[5];
  fs::path p = path + unK;
  auto ftime = fs::last_write_time(p);
  // assuming system_clock
  std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
  char timeB[80];
  strftime(timeB, 80, "%b/%d/%y", std::localtime(&cftime));

  if (fs::is_directory(path + unK)) {
    row[0] =
        R"(<th><img src="/assets/folder_icon_edit.png" alt="[DNE]" width="20"></th>)";
    row[1] = "<th><a href = \"" + unK + "/\">" + unK + "</a></th>";
    row[2] = "<th>0 B</th>";
    row[3] = "<th>--</th>";
  } else {
    if ((path + unK).find(".m4v") != std::string::npos) {
      row[0] =
          R"(<th><img src="/assets/video_icon.png" alt="[DNE]" width="20"></th>)";
    } else {
      row[0] =
          R"(<th><img src="/assets/file_icon_edit.png" alt="[DNE]" width="20"></th>)";
    }
    row[1] = "<th><a href = \"" + unK + "\">" + unK + "</a></th>";
    row[2] = "<th>" + byteConversion(fs::file_size(p)) + "</th>";
    row[3] = "<th>" + string(timeB) + "</th>";
  }
  return row[0] + row[1] + row[2] + row[3];
}

// EX creats and fills the vector that holds the table rows html
vector<string> getFileDirectory(string path) {
  vector<string> webContent;
  path = homeFilePath + path;
  DIR *dir;
  struct dirent *ent;

  // EX checks if directery is open
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("");
    return webContent;
  }

  /* EX print all the files and directories within directory */
  int i = 0;
  while ((ent = readdir(dir)) != NULL) {
    string unK = string(ent->d_name); // what is this
    printf("%s\n", ent->d_name);
    webContent.push_back("<tr>" + creatRow(path, unK) + "</tr>");
    i++;
  }
  closedir(dir);
  return webContent;
}

// EX looks at the files
string getFile(string path) {
  path = homeFilePath + path;
  path = path.substr(0, path.size() - 1);
  string data;

  cout << "getFile path = " << path << endl;

  ifstream myfile(path);
  if (myfile.is_open()) {
    char c;
    while (myfile.get(c)) {
      data += (char)c;
    }
    myfile.close();
  } else {
    cout << "Unable to open file: " << path << endl;
  }
  cout << data.size() << endl;
  return data;
}

// EX serves the files
void serveFile(const HttpRequest &req, HttpResponse *resp, const string &type) {
  string path = homeFilePath + req.request_uri_;

  int fd = open(path.c_str(), 0);
  if (fd == -1) {
    cout << "Unable to open file: " << path << endl;
    return;
  }

  ssize_t file_size = lseek(fd, 0, SEEK_END);
  ssize_t err = lseek(fd, 0, SEEK_SET);

  // EX debug for lseek
  cout << file_size << ": " << err << ": " << endl;

  resp->SetContent(type, fd, (size_t)file_size);
}

// EX this passes the file type to serveIndexHtml()
string getMimeType(string path) {
  string img = ".jpg";
  string vid = ".m4v";
  string html = ".html";

  if (path.find(img) != std::string::npos) {
    return "image/jpg";
  } else if (path.find(vid) != std::string::npos) {
    return "video/mp4";
  } else if (path.find(html) != std::string::npos) {
    return "text/html";
  }
  return "text/plain";
}

// EX The function we want to make the thread run.
void handle(const TcpConnection &conn) {
  HttpRequest req = HttpRequest::parse(conn.fd);
  HttpResponse resp;

  cout << "## " << req.method_ << " " << req.request_uri_ << endl;
  if (req.request_uri_ == "/stop") {
    exit(1);
  }
  if (req.request_uri_ == "/recent") {
    resp.SetHtmlContent(htmlFormat(everyFileSort(homeFilePath, 50)));
  } else {
    string path = homeFilePath + req.request_uri_;
    if (fs::is_directory(path)) {
      resp.SetHtmlContent(htmlFormat(webContentSort(req.request_uri_)));
    } else {
      serveFile(req, &resp, getMimeType(path));
    }
  }

  resp.Send(conn.fd);
}

int main() {
  TcpServer server(handle);
  server.Run(port, 4);
  // cout << everyFileSort("/home/calvin/Desktop/Code", 5) << endl; // debug
}
