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

#include "handler.h"
#include "html.h"
#include "http.h"
#include "log.h"
#include "server.h"

using namespace calvin;
using namespace std;

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
