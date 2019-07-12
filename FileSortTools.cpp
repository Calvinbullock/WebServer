#include <iostream>
#include <chrono>

using namespace std;

class FileSort {
  std::chrono::nanoseconds date_created;
  string row;

public:
  FileSort(std::chrono::nanoseconds dateIn, string rowIn) {
    row = rowIn;
    date_created = dateIn;
  }

  std::chrono::nanoseconds getDate() const { return date_created; }

  string getRow() { return row; }
};
