#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <regex>
#include <functional>

#include <Shlobj.h>

constexpr std::size_t operator"" _hash(const char* str, const std::size_t size)
{
	std::size_t val{ std::_FNV_offset_basis };
	for (std::size_t idx{}; idx < size; ++idx) {
		val ^= static_cast<std::size_t>(str[idx]);
		val *= std::_FNV_prime;
	}

	return val;
}