// HttpRequest/HttpResponse classes

#ifndef CALVIN_HTTP_H_
#define CALVIN_HTTP_H_

#include <string>
#include <vector>

namespace calvin {

class HttpRequest {
public:
  typedef ssize_t RecvFunc(int sockfd, void *buf, size_t len, int flags);

  // Use this to parse an http request, if it returns an empty HttpRequest
  // there was an error during parsing..
  static HttpRequest ParseRequest(int fd, RecvFunc *recv);

  void Print() const;

  const std::string &Method() const { return method_; }
  const std::string &RequestUri() const { return request_uri_; }
  const std::vector<std::string> &Headers() const { return headers_; }
  const std::string &Content() const { return content_; }

private:
  HttpRequest() {}
  HttpRequest(const std::string &method, const std::string &request_uri,
              const std::string &http_version,
              const std::vector<std::string> headers, const char *content,
              const size_t content_length)
      : method_(method), request_uri_(request_uri), http_version_(http_version),
        headers_(headers), content_(content, content_length) {}

  const std::string method_;
  const std::string request_uri_;
  const std::string http_version_;
  const std::vector<std::string> headers_; // should be a map
  const std::string content_;
};

// TODO: This class is really ugly at this point. Just don't even look.
class HttpResponse {
public:
  std::string content;
  std::string type;
  int fd_data = -1;
  size_t fd_length;

  void SetHtmlContent(const std::string &contentx) {
    this->content = contentx;
    this->type = "text/html";
  }

  void SetContent(const std::string typex, int data_fd, size_t length) {
    this->type = typex;
    this->fd_data = data_fd;
    this->fd_length = length;
  }

  void Send(int fd);
};

} // namespace calvin

#endif // CALVIN_HTTP_H_
