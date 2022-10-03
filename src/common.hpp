#pragma once

#include <mova.h>
#include <mgame.h>
#include <glm.hpp>
#include <sstream>

using glm::vec2;
using glm::vec3;
using glm::vec4;

std::string encodeRLE(const std::string& str) {
  std::string letters;

  for (int j = 0; j < str.size(); j++) {
    int count = 1;
    while (str[j] == str[j + 1]) {
      count++;
      j++;
    }
    if (count != 1) letters += std::to_string(count);
    letters.push_back(str[j]);
  }
  return letters;
}

std::string decodeRLE(std::string s) {
  int i = 0;
  std::string res = "";
  while (i < s.size()) {
    if (!isdigit(s[i])) {
      res += s[i];
      i++;
      continue;
    }
    int j = i;
    while (isdigit(s[i]) && (i < s.size())) i++;
    int length = std::stoi(s.substr(j, i - j + 1));
    res += std::string(length, s[i]);
    i++;
  }
  return res;
}

template <typename T>
std::string to_string(const T a_value, const int n = 6){
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}
