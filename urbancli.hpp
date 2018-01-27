#pragma once

#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <cstdlib>

#include <json/json.h>

#include <curl/curl.h>

class Query;

struct Definition {
public:
  std::string definition;
  std::string example;
  std::string author;
  std::string permalink;
  int thumbs_up;
  int thumbs_down;
  int score;
};

class Query {
public:
  Query(std::string term);

  std::vector<Definition> definitions;
  
private:
  std::string send(std::string url);
  void parse(std::string json);
};