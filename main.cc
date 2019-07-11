// To compile, in term run:
// clang++ -Wall -Werror -Wshadow -pthread http.cc server.cc main.cc -o
// server
// it angry

#include "http.h"
#include "server.h"
#include <iostream>
#include <memory>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include </home/calvin/Desktop/Code/Web/FileSortTools.cpp>
#include <algorithm>
#include <dirent.h>
#include <experimental/filesystem>
#include <fstream>
#include <vector>

namespace fs = std::experimental::filesystem;
#define BUF_SIZE 1024 * 1024
using namespace obsequi;
using namespace std;

// const string homeFilePath = "/var/www/html";
const string homeFilePath = "/home/calvin/Desktop/Code";
vector<string> getFileDirectory(string path);
string creatRow(string path, string unK);

/* -------------------------------- ------- -------------------------------- **
-- -------------------------------- Methods -------------------------------- --
-- ------------- in a coment EX = Exspanation or do not remove ------------- --
** -------------------------------- ------- -------------------------------- */

// testing changes

// TODO deal with dSE
// TODO find a better func name
// EX puts every file in a vetor with the date it was added
vector<FileSort> everyFileVec(string path) {
  vector<FileSort> everyFile;
  path = homeFilePath + path;
  DIR *dir;
  struct dirent *ent;
  // EX checks if directery is open
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("");
    return everyFile;
  }

  /* EX adds all the files and directories within directory to everyFile*/
  int i = 0;
  while ((ent = readdir(dir)) != NULL) {
    string unK = string(ent->d_name); // what is this
    fs::path p = path + unK;
    auto dSE = fs::last_write_time(p).time_since_epoch();
    printf("%s\n", ent->d_name);

    if (fs::is_directory(path + unK)) {
      everyFileVec(path + unK);
    } else {
      // TODO remove creatRow and just add block of code
      everyFile.push_back(
          FileSort(dSE.count(), "<tr>" + creatRow(path, unK) + "</tr>"));
    }
    i++;
  }
  closedir(dir);
  return everyFile;
}

// TODO sort bases on date
// EX returns the html table for oldest and newest files
string everyFileSort(string path, int numOFRows) {
  vector<FileSort> everyFile = everyFileVec(path);
  string html;

  sort(everyFile.begin(), everyFile.end(),
       [](const FileSort &a, const FileSort &b) {
         return a.getDate() < b.getDate();
       });
  // concatinats the chosen into a string
  for (int i = 0; i < everyFile.size(); i++) {
    html = "<table>" + everyFile[i].getRow() + "</table>";
  }
  return html;
}

// EX sorts the vector indexes that hold the rows & formats for HTML table
string webContentSort(string path) {
  vector<string> webContent = getFileDirectory(path);
  string webContentF;
  // int parentDirecteryI;
  /*
  cout << endl;
  cout << endl;
  cout << endl;
  cout << endl;
  */
  sort(webContent.begin(), webContent.end());
  for (int i = 0; i < webContent.size(); i++) {
    // TODO should this if move to vec creation to eliminate to itterations?
    if (webContent[i].find("./") != std::string::npos ||
        webContent[i].find("..") != std::string::npos) {
      webContent[i].erase(i);
      // cout << ". = " << i << ", " << webContent[i] << endl;
    } else {
      // cout << "path = " << i << ", " << webContent[i] << endl;
      webContentF = webContentF + webContent[i];
    }
  }
  // EX this row is the parentDirectory Row than added to the rest
  webContentF = R"stop(
        <tr>
          <th> </th>
          <th><a href="../"> &lt;&lt;PD </a></th>
          <th> </th>
          <th> </th>
          <th> </th>
        </tr>
      )stop" + webContentF;
  return webContentF;
}

// EX creats the html page
string htmlFormat(string tableRows) {
  return R"RAW(<!DOCTYPE html> 
  <html>
    <head>
      <title>Movie index</title>
      <meta name="viewport" content="width=device-width,inital-scale=1">
      <style>
      body {
        background-color: #272727;
        color: #ffffff;
      }

      a {
        color: #cccc00;
      }

      a:visited {
        color: #009a00;
      }

      a:hover {
        color: #ffff4d;
      }
      #one {
        color: #ff0000;
      }
    </style>
    </head>

    <body>
      <div>
      </div>
      <p>
        <h2>Movie Index</h2>
        The "stop" will turn off the site.
        <a id="one" href="/stop">STOP</a><br>
        <table style="width:30%">
          <tr>
            <th>Icon</th>
            <th>Name</th>
            <th>Size</th>
            <th>Date </th>
            <th>Date </th>
          </tr>
          <tr>
            <th colspan="5">
              <hr>
            </th>
          </tr>)RAW" +
         tableRows +
         R"RAW(<tr>
            <th colspan="5">
              <hr>
            </th>
          </tr>
        </table>
        v.2.1
      </p>
    </body>

  </html>)RAW";
}

