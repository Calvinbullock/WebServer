#ifndef CALVIN_HANDLER_H_
#define CALVIN_HANDLER_H_

#include <memory>
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
  bool recent;
};

bool parseRequestUri(const std::string &uri, UriInfo *uri_info);

void handle(const TcpConnection &conn);

} // namespace calvin

#endif // CALVIN_HANDLER_H_
