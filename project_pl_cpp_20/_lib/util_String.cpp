#include "util_String.h"

//
string FormatA(const char *pFormat, ...)
{
	char szBuffer[eString::BUFFER_MAX + 1];
	memset(szBuffer, 0, sizeof(szBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnprintf_s(szBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	return std::string(szBuffer);
}

wstring FormatW(const WCHAR *pFormat, ...)
{
	WCHAR wcsBuffer[eString::BUFFER_MAX + 1];
	memset(wcsBuffer, 0, sizeof(wcsBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnwprintf_s(wcsBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	return std::wstring(wcsBuffer);
}

void TokenizeA(string &str, vector<string> &tokens, string delimiter)
{
	auto lastPos = str.find_first_not_of(delimiter, 0);
	auto pos = str.find_first_of(delimiter, lastPos);

	while( string::npos != pos || string::npos != lastPos )
	{
		tokens.push_back(str.substr(lastPos, pos-lastPos));

		lastPos = str.find_first_not_of(delimiter, pos);
		pos = str.find_first_of(delimiter, lastPos);
	}
}

void TokenizeW(wstring &str, vector<wstring> &tokens, wstring delimiter)
{
	auto lastPos = str.find_first_not_of(delimiter, 0);
	auto pos = str.find_first_of(delimiter, lastPos);

	while( wstring::npos != pos || wstring::npos != lastPos )
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		lastPos = str.find_first_not_of(delimiter, pos);
		pos = str.find_first_of(delimiter, lastPos);
	}
}
