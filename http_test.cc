// What we are testing
#include "http.h"

#include "gtest/gtest.h"
#include <cstdio>
#include <fstream>
#include <iostream>

using namespace std;

FILE *GetFileForTest(const char *data, const char *tmp_filename) {
  ofstream out_file;

  out_file.open(tmp_filename, ios_base::trunc);
  if (!out_file.is_open()) {
    exit(1);
  }
  out_file << data;
  out_file.close();

  FILE *fd = fopen(tmp_filename, "r");
  if (fd == nullptr) {
    exit(1);
  }

  return fd;
}

TEST(HttpTest, test1_success) {
  auto *fd = GetFileForTest("GET / HTTP/1.1\r\n"
                            "Host: 192.168.86.2\r\n"
                            "User-Agent: curl/7.52.1\r\n"
                            "Accept: */*\r\n"
                            "\r\n",
                            "/tmp/httptest_test1");

  auto r = calvin::HttpRequest::ParseRequest(
      fileno(fd),
      [](int fd, void *d, size_t bytes, int) { return read(fd, d, bytes); });

  EXPECT_EQ("GET", r.Method());
  EXPECT_EQ("/", r.RequestUri());
  EXPECT_EQ((vector<string>{"Host: 192.168.86.2", "User-Agent: curl/7.52.1",
                            "Accept: */*"}),
            r.Headers());
  EXPECT_EQ("", r.Content());
  fclose(fd);
}

TEST(HttpTest, test2_success) {
  auto *fd = GetFileForTest(
      "GET /hello.htm HTTP/1.1\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Host: www.tutorialspoint.com\r\n"
      "Accept-Language: en-us\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: Keep-Alive\r\n"
      "\r\n",
      "/tmp/httptest_test2");
  auto r = calvin::HttpRequest::ParseRequest(
      fileno(fd),
      [](int fd, void *d, size_t bytes, int) { return read(fd, d, bytes); });

  EXPECT_EQ("GET", r.Method());
  EXPECT_EQ("/hello.htm", r.RequestUri());
  EXPECT_EQ((vector<string>{
                "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)",
                "Host: www.tutorialspoint.com", "Accept-Language: en-us",
                "Accept-Encoding: gzip, deflate", "Connection: Keep-Alive"}),
            r.Headers());
  EXPECT_EQ("", r.Content());
  fclose(fd);
  ofstream out_file;
}
