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
#include <locale>
#include <memory>
#include <regex>
#include <vector>

#include "html.h"
#include "http.h"
#include "log.h"
#include "server.h"

namespace fs = std::experimental::filesystem;
using namespace calvin;
using namespace std;

const string homeFilePath = ".";

struct FileSort {
  string path;
  string fileName;
  std::chrono::nanoseconds date_created;

  string getLink() { return path + fileName; }
};

/* -------------------------------- ------- -------------------------------- **
-- -------------------------------- Methods -------------------------------- --
-- ------------- in a coment EX = Explanation or do not remove ------------- --
** -------------------------------- ------- -------------------------------- */

string FilePath(const string &path) { return homeFilePath + path; }

// EX creats the html page
string htmlFormat(string tableRows) {
  return HTML_HEAD + tableRows + HTML_TAIL;
}

// EX converts a string to lower case
string toLower(string str) {
  for (size_t i = 0; i < str.length(); i++) {
    str[i] = (char)tolower(str[i]);
  }
  return str;
}

// EX converts bytes to needed unit
string formatFileSize(unsigned long size) {
  int unit = 0;
  string units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

  while (size >= 1024) {
    size /= 1024;
    ++unit;
  }
  return to_string((int)size) + " " + units[unit];
}

// TODO needs the last time a file was accsesed
// EX returns the elements of a table row
string creatRow(FileSort file) {
  string row[5];
  string filepath = FilePath(file.getLink());
  fs::path p = filepath;
  auto ftime = fs::last_write_time(p);
  // EX assuming system_clock
  std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
  char timeB[80];
  strftime(timeB, 80, "%b/%d/%y", std::localtime(&cftime));

  string icon;
  if ((filepath).find(".m4v") != std::string::npos ||
      (filepath).find(".mp4") != std::string::npos) {
    icon = "<img src= \"/assets/video_icon.png\" ";
  } else if (filepath.find(".mp3") != std::string::npos) {
    icon = "<img src=\"/assets/audio_icon_edit.png\" ";
  } else {
    icon = "<img src=\"/assets/file_icon_edit.png\" ";
  }

  if (fs::is_directory(filepath)) {
    row[0] =
        R"(<td class="icon"><img src="/assets/folder_icon_edit.png" alt="[DNE]" width="20"></td>)";
    row[1] = "<td class=\"filename\" ><a href = \"" + file.getLink() + "/\">" +
             file.fileName + "</a></td>";
    row[2] = "<td class=\"filesize\" >0 B</td>"; // TODO get dur size func here
    row[3] = "<td class=\"lastmodified\">--</td>";
  } else {
    row[0] =
        R"(<td class="icon">)" + icon + R"(alt="[DNE]" width="20"></td>)";
    row[1] = "<td class=\"filename\" ><a href = \"" + file.getLink() + "\">" +
             file.fileName + "</a></td>";
    row[2] =
        "<td class=\"filesize\" >" + formatFileSize(fs::file_size(p)) + "</td>";
    row[3] = "<td class=\"lastmodified\">" + string(timeB) + "</td>";
  }
  return "<tr>" + row[0] + row[1] + row[2] + row[3] + "</tr>\n";
}

// EX creats and fills the vector that holds the table rows html
vector<FileSort> getFileDirectory(string path) {
  vector<FileSort> webContent;
  string filepath = FilePath(path);
  DIR *dir;
  struct dirent *ent;

  // EX checks if directery is open
  if ((dir = opendir(filepath.c_str())) == NULL) {
    perror("");
    return webContent;
  }

  /* EX print all the files and directories within directory */
  int i = 0;
  while ((ent = readdir(dir)) != NULL) {
    string fileName = string(ent->d_name); // what is this
    // LOG_DEBUG("directory entry: %s", ent->d_name);
    webContent.push_back(FileSort{path, fileName});
    i++;
  }
  closedir(dir);
  return webContent;
}

