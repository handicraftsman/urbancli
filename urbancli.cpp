#include "urbancli.hpp"

Query::Query(std::string _term) : term(_term) {
  std::string t;

  for (char c : term) {
    if (c == ' ' || c == '\t' || c == '&') t += '-';
    if (c == '\r' || c == '\n') t += '\0';
    if (c >= '0') t += c;
  }
  const std::string url = "http://api.urbandictionary.com/v0/define?term=" + t;

  parse(send(url));
}

static unsigned writer(char* data, size_t size, size_t nmemb, std::string* writer) {
  if (writer == NULL)
    return 0;
  writer->append(data, size*nmemb);
  return size * nmemb;
}

std::string Query::send(std::string url) {
  std::string buf;
  char ebuf[CURL_ERROR_SIZE];

  CURL* curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
  curl_easy_setopt(curl, CURLOPT_HEADERDATA,    NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &buf);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,   ebuf);

  CURLcode code = curl_easy_perform(curl);

  curl_easy_cleanup(curl);

  if (code != CURLE_OK) throw std::runtime_error("CURL error: failed to get `" + url + "` [" + ebuf + "]");

  return buf;
}

void Query::parse(std::string json) {
  Json::Value root;
  std::string errs;

  Json::CharReaderBuilder b;
  std::stringstream s(json);
  bool ok = parseFromStream(b, s, &root, &errs);
  if (!ok) throw std::runtime_error("JSON error: " + errs);

  if (!root.isObject()) throw std::runtime_error("JSON error: expected Object, got something else");
  for (auto val : root["list"]) {
    if (val.isObject()) {
      Definition def;

      #define rep(s) std::regex_replace(std::regex_replace(s, std::regex("[\r\n]*$"), ""), std::regex("[\r]?\n[\r\n]*"), "\n    ")

      def.definition = rep(val["definition"].asString());
      def.example = rep(val["example"].asString());
      def.author = val["author"].asString();
      def.permalink = val["permalink"].asString();
      def.thumbs_up = val["thumbs_up"].asInt();
      def.thumbs_down = val["thumbs_down"].asInt();
      def.score = def.thumbs_up - def.thumbs_down;

      #undef rep

      definitions.push_back(def);
    }
  }

  std::stable_sort(definitions.begin(), definitions.end(), [] (Definition d1, Definition d2) { return d1.score < d2.score; });
}

void printQuery(Query& query) {
  std::cout << std::endl;

  for (auto it = query.definitions.begin(); it != query.definitions.end(); ++it) {
    Definition& def = *it;

    std::cout << "\x1b[1mTerm:\x1b[0m " << query.term << std::endl;
    std::cout << "\x1b[1mDefinition:\x1b[0m" << std::endl << "    " << def.definition << std::endl;
    std::cout << "\x1b[1mExample(s):\x1b[0m" << std::endl << "    " << def.example << std::endl;
    std::cout << "\x1b[1mAuthor:\x1b[0m " << def.author << std::endl;
    std::cout << "\x1b[1mScore:\x1b[0m -" << def.thumbs_down << " | " << def.score << " | +" << def.thumbs_up << std::endl; 
    std::cout << "\x1b[1mPermalink:\x1b[0m " << def.permalink << std::endl;

    if (it != query.definitions.end() - 1) {
      std::cout << std::endl << "================================================================" << std::endl << std::endl;
    } else {
      std::cout << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  std::cin.unsetf(std::ios_base::skipws);
  curl_global_init(CURL_GLOBAL_ALL);
 
  std::cout << "UrbanCLI by Nickolay Ilyushin (handicraftsman) <nickolay02@inbox.ru>" << std::endl;
  
  if (argc > 1) {
    std::string term;
    for (int i = 1; i < argc; ++i) {
      if (i != 1)
        term.append(" ");
      term.append(argv[i]);
      Query query(term);
      printQuery(query);
    }
    return 0;
  }

  std::cout << "Type .quit or .exit or press Ctrl+D to exit" << std::endl << std::endl;

  for (;;) {
    std::cout << "UD> ";
    
    std::string term;
    
    if (!std::getline(std::cin, term)) break;
    if (term == ".exit" || term == ".quit") break;

    Query query(term);
    printQuery(query);    
  }

  return 0;
}