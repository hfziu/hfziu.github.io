// generate index.html for posts/*.txt
// C++17 and libboost required

#include <algorithm>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
using std::string, std::string_view, std::vector;

const string_view kPostDir = "posts";
const string_view kPostIndexFile = "posts/index.html";
const string_view kIndexPageTitle = "~hfziu/posts";
const string_view kIndexPageDesc = "Index page of posts";

// build the entry list in reverse chronological order
vector<fs::path> BuildEntryList() {
  vector<fs::path> entries;
  for (const auto &entry : fs::directory_iterator(kPostDir)) {
    if (entry.path().extension() != ".txt")
      continue;
    entries.push_back(entry.path());
  }
  std::sort(entries.begin(), entries.end(), std::greater<string>());
  return entries;
}

string ReadFile(const fs::path &path) {
  std::ifstream f(path, std::ios::in);
  const auto size = fs::file_size(path);
  // WARNING: we assume the post file is not too big
  string content(size, '\0');
  f.read(content.data(), size);
  return content;
}

class Post {
public:
  Post(const fs::path &path) : content_(ReadFile(path)) {
    // parse the date from the file name
    filename_ = path.filename();
    date_ = filename_.substr(0, 10);
    // validate the date string
    try {
      boost::gregorian::date d(boost::gregorian::from_simple_string(date_));
    } catch (const std::exception &e) {
      std::cerr << "Invalid date format: " << date_ << std::endl;
      exit(1);
    }
    ScanMetadata();
  }

  string_view filename() const { return filename_; }
  string_view date() const { return date_; }
  string_view title() const { return title_; }
  string_view summary() const { return summary_; }
  string_view tags() const { return tags_; }
  string_view uuid() const { return uuid_; }

  string DebugString() const {
    return "date: " + date_ + ", title: " + title_ + ", summary: " + summary_ +
           ", tags: " + tags_ + ", uuid: " + uuid_;
  }

private:
  string_view ParseMetadata(const string &tag) {
    size_t pos = content_.find(tag);
    if (pos != string::npos) {
      pos += tag.size();
      size_t end = content_.find('\n', pos);
      if (end == string::npos)
        end = content_.size();
      string_view result = string_view(content_).substr(pos, end - pos);
      result.remove_prefix(
          std::min(result.find_first_not_of(": \t"), result.size()));
      return result;
    }
    return {};
  }

  void ScanMetadata() {
    // scan the content for metadata containing
    // $#t: title, $#s: summary, $#o: tags, $#u: uuid
    // if not found, use the first line as title, without the leading '#'
    title_ = ParseMetadata("$#t");
    if (title_.empty()) {
      size_t pos = content_.find('\n');
      title_ = content_.substr(0, pos);
      title_.erase(0, title_.find_first_not_of("#: \t"));
      if (title_.empty()) {
        title_ = "Untitled";
        std::cerr << "No title: " << filename_ << std::endl;
      }
    }
    summary_ = ParseMetadata("$#s");
    tags_ = ParseMetadata("$#o");
    uuid_ = ParseMetadata("$#u");
  }

  const string content_;
  string filename_;
  string date_;
  string title_;
  string summary_;
  string tags_;
  string uuid_;
};

void GenIndexHtml(const vector<fs::path> &entries) {
  std::ofstream f(kPostIndexFile, std::ios::out);
  f << R"(<!DOCTYPE html>
<html lang="en">
  <head>
    <title>)"
    << kIndexPageTitle << R"(</title>
    <link rel="stylesheet" href="/assets/site.css" />
    <meta name="description" content=")"
    << kIndexPageDesc << R"(" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
  </head>
  <body>
    <h1 id="posts">posts</h1>
    <a href="/">Go back</a>
    <ul>
)";
  for (const auto &entry : entries) {
    Post post(entry);
    f << "      <li>" << post.date() << ": <a href=\"" << post.filename()
      << "\">" << post.title() << "</a></li>\n";
  }
  f << R"(    </ul>
  </body>
</html>)";
}

int main() {
  auto entries = BuildEntryList();
  GenIndexHtml(entries);
  return 0;
}