/* EX recurses trough each directery and adds every file to a vec send target a
 * blank string to just get every file and its size*/
void recursiveIndex(string searchTarget, string path,
                    vector<FileSort> *fileIndex) {
  path += "/";
  DIR *dir;
  // EX regex for search
  std::regex e(toLower(searchTarget));
  struct dirent *ent;
  // EX checks if directery is open
  if ((dir = opendir(FilePath(path).c_str())) == NULL) {
    perror("");
    return;
  }
  while ((ent = readdir(dir)) != NULL) {
    string fileName = string(ent->d_name);
    fs::path p = FilePath(path + fileName);
    // EX dse is for recent files
    auto dSE = fs::last_write_time(p).time_since_epoch();

    if (fs::is_directory(FilePath(path + fileName))) {
      // EX "if" checks for the "." ".." so it dose not infa loop
      if ((fileName != ".") && (fileName != "..")) {
        recursiveIndex(searchTarget, path + fileName, fileIndex);
      }
    } else if (regex_search(toLower(path + fileName), e)) {
      fileIndex->push_back(FileSort{path, fileName, dSE});
    }
  }
  closedir(dir);
}

// EX loopEnd should be 0 if you want to search results and in recent taget= ""
// EX a combanation of everyFileSort() & fileSearchSort()
string ultSort(string searchTarget, size_t loopEnd) {
  string htmlReturn;
  vector<FileSort> files;
  if (searchTarget.length() > 0) {
    // EX the 10 came from the keyword "/search" and html gook "?x="
    searchTarget = searchTarget.substr(10, searchTarget.length() - 6);
  }
  recursiveIndex(searchTarget, "", &files);

  // EX "if" for recent "else" for search
  if (loopEnd > 0) {
    sort(files.begin(), files.end(), [](const FileSort &a, const FileSort &b) {
      return a.date_created > b.date_created;
    });

  } else {
    sort(files.begin(), files.end(), [](const FileSort &a, const FileSort &b) {
      return a.fileName < b.fileName;
    });
    loopEnd = files.size();
  }
  // EX bi-function compatable
  for (size_t i = 0; i < loopEnd; i++) {
    htmlReturn += creatRow(files[i]);
  }
  return htmlReturn;
}

// TODO mearge with other sort Method mabey
// EX sorts the vector indexes that hold the rows & formats for HTML table
string webContentSort(string path) {
  vector<FileSort> webContent = getFileDirectory(path);
  // EX this row is the parentDirectory Row
  string htmlReturn = R"stop(
        <tr>
          <td class="icon" ><img src="/assets/backArrow_icon_edit.png" alt="[DNE]" width="10"></td>
          <td class="filename" ><a href="../"> &lt;&lt;PD </a></td>
          <td class="filesize"> -- </td>
          <td class="lastmodified"> -- </td>
        </tr>
      )stop";

  sort(webContent.begin(), webContent.end(),
       [](const FileSort &a, const FileSort &b) {
         return a.fileName < b.fileName;
       });
  for (size_t i = 0; i < webContent.size(); i++) {
    // TODO should this "if" move to vec creation to eliminate to itterations?
    if (webContent[i].fileName == "." || webContent[i].fileName == "..") {
      webContent.erase(webContent.begin() + (long)i);
    } else {
      htmlReturn += creatRow(webContent[i]);
    }
  }
  return htmlReturn;
}

// TODO finish this func
// EX finds the byte range
bool byteRange(const vector<string> &headers, size_t *start, size_t *end) {
  // void byteRange(const HttpRequest &req) {
  int range[2];
  string target;
  unsigned long targetNumS;
  unsigned long targetNumE;
  for (unsigned long i = 0; i < headers.size(); i++) {
    if (headers[i].find("Range") != std::string::npos) {
      target = headers[i];
    }
  }
  if (target.length() > 1) {
    for (unsigned long i = 0; i < target.length(); i++) {
      if (target[i] == '=') {
        targetNumS = i;
      } else if (target[i] == '-') {
        targetNumE = i;
      }
    }
    range[0] = atoi(target.c_str() + targetNumS + 1);
    range[1] = atoi(target.c_str() + targetNumE + 1);
    *start = (size_t)range[0];
    *end = (size_t)range[1];
    return true;
  }
  return false;
}

