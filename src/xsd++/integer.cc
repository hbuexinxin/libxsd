/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "const.h"   /* for XSD_INTEGER_CAPTURES */
#include "integer.h"
#include "regex.h"   /* for std::regex, std::regex_match() */

#include <algorithm> /* for std::copy() */
#include <cassert>   /* for assert() */
#include <cerrno>    /* for errno */
#include <cinttypes> /* for std::strtoimax() */

using namespace std::regex_constants;
using namespace xsd;

////////////////////////////////////////////////////////////////////////////////

constexpr char integer::name[];

constexpr char integer::pattern[];

static const std::regex integer_regex{integer::pattern};

////////////////////////////////////////////////////////////////////////////////

static bool
parse_literal(const char* literal,
              bool& sign,
              std::string& integer) {
  std::cmatch matches;
  if (!std::regex_match(literal, matches, integer_regex, match_not_null)) {
    return false; /* invalid literal */
  }

  assert(matches.size() == XSD_INTEGER_CAPTURES);

  /* 3.3.13.2 'The preceding optional "+" sign is prohibited' */
  if (matches[1].length()) {
    switch (*matches[1].first) {
      case '-': sign = false; break;
      case '+': sign = true; break;
    }
  }

  /* 3.3.13.2 'Leading zeroes are prohibited' */
  integer.append(matches[2].first, matches[2].second);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

integer::value_type
integer::parse(const char* literal) {
  std::error_condition error;
  const auto value = parse(literal, error);

  if (error) {
    if (error == std::errc::invalid_argument) {
      throw std::invalid_argument{literal};
    }
    if (error == std::errc::result_out_of_range) {
      if (value == INTMAX_MIN) {
        throw std::underflow_error{literal};
      }
      else {
        throw std::overflow_error{literal};
      }
    }
  }

  return value;
}

integer::value_type
integer::parse(const char* literal,
               std::error_condition& error) noexcept {
  return parse(literal, INTMAX_MIN, INTMAX_MAX, error);
}

integer::value_type
integer::parse(const char* literal,
               const integer::value_type min_value,
               const integer::value_type max_value,
               std::error_condition& error) noexcept {
  bool sign{true};
  std::string integer;

  if (!parse_literal(literal, sign, integer)) {
   error = std::errc::invalid_argument;
   return {};
  }

  errno = 0;
  auto value = std::strtoimax(integer.c_str(), nullptr, 10);
  if (sign == false) {
    value = -value;
  }

  if (errno) {
    error.assign(errno, std::generic_category());
  }
  else if (value < min_value) {
    error = std::errc::result_out_of_range;
    value = INTMAX_MIN;
  }
  else if (value > max_value) {
    error = std::errc::result_out_of_range;
    value = INTMAX_MAX;
  }

  return value;
}

bool
integer::match(const char* literal) noexcept {
  return std::regex_match(literal, integer_regex, match_not_null);
}

bool
integer::validate() const noexcept {
  return integer::match(_literal);
}

bool
integer::canonicalize() noexcept {
  bool sign{true};
  std::string integer;

  if (!parse_literal(c_str(), sign, integer)) {
    throw std::invalid_argument{c_str()}; /* invalid literal */
  }

  char buffer[256] = "";
  char* output = buffer;

  /* 3.3.13.2 'The preceding optional "+" sign is prohibited' */
  if (sign == false) *output++ = '-';

  /* 3.3.13.2 'Leading zeroes are prohibited' */
  output = std::copy(integer.cbegin(), integer.cend(), output);

  *output++ = '\0';

  if (_literal.compare(buffer) != 0) {
    _literal.assign(buffer);
    return true; /* now in canonical form */
  }

  return false; /* already in canonical form */
}

integer::operator long long() const {
  std::error_condition error;
  const auto result = value(error);
  if (error) throw std::bad_cast{};
  return result;
}

integer::value_type
integer::value() const {
  return parse(c_str());
}

integer::value_type
integer::value(std::error_condition& error) const noexcept {
  return parse(c_str(), error);
}
