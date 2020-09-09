/*
                                    88888888
                                  888888888888
                                 88888888888888
                                8888888888888888
                               888888888888888888
                              888888  8888  888888
                              88888    88    88888
                              888888  8888  888888
                              88888888888888888888
                              88888888888888888888
                             8888888888888888888888
                          8888888888888888888888888888
                        88888888888888888888888888888888
                              88888888888888888888
                            888888888888888888888888
                           888888  8888888888  888888
                           888     8888  8888     888
                                   888    888

                                   OCTOBANANA

Licensed under the MIT License

Copyright (c) 2019 Brett Robinson <https://octobanana.com/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ob/string.hh"

#include <cstddef>

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>
#include <limits>
#include <utility>
#include <optional>
#include <regex>

namespace OB::String
{

std::string plural(std::string const& str, std::size_t num)
{
  if (num == 1)
  {
    return str;
  }

  return str + "s";
}

std::string plural(std::string const& str, std::string const& end, std::size_t num)
{
  if (num == 1)
  {
    return str;
  }

  return str + end;
}

std::string file(std::string const& str)
{
  std::ifstream file {str};
  file.seekg(0, std::ios::end);
  std::size_t size (static_cast<std::size_t>(file.tellg()));
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], static_cast<std::streamsize>(size));

  return content;
}

std::string replace(std::string str, std::string const& key, std::string const& val,
  std::size_t size)
{
  std::size_t pos {0};

  while (size-- > 0)
  {
    pos = str.find(key, pos);

    if (pos == std::string::npos)
    {
      break;
    }

    str.replace(pos, key.size(), val);
    pos += val.size();
  }

  return str;
}

std::pair<std::string, std::size_t> replace(std::string str, std::regex const& rx, std::function<std::string(std::smatch const&)> fn, std::size_t size)
{
  std::size_t pos {0};
  std::size_t count {0};
  std::string tmp {str};
  std::string val;
  std::smatch match;

  while ((size-- > 0) && std::regex_search(tmp, match, rx, std::regex_constants::match_not_null))
  {
    ++count;
    pos += match.prefix().str().size();
    val = fn(match);
    str.replace(pos, match.str().size(), val);
    pos += val.size();
    tmp = match.suffix();
  }

  return {str, count};
}

std::string format(std::string str, std::unordered_map<std::string, std::string> args)
{
  if (args.empty())
  {
    return str;
  }

  std::string res = str;
  std::size_t pos {0};
  std::smatch match;
  std::regex const rx {"\\{([^\\:]+?)\\}"};

  while (std::regex_search(res, match, rx, std::regex_constants::match_not_null))
  {
    std::string m {match[0]};
    std::string arg_pos {match[1]};
    pos += static_cast<std::size_t>(match.prefix().length());

    if (args.find(arg_pos) == args.end())
    {
      ++pos;
      res = res.substr(std::string(match.prefix()).size() + 1);

      continue;
    }

    str.replace(pos, m.size(), args[arg_pos]);
    pos += args[arg_pos].size();
    res = match.suffix();
  }

  return str;
}

std::string xformat(std::string str, std::unordered_map<std::string, std::string> args)
{
  if (args.empty())
  {
    return str;
  }

  std::string res = str;
  std::size_t pos {0};
  std::smatch match;
  std::regex const rx {"\\{(\\w+)(?:(:[^\\r]*?:\\1)?)\\}"};

  while (std::regex_search(res, match, rx, std::regex_constants::match_not_null))
  {
    bool is_simple {true};
    std::string m {match[0]};
    std::string first {match[1]};
    std::string second {match[2]};
    pos += std::string(match.prefix()).size();

    if (second.size() > 0)
    {
      is_simple = false;
    }

    if (args.find(first) == args.end())
    {
      pos += m.size();
      res = match.suffix();

      continue;
    }

    if (is_simple)
    {
      str.replace(pos, m.size(), args[first]);
      pos += args[first].size();
      res = match.suffix();
    }
    else
    {
      std::smatch match_complex;
      std::regex rx2 {"^:([*]{1})(.+)([a-z0-9]{1}):([^\\r]+):" + first + "$"};

      if (std::regex_match(second, match_complex, rx2, std::regex_constants::match_not_null))
      {
        std::string type {match_complex[1]};
        std::string delim {match_complex[2]};
        std::string index_char {match_complex[3]};
        std::string rstr {match_complex[4]};

        auto vec = String::split(args[first], delim);
        std::stringstream ss;

        for (auto const& e : vec)
        {
          ss << String::replace(rstr, "[" + index_char + "]", e);
        }

        auto nstr = String::xformat(ss.str(), args);
        str.replace(pos, m.size(), nstr);
        pos += nstr.size();
        res = match.suffix();
      }
      else
      {
        pos += m.size();
        res = match.suffix();
      }
    }
  }

  return str;
}

std::string to_string(double const val, int const pre)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(pre) << val;

  return ss.str();
}

std::vector<std::string> split(std::string const& str,
  std::string const& delim, std::size_t size)
{
  std::vector<std::string> vtok;
  std::size_t start {0};
  auto end = str.find(delim);

  while ((size-- > 0) && (end != std::string::npos))
  {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.size();
    end = str.find(delim, start);
  }

  vtok.emplace_back(str.substr(start, end));

  return vtok;
}

std::vector<std::string_view> split_view(std::string_view str,
  std::string_view delim, std::size_t size)
{
  std::vector<std::string_view> vtok;
  std::size_t start {0};
  auto end = str.find(delim);

  while ((size-- > 0) && (end != std::string_view::npos))
  {
    vtok.emplace_back(str.data() + start, end - start);
    start = end + delim.size();
    end = str.find(delim, start);
  }

  vtok.emplace_back(str.data() + start, str.size() - start);

  return vtok;
}

std::string uppercase(std::string const& str)
{
  auto const to_upper = [](char& c)
  {
    if (c >= 'a' && c <= 'z')
    {
      c += 'A' - 'a';
    }

    return c;
  };

  std::string res {str};

  for (char& c : res)
  {
    c = to_upper(c);
  }

  return res;
}

std::string lowercase(std::string const& str)
{
  auto const to_lower = [](char& c)
  {
    if (c >= 'A' && c <= 'Z')
    {
      c += 'a' - 'A';
    }

    return c;
  };

  std::string res {str};

  for (char& c : res)
  {
    c = to_lower(c);
  }

  return res;
}

std::string trim(std::string str)
{
  auto start = str.find_first_not_of(" \t\n\r\f\v");

  if (start != std::string::npos)
  {
    auto end = str.find_last_not_of(" \t\n\r\f\v");
    str = str.substr(start, end - start + 1);

    return str;
  }

  return {};
}

bool assert_rx(std::string const& str, std::regex rx)
{
  std::smatch m;

  if (std::regex_match(str, m, rx, std::regex_constants::match_not_null))
  {
    return true;
  }

  return false;
}

std::optional<std::vector<std::string>> match(std::string const& str, std::regex rx)
{
  std::smatch m;

  if (std::regex_match(str, m, rx, std::regex_constants::match_not_null))
  {
    std::vector<std::string> v;

    for (auto const& e : m)
    {
      v.emplace_back(std::string(e));
    }

    return v;
  }

  return {};
}

std::string repeat(std::size_t const num, std::string const& str)
{
  if (num == 0)
  {
    return {};
  }

  if (num == 1)
  {
    return str;
  }

  std::string res;
  res.reserve(str.size() * num);

  for (std::size_t i {0}; i < num; ++i)
  {
    res += str;
  }

  return res;
}

bool starts_with(std::string const& str, std::string const& val)
{
  if (str.empty() || str.size() < val.size())
  {
    return false;
  }

  if (str.compare(0, val.size(), val) == 0)
  {
    return true;
  }

  return false;
}

std::string escape(std::string str)
{
  for (std::size_t pos = 0; (pos = str.find_first_of("\n\t\r\a\b\f\v\"\'\?", pos)); ++pos)
  {
    if (pos == std::string::npos)
    {
      break;
    }

    switch (str.at(pos))
    {
      case '\n':
      {
        str.replace(pos++, 1, "\\n");
        break;
      }
      case '\t':
      {
        str.replace(pos++, 1, "\\t");
        break;
      }
      case '\r':
      {
        str.replace(pos++, 1, "\\r");
        break;
      }
      case '\a':
      {
        str.replace(pos++, 1, "\\a");
        break;
      }
      case '\b':
      {
        str.replace(pos++, 1, "\\b");
        break;
      }
      case '\f':
      {
        str.replace(pos++, 1, "\\f");
        break;
      }
      case '\v':
      {
        str.replace(pos++, 1, "\\v");
        break;
      }
      case '\?':
      {
        str.replace(pos++, 1, "\\?");
        break;
      }
      case '\'':
      {
        str.replace(pos++, 1, "\\'");
        break;
      }
      case '"':
      {
        str.replace(pos++, 1, "\\\"");
        break;
      }
      default:
      {
        break;
      }
    }
  }

  return str;
}

std::string unescape(std::string str)
{
  for (std::size_t pos = 0; (pos = str.find("\\", pos)); ++pos)
  {
    if (pos == std::string::npos || pos + 1 == std::string::npos)
    {
      break;
    }

    switch (str.at(pos + 1))
    {
      case 'n':
      {
        str.replace(pos, 2, "\n");
        break;
      }
      case 't':
      {
        str.replace(pos, 2, "\t");
        break;
      }
      case 'r':
      {
        str.replace(pos, 2, "\r");
        break;
      }
      case 'a':
      {
        str.replace(pos, 2, "\a");
        break;
      }
      case 'b':
      {
        str.replace(pos, 2, "\b");
        break;
      }
      case 'f':
      {
        str.replace(pos, 2, "\f");
        break;
      }
      case 'v':
      {
        str.replace(pos, 2, "\v");
        break;
      }
      case '?':
      {
        str.replace(pos, 2, "\?");
        break;
      }
      case '\'':
      {
        str.replace(pos, 2, "'");
        break;
      }
      case '"':
      {
        str.replace(pos, 2, "\"");
        break;
      }
      default:
      {
        break;
      }
    }
  }

  return str;
}

std::size_t count(std::string const& str, std::string const& val)
{
  std::size_t pos {0};
  std::size_t count {0};

  for (;;)
  {
    pos = str.find(val, pos);

    if (pos == std::string::npos)
    {
      break;
    }

    ++count;
    ++pos;
  }

  return count;
}

std::size_t damerau_levenshtein(std::string const& lhs, std::string const& rhs,
  std::size_t const weight_insert, std::size_t const weight_substitute,
  std::size_t const weight_delete, std::size_t const weight_transpose)
{
  if (lhs == rhs)
  {
    return 0;
  }

  std::string_view lhsv {lhs};
  std::string_view rhsv {rhs};

  bool swapped {false};
  if (lhsv.size() > rhsv.size())
  {
    swapped = true;
    std::swap(lhsv, rhsv);
  }

  for (std::size_t i = 0; i < lhsv.size(); ++i)
  {
    if (lhsv.at(i) != rhsv.at(i))
    {
      if (i)
      {
        lhsv.substr(i);
        rhsv.substr(i);
      }

      break;
    }
  }

  for (std::size_t i = 0; i < lhsv.size(); ++i)
  {
    if (lhsv.at(lhsv.size() - 1 - i) != rhsv.at(rhsv.size() - 1 - i))
    {
      if (i)
      {
        lhsv.substr(0, lhsv.size() - 1 - i);
        rhsv.substr(0, rhsv.size() - 1 - i);
      }

      break;
    }
  }

  if (swapped)
  {
    std::swap(lhsv, rhsv);
  }

  if (lhsv.empty())
  {
    return rhsv.size() * weight_insert;
  }

  if (rhsv.empty())
  {
    return lhsv.size() * weight_delete;
  }

  std::vector<std::size_t> v0 (rhsv.size() + 1, 0);
  std::vector<std::size_t> v1 (rhsv.size() + 1, 0);
  std::vector<std::size_t> v2 (rhsv.size() + 1, 0);

  for (std::size_t i = 0; i <= rhsv.size(); ++i)
  {
    v1.at(i) = i * weight_insert;
  }

  for (std::size_t i = 0; i < lhsv.size(); ++i)
  {
    v2.at(0) = (i + 1) * weight_delete;
    for (std::size_t j = 0; j < rhsv.size(); j++)
    {
      v2.at(j + 1) = std::min(
        // deletion
        v1.at(j + 1) + weight_delete,
        std::min(
          // insertion
          v2.at(j) + weight_insert,
          // substitution
          v1.at(j) + (weight_substitute * (lhsv.at(i) != rhsv.at(j)))));

      if (i > 0 && j > 0 &&
        (lhsv.at(i - 1) == rhsv.at(j)) &&
        (lhsv.at(i) == rhsv.at(j - 1)))
      {
        v2.at(j + 1) = std::min(
          v0.at(j + 1),
          // transposition
          v0.at(j - 1) + weight_transpose);
      }
    }

    std::swap(v0, v1);
    std::swap(v1, v2);
  }

  return v1.at(rhsv.size());
}

} // namespace OB::String
