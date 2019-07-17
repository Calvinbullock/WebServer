#include "http.h"

#include <iostream>
#include <sstream>
#include <string.h>
#include <string>

// For send/recv
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"

#define BUF_SIZE 1024 * 16

using namespace std;

namespace calvin {

void HttpRequest::Print() const {
  cout << "##### HEADER #######" << endl;
  cout << "Method: " << method_ << endl;
  cout << "URI: " << request_uri_ << endl;
  cout << "Version: " << http_version_ << endl;
  for (auto str : headers_) {
    cout << str << endl;
  }
  cout << content_ << endl;
  cout << "##### END #######" << endl;
}

HttpRequest HttpRequest::ParseRequest(int fd, HttpRequest::RecvFunc *recv) {
  LOG_INFO("parsing HTTP request (fd: %d)", fd);

  char buffer[BUF_SIZE];
  ssize_t len = recv(fd, buffer, BUF_SIZE - 1, 0);
  if (len < 0) {
    LOG_WARNING("unable to recv request (fd: %d)", fd);
    return HttpRequest();
  }
  buffer[len] = '\0';

  // cout << "START " << len << " ###########################" << endl;
  // cout << string(buffer, len) << endl;
  // cout << "END   ################################" << endl;

  const char *ptr;
  const char *curr = buffer;
  const char *content = NULL;
  vector<string> headers;

  while ((ptr = strstr(curr, "\r\n")) != NULL) {
    if (ptr - buffer > len) { // Exceeded buffer size.
      LOG_WARNING("buffer overflow (fd: %d)", fd);
      return HttpRequest();
    }
    if (ptr - curr == 0) { // End of header.
      content = ptr + 2;
      if (headers.size() == 0) {
        LOG_WARNING("empty request? (fd: %d)", fd);
        return HttpRequest();
      }

      size_t l1 = headers[0].find_first_of(' ');
      string method(headers[0], 0, l1);

      size_t l2 = headers[0].find_first_of(' ', l1 + 1);
      string request_uri(headers[0], l1 + 1, l2 - (l1 + 1));

      string http_version(headers[0], l2 + 1);
      headers.erase(headers.begin());

      size_t content_length = (size_t)(len - (content - buffer));

      LOG_INFO("parsed HTTP request (fd: %d, %s:%s, len: %ld)", fd,
               method.c_str(), request_uri.c_str(), content_length);
      return HttpRequest(method, request_uri, http_version, headers, content,
                         content_length);
    }
    headers.push_back(string(curr, (size_t)(ptr - curr)));
    // cout << "header = " << headers[headers.size() - 1] << endl;
    curr = ptr + 2;
  }
  LOG_WARNING("invalid request? (fd: %d)", fd);
  return HttpRequest();
}

void HttpResponse::Send(int fd) {
  if (fd_data == -1) {
    LOG_INFO("sending response, MIME: %s, length: %lu", type.c_str(),
             content.size());

    stringstream buf;

    buf << "HTTP/1.0 200 OK\r\n";
    buf << "Content-type:" << type << "\r\n";
    buf << "Content-Length:" << content.size() << "\r\n";
    buf << "Accept-Ranges: bytes"
        << "\r\n";
    buf << "\r\n";
    buf << content;

    send(fd, buf.str().c_str(), buf.str().size(), 0);
  } else {
    LOG_INFO("sending response, MIME: %s, length: %lu, fd: %d", type.c_str(),
             fd_length, fd_data);
    stringstream buf;

    buf << "HTTP/1.0 200 OK\r\n";
    buf << "Content-type:" << type << "\r\n";
    buf << "Content-Length:" << fd_length << "\r\n";
    buf << "Accept-Ranges: bytes"
        << "\r\n";
    buf << "\r\n";
    send(fd, buf.str().c_str(), buf.str().size(), 0);

    ssize_t bufx = 1;
    char buf1[4096 * 32];
    while (bufx > 0) {
      bufx = read(fd_data, buf1, 4096 * 32);
      ssize_t rv = send(fd, buf1, (size_t)(bufx), 0);
      if (rv != bufx) {
        break;
      }
    }

    close(fd_data);
  }
}

} // namespace calvin