// TODO finish the commented code
// EX serves the files
void serveFile(const HttpRequest *req, HttpResponse *resp, const string &type) {
  string path = FilePath(req->RequestUri());
  int fd = open(path.c_str(), 0);

  if (fd == -1) {
    cout << "Unable to open file: " << path << endl;
    resp->SendHtmlResponse("Invalid Request");
    return;
  }

  ssize_t file_size = lseek(fd, 0, SEEK_END);
  ssize_t err = lseek(fd, 0, SEEK_SET);

  // EX debug for lseek
  cout << file_size << ": " << err << ": " << endl;
  // TODO finish the stuff below
  /*size_t start, end;
  if (byteRange(req->Headers(), &start, &end)) {
    if (end == 0) {
      end = (size_t)file_size;
    }
    LOG_INFO("Send partial response: %ld %ld", start, end);
    // resp->SendPartialResponse(type, fd, (size_t)file_size, start, end);
    resp->SendResponse(type, fd, (size_t)file_size);
  } else {*/
  resp->SendResponse(type, fd, (size_t)file_size);
  close(fd);
}

// EX this passes the file type to serveIndexHtml()
string getMimeType(string path) {
  string img = ".jpg";
  string m4v = ".m4v";
  string html = ".html";
  string mp4 = ".mp4";
  string css = ".css";

  if (path.find(img) != std::string::npos) {
    return "image/jpg";
  } else if (path.find(html) != std::string::npos) {
    return "text/html";
  } else if (path.find(css) != std::string::npos) {
    return "text/css";
  } else if (path.find(m4v) != std::string::npos) {
    return "video/x-m4v";
  } else if (path.find(mp4) != std::string::npos) {
    return "video/mp4";
  }

  return "text/plain";
}

// EX The function we want to make the thread run.
void handle(const TcpConnection &conn) {
  auto req = HttpRequest::ParseRequest(conn.fd, recv);
  if (!req) {
    LOG_INFO("INVALID REQUEST...");
    return;
  }
  LOG_INFO("%s request for \"%s\"", req->Method().c_str(),
           req->RequestUri().c_str());

  HttpResponse resp(conn.fd, send);
  // TODO helps with handling brkn requets unfin
  if (req->RequestUri() == "/stop") {
    exit(1);
  }
  if (req->RequestUri().substr(0, 7) == "/search") {
    resp.SendHtmlResponse(htmlFormat(ultSort(req->RequestUri(), 0)));
  } else if (req->RequestUri() == "/recent") {
    // EX reason for the blank string is further explained in the func ultSort
    resp.SendHtmlResponse(htmlFormat(ultSort("", 50)));
  } else {
    string path = FilePath(req->RequestUri());
    if (fs::is_directory(path)) {
      resp.SendHtmlResponse(htmlFormat(webContentSort(req->RequestUri())));
    } else {
      serveFile(req.get(), &resp, getMimeType(path));
    }
  }
}

#if 1
int main(int argc, char *argv[]) {
  int port = 8000;
  if (argc > 1) {
    port = atoi(argv[1]);
    if (port < 1 || port > __SHRT_MAX__) {
      cout << "Invalid port: " << argv[1] << endl;
      return 1;
    }
  }
  TcpServer server(handle);
  server.Run((uint16_t)port, 20);
  return 0;
}

#elif 0

// testing stuff
bool test(const TcpConnection &conn) {
  // test
  return true;
}

int main(int argc, char *argv[]) {
  // test(byteRange(), 2);
  cout << fileSearchSort("/search/bug") << endl;
}

#endif
