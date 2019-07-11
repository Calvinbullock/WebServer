#include <iostream>
using namespace std;

class FileSort {
  int date_created;
  string row;

public:
  FileSort(int dateIn, string rowIn) {
    row = rowIn;
    date_created = dateIn;
  }

  int getDate() const { return date_created; }

  string getRow() { return row; }
};