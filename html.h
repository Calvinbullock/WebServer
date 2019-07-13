// EX creats the html page
#include <string>

const std::string HTML_HEAD = R"RAW(<!DOCTYPE html>
  <html>
    <head>
      <title>Movie index</title>
      <meta name="viewport" content="width=device-width,inital-scale=1">
      <style>
      body {
        background-color: #272727;
        color: #ffffff;
      }

      a {
        color: #cccc00;
      }

      a:visited {
        color: #009a00;
      }

      a:hover {
        color: #ffff4d;
      }
      #one {
        color: #ff0000;
      }
    </style>
    </head>

    <body>
      <div>
      </div>
      <p>
        <h2><a href="/">Movie Index</a></h2>
        <a id="one" href="/recent">Recently Added</a><br>
        <table style="width:30%">
          <tr>
            <th>Icon</th>
            <th>Name</th>
            <th>Size</th>
            <th>Last Modified </th>
            <th>Date </th>
          </tr>
          <tr>
            <th colspan="5">
              <hr>
            </th>
          </tr>)RAW";

const std::string HTML_TAIL = R"RAW(<tr>
            <th colspan="5">
              <hr>
            </th>
          </tr>
        </table>
        v.2.1
      </p>
    </body>

  </html>)RAW";
