#ifndef UTIL_H_CED7A66D
#define UTIL_H_CED7A66D

#include <algorithm>
#include <string>
#include <iterator>
#include <cctype>
#include <sstream>

namespace cadence {

  inline std::string uppercase(std::string in)
  {
    std::string result;
    std::transform(std::begin(in), std::end(in), std::back_inserter(result), [] (char ch)
    {
      return std::toupper(ch);
    });

    return result;
  }

  template <class InputIterator>
  std::string implode(InputIterator first, InputIterator last, std::string delimiter)
  {
    std::stringstream result;

    for (InputIterator it = first; it != last; it++)
    {
      if (it != first)
      {
        result << delimiter;
      }

      result << *it;
    }

    return result.str();
  }

  template <class OutputIterator>
  void split(std::string input, std::string delimiter, OutputIterator out)
  {
    while (!input.empty())
    {
      int divider = input.find(delimiter);
      if (divider == std::string::npos)
      {
        *out = input;
        out++;

        input = "";
      } else {
        *out = input.substr(0, divider);
        out++;

        input = input.substr(divider+delimiter.length());
      }
    }
  }

  template <class Container>
  Container split(std::string input, std::string delimiter)
  {
    Container result;

    split(input, delimiter, std::back_inserter(result));

    return result;
  }

};

#endif /* end of include guard: UTIL_H_CED7A66D */
