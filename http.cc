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

#define BUF_SIZE 1024 * 16

using namespace std;

namespace obsequi {

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

HttpRequest HttpRequest::parse(int fd) {
  char buffer[BUF_SIZE];
  ssize_t len = recv(fd, buffer, BUF_SIZE - 1, 0);
  // TODO: We should verify it is not negative.
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
      cerr << "BUFFER OVERFLOW" << endl;
      return HttpRequest();
    }
    if (ptr - curr == 0) { // End of header.
      content = ptr + 2;
      if (headers.size() == 0)
        return HttpRequest();

      size_t l1 = headers[0].find_first_of(' ');
      string method(headers[0], 0, l1);

      size_t l2 = headers[0].find_first_of(' ', l1 + 1);
      string request_uri(headers[0], l1 + 1, l2 - (l1 + 1));

      string http_version(headers[0], l2 + 1);
      headers.erase(headers.begin());

      return HttpRequest(method, request_uri, http_version, headers, content,
                         (size_t)(len - (content - buffer)));
    }
    headers.push_back(string(curr, (size_t)(ptr - curr)));
    curr = ptr + 2;
  }
  cerr << "END OF BUFFER" << endl;
  return HttpRequest();
}

void HttpResponse::Send(int fd) {
  printf("FD DESCRIPTOR: %d\n", fd_data);
  if (fd_data == -1) {
    stringstream buf;

    buf << "HTTP/1.0 200 OK\r\n";
    buf << "Content-type:" << type << "\r\n";
    buf << "Content-Length:" << content.size() << "\r\n";
    buf << "\r\n";
    buf << content;

    send(fd, buf.str().c_str(), buf.str().size(), 0);
  } else {
    printf("Start\n");
    stringstream buf;

    buf << "HTTP/1.0 200 OK\r\n";
    buf << "Content-type:" << type << "\r\n";
    buf << "Content-Length:" << fd_length << "\r\n";
    buf << "\r\n";
    send(fd, buf.str().c_str(), buf.str().size(), 0);
    printf("Start\n");

    ssize_t bufx = 1;
    char buf1[4096 * 32];
    while (bufx > 0) {
      bufx = read(fd_data, buf1, 4096 * 32);
      // kjjprintf("%d %d %d\n", fd_data, fd, bufx);
      ssize_t rv = send(fd, buf1, (size_t)(bufx), 0);
      // printf("%d %d %d %d\n", fd_data, fd, bufx, rv);
      if (rv != bufx) {
        break;
      }
    }

    // sendfile(fd, fd_data, 0, fd_length);
    close(fd_data);
    printf("End\n");
  }
}

} // namespace obsequi
