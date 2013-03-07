/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xsd++/datetime.h"

#include "xsd++/regex.h" /* for std::regex, std::regex_match() */

using namespace xsd;

constexpr char datetime::name[];

constexpr char datetime::pattern[];

static const std::regex datetime_regex(datetime::pattern);

bool
datetime::match(const std::string& literal) noexcept {
  return std::regex_match(literal, datetime_regex);
}

bool
datetime::validate() const noexcept {
  return datetime::match(_literal);
}

bool
datetime::canonicalize() noexcept {
  return false; // TODO
}
