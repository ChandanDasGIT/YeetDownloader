#pragma once

#include <string>

// SHA-256 (FIPS 180-4) as lowercase hex.
std::string sha256Hex(const std::string &data);
