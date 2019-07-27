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
using namespace std;

namespace calvin {

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

// EX creats the html page that is sent to the conected device
string htmlFormat(string tableRows) {
  return HTML_HEAD + tableRows + HTML_TAIL;
}

// EX converts all char's in a string to lower case
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

// EX will recursively find the size of directerys
unsigned long directorySize(string path) {
  DIR *dir;
  struct dirent *ent;
  unsigned long byteSize = 0;
  // EX checks if directery is open
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("");
    return byteSize;
  }

  while ((ent = readdir(dir)) != NULL) {
    string fileName = string(ent->d_name);
    string pathy = path + "/" + fileName;
    if (fs::is_directory(pathy)) {
      if ((fileName != ".") && (fileName != "..")) {
        byteSize += directorySize(pathy);
      }
    } else {
      fs::path p = FilePath(pathy);
      byteSize += fs::file_size(pathy);
    }
  }
  closedir(dir);
  return byteSize;
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

  // EX finds the file icon
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
    row[2] = "<td class=\"filesize\" > " +
             formatFileSize(directorySize(filepath)) + " </td>";
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

/* EX creats a vector then fills it with html strings that make rows for the
 table */
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

  // EX print all the files and directories within directory
  int i = 0;
  while ((ent = readdir(dir)) != NULL) {
    // TODO lable what "fileName" is
    string fileName = string(ent->d_name);
    webContent.push_back(FileSort{path, fileName});
    i++;
  }
  closedir(dir);
  return webContent;
}

/* EX recurses trough each directery and adds every file to a vec send
 * target a blank string to just get every file and its size*/
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

// EX if you want to use recent, SearchTaget needs to be ""
/* EX this recursise trought every directer to eather find the most recently
 * changed file or the file that has search target in it */
string ultSort(string searchTarget, size_t loopEnd) {
  string htmlReturn;
  vector<FileSort> files;
  // EX 10 as that is the min the substring needs
  if (searchTarget.length() > 10) {
    // EX the 10 came from the keyword "/search" and html gook "?x="
    searchTarget = searchTarget.substr(10, searchTarget.length() - 6);
  }
  recursiveIndex(searchTarget, "", &files);

  // EX "if" for recent "else" for search
  if (searchTarget.length() == 0) {
    sort(files.begin(), files.end(), [](const FileSort &a, const FileSort &b) {
      return a.date_created > b.date_created;
    });

  } else {
    sort(files.begin(), files.end(), [](const FileSort &a, const FileSort &b) {
      return a.fileName < b.fileName;
    });
  }

  // TODO this was casueing an std::bad_alloc keep an eye out
  for (size_t i = 0; i < loopEnd && i < files.size(); i++) {
    htmlReturn += creatRow(files[i]);
  }
  return htmlReturn;
}

// TODO mearge with ultSort() mabey
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
    // EX this "if" pulls out the directery traversive links
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

  // EX "cout" is debug for lseek
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

  // EX based on file type sends the corasponding mime type
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
  // TODO helps with handling brkn requets [unfin]
  if (req->RequestUri() == "/stop") {
    exit(1);
  }
  if (req->RequestUri().substr(0, 7) == "/search") {
    resp.SendHtmlResponse(htmlFormat(ultSort(req->RequestUri(), 10)));
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
}
