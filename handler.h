#ifndef CALVIN_HANDLER_H_
#define CALVIN_HANDLER_H_

#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "server.h"

namespace calvin {

std::string toLower(std::string str);
std::string formatFileSize(size_t size);
std::string getMimeType(std::string path);

struct UriInfo {
  std::string path;
  std::string search_param;
  bool recent = false;
};

void parseRequestUri(const std::string &uri, UriInfo *uri_info);

struct FileSort {
  std::string path;
  std::string fileName;
  std::chrono::nanoseconds date_created;
  size_t fileSize = 0;
  bool isDirectory = false;

  std::string getLink() { return path + fileName; }
};
std::vector<FileSort> listFiles(const std::string &path, bool recursive,
                                const std::regex *re);

void handle(const TcpConnection &conn);

} // namespace calvin

#endif // CALVIN_HANDLER_H_
