// EX creats the html page
#include <string>

const std::string HTML_HEAD = R"RAW(<!DOCTYPE html>
<html>

  <head>
    <title>Movie index</title>
    <meta name="viewport" content="width=device-width,inital-scale=1">
    <link href="/assets/basic.css" rel="stylesheet" type="text/css" media="all">
  </head>

  <body>
    <div>
    </div>
    <p>
      <h2><a href="/">Movie Index</a></h2>

      <form class="mid" action="/">
        Search: <input type="text" name="search"><br>
      </form>

      <table>
        <tr>
          <th></th>
          <th>Name</th>
          <th>Size</th>
          <th>Mod Date</th>
        </tr>
        <tr class="low">
          <td colspan="4">
            <hr>
          </td>
        </tr>
        <!-- END OF HEADER-->
        )RAW";

const std::string HTML_TAIL = R"RAW(
          <!-- START OF FOOTER-->
          <tr class="low">
            <td colspan="4">
              <hr>
            </td>
          </tr>
          <tr>
            <td class="icon"><img src="/assets/folder_icon_edit.png" alt="[DNE]" width="20"></td>
            <td class="filename"><a id="one" href="/?recent">Recently_Added</a><br></td>
            <td class="filesize">--</td>
            <td class="lastmodified">--</td>
          </tr>
        <tr class="low">
          <td colspan="4">
            <hr>
          </td>
        </tr>
      </table>
      <div class="low">v0.11</div>
    </p>
  </body>

</html>)RAW";