// TODO needs the last time a file was accsesed
// TODO strftime insted of asctime
// EX returns the elements of a table row
string creatRow(string path, string unK) {
  // EX this method is called only from getFiledirectery
  string row[5];
  fs::path p = path + unK;
  auto ftime = fs::last_write_time(p);
  std::time_t cftime =
      decltype(ftime)::clock::to_time_t(ftime); // assuming system_clock
  string timeA = asctime(std::localtime(&cftime));
  row[1] = "<th><a href = \"" + unK + "/\">" + unK + "</a></th>";

  if (fs::is_directory(path + unK)) {
    row[0] =
        R"(<th><img src="/Web/assets/folder_icon.jpg/" alt="[DNE]" width="20"></th>)";
    row[2] = "<th>null</th>";
    row[3] = "<th>null</th>";
    row[4] = "<th>null</th>";
  } else {
    row[0] =
        R"(<th><img src="/Web/assets/file_icon.png/" alt="[DNE]" width="20"></th>)";
    row[2] = "<th>" + to_string(fs::file_size(p)) + "</th>";
    row[3] = "<th>" + timeA + "</th>";
    row[4] = "<th>time2</th>";
  }
  return row[0] + row[1] + row[2] + row[3] + row[4];
}

// TODO clean up ----------------------------------------------------**
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

// EX ?
void serveFile(const HttpRequest &req, HttpResponse *resp, const string &type) {
  string path = homeFilePath + req.request_uri_;
  path = path.substr(0, path.size() - 1);

  cout << "serveFile path = " << path << endl;

  int fd = open(path.c_str(), 0);
  if (fd == -1) {
    cout << "Unable to open file: " << path << endl;
  }

  int file_size = lseek(fd, 0, SEEK_END);
  int err = lseek(fd, 0, SEEK_SET);

  // kj char buf[100];
  // int bufx = read(fd, buf, 10);
  // buf[10] = '\0';
  //
  cout << file_size << ": " << err << ": " << endl; // << bufx << buf << endl;

  resp->SetContent(type, fd, file_size);
}

// EX this passes the file type to serveIndexHtml()
int checkFileType(string path) {
  string img = ".jpg";
  string vid = ".m4v";
  string cpp = ".cpp";
  string txt = ".txt";

  if (path.find(img) != std::string::npos) {
    return 0;
  } else if (path.find(vid) != std::string::npos) {
    return 1;
  } else if ((path.find(cpp) != std::string::npos) ||
             (path.find(txt) != std::string::npos)) {
    return 2;
  }
  return 2;
}

// EX decides witch html header to use
void serveIndexHtml(const HttpRequest &req, HttpResponse *resp) {
  req.Print();
  string path = homeFilePath + req.request_uri_;
  if (fs::is_directory(path)) {
    resp->SetHtmlContent(htmlFormat(webContentSort(req.request_uri_)));
  } else if (checkFileType(path) == 2) {
    serveFile(req, resp, "text/html");
    // resp->SetContent(getFile(req.request_uri_), "text/html");
  } else if (checkFileType(path) == 1) {
    serveFile(req, resp, "video/mp4");
    // resp->SetContent(getFile(req.request_uri_), "video/mp4");
  } else if (checkFileType(path) == 0) {
    resp->SetContent(getFile(req.request_uri_), "image/jpg");
  }
}

// EX The function we want to make the thread run.
void handle(const TcpConnection &conn) {
  HttpRequest req = HttpRequest::parse(conn.fd);
  HttpResponse resp;

  cout << "## " << req.method_ << " " << req.request_uri_ << endl;
  // req.Print();
  // cout << req.content_ << endl;
  if (req.request_uri_ == "/stop") {
    exit(1);
  }
  if (req.request_uri_ == "/recent") {
    resp.SetHtmlContent(htmlFormat("List of Recent files"));
  }
  serveIndexHtml(req, &resp);

  resp.Send(conn.fd);
}

// -- MAIN --
int main() {
  TcpServer server(handle);
  server.Run(8000, 4);
  // cout << webContentSort("/home/calvin") << endl;
}
