/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 1997-2002 The LiteStep Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef __SETTINGSFILEPARSER_H
#define __SETTINGSFILEPARSER_H

#include "../utility/common.h"
#include "settingsdefines.h"

class FileParser //: boost::noncopyable
{
public:
	FileParser(SettingsMap* pSettingsMap);
	~FileParser();

	void ParseFile(LPCWSTR pwzFileName);

private:
	SettingsMap* m_pSettingsMap;
	FILE* m_phFile;

	unsigned int m_uLineNumber;
	wchar_t m_wzFullPath[MAX_PATH_LENGTH] {0};

	bool _ReadLineFromFile(LPWSTR pwzName, LPWSTR pwzValue);
	void _StripString(LPWSTR pwzString);
	void _ProcessLine(LPCWSTR pwzName, LPCWSTR pwzValue);
	void _ProcessIf(LPCWSTR pwzExpression);
	void _SkipIf();

};

#endif // __SETTINGSFILEPARSER_H
