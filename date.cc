/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xsd++/date.h"

#include <regex> /* for std::regex, std::regex_match() */

using namespace xsd;

static const std::regex date_regex(date::pattern);

bool
date::match(const std::string& literal) noexcept {
  return std::regex_match(literal, date_regex);
}

bool
date::validate() const noexcept {
  return true; // TODO
}

bool
date::canonicalize() noexcept {
  return false; // TODO
}
