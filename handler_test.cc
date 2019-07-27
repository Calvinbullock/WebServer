// Code under test.
#include "handler.h"

#include <cstdio>
#include <fstream>
#include <iostream>

#include "log.h"
#include "gtest/gtest.h"

using namespace std;
using namespace calvin;

TEST(HandlerTest, test_toLower) {
  EXPECT_EQ(toLower("abC"), "abc");
  EXPECT_EQ(toLower("DON"), "don");
  EXPECT_EQ(toLower("abc-*"), "abc-*");
}

TEST(HandlerTest, test_formatFileSize) {
  EXPECT_EQ(formatFileSize(0), "0 B");
  EXPECT_EQ(formatFileSize(1234), "1 KB");
  EXPECT_EQ(formatFileSize(837483743), "798 MB");
  EXPECT_EQ(formatFileSize(935234233423), "871 GB");
  EXPECT_EQ(formatFileSize(234102349871234983LLU), "207 PB");
}

void parseRequestTester(string uri, string path, string search, bool recent) {
  cout << uri << endl;
  UriInfo info;
  parseRequestUri(uri, &info);
  EXPECT_EQ(info.path, path);
  EXPECT_EQ(info.search_param, search);
  EXPECT_EQ(info.recent, recent);
}

TEST(HandlerTest, test_parseRequestUri) {
  parseRequestTester("/?search=foo", "/", "foo", false);
  parseRequestTester("/test/?search=foo", "/test/", "foo", false);

  parseRequestTester("/?recent", "/", "", true);
  parseRequestTester("/iuoiuo/?recent", "/iuoiuo/", "", true);

  parseRequestTester("/", "/", "", false);
  parseRequestTester("/good", "/good", "", false);
  // Should test URI like:
  // "/?search=foo"
  // "/asdf/?search=foo"
  // "/?recent

  // "/asdfasd/asfasdf/?recent
  // "/good.html"
  // "/good/
  // "/stop
}
