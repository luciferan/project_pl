#include "util_String.h"
#include <memory>

//
string FormatA(const char *pFormat, ...)
{
	char* pszBuffer = new char[eString::BUFFER_MAX + 1];
	memset(pszBuffer, 0, sizeof(pszBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnprintf_s(pszBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	std::string str(pszBuffer);
	delete[] pszBuffer;
	return str;
}

wstring FormatW(const WCHAR *pFormat, ...)
{
	WCHAR* pwcsBuffer = new WCHAR[eString::BUFFER_MAX + 1];
	memset(pwcsBuffer, 0, sizeof(pwcsBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnwprintf_s(pwcsBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	std::wstring wstr(pwcsBuffer);
	delete[] pwcsBuffer;
	return wstr;
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
