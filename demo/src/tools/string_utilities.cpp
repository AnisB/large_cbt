#include "tools/string_utilities.h"

std::wstring convert_to_wide(const std::string& str)
{
	size_t stringSize = str.size();
	std::wstring wc(stringSize, L'#');
	mbstowcs(&wc[0], str.c_str(), stringSize);
	return wc;
}

std::wstring convert_to_wide(const char* str, uint32_t strLength)
{
	size_t stringSize = strLength;
	std::wstring wc(stringSize, L'#');
	mbstowcs(&wc[0], str, stringSize);
	return wc;
}

std::string convert_to_regular(const std::wstring& wstr)
{
	size_t stringSize = wstr.size();
	std::string str(stringSize, '#');
	wcstombs(&str[0], wstr.c_str(), stringSize);
	return str;
}

void split(const std::string& parString, char parSeparator, std::vector<std::string>& _out)
{
	std::stringstream streamObj(parString);
	std::string item;
	while (std::getline(streamObj, item, parSeparator))
	{
		_out.push_back(item);
	}
}
