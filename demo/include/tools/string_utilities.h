#pragma once

// System includes
#include <string>
#include <vector>
#include <sstream>

// String manipulation
std::wstring convert_to_wide(const std::string& str);
std::wstring convert_to_wide(const char* str, uint32_t strLength);
std::string convert_to_regular(const std::wstring& str);
void split(const std::string& parString, char parSeparator, std::vector<std::string>& _out);

template <typename T>
T convert_from_string(const std::string& _string)
{
	std::stringstream stream(_string);
	T val;
	stream >> val;
	return val;
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return std::move(out).str();
}
