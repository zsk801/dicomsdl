/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <list>
#include "utilfunc.h"

namespace dicom { //------------------------------------------------------

#ifndef _HAVE_WINDOWS_H

#include <glob.h>
#define PATHSEP '/'

stringlist find_files(const char *pattern)
{
	stringlist fnlist;

	glob_t globbuf;
	globbuf.gl_offs = 0;
	glob(pattern, GLOB_DOOFFS, NULL, &globbuf);

	for (int i = 0; i < globbuf.gl_pathc; i++) {
		fnlist.push_back(std::string(globbuf.gl_pathv[i]));
	}

	globfree(&globbuf);

	return fnlist;
}

#endif

#ifdef _HAVE_WINDOWS_H

// An implementation of POSIX glob() for win32

#include <windows.h>
#define PATHSEP '\\'

void process_pattern(
		std::string base, std::string pat, stringlist &fnlist);

void listdir_with_pattern(
		std::string base, std::string pat, stringlist &fnlist,
		std::string subpat="")
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR szDir[MAX_PATH];

	std::string s = base+pat;
	strncpy(szDir, s.c_str(), MAX_PATH);
	szDir[MAX_PATH-1] = '\0';

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE != hFind) {
		do {
			std::string fname = ffd.cFileName;
			if (fname == "." || fname == "..")
				continue;
			if (subpat == "") {
				std::string fn = base+fname;
				if (fn.substr(0,2) == std::string(".")+PATHSEP)
					fn = fn.substr(2); // remove unnecessary heading '.\'
				fnlist.push_back(fn);
			} else {
				process_pattern(base+fname+PATHSEP, subpat, fnlist);
			}
		}
		while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
}

void process_pattern(
		std::string base, std::string pat, stringlist &fnlist)
{
	while (pat.size() && pat[0] == PATHSEP)
		pat = pat.substr(1);

	if (pat == "")
		pat = "*";

	if (pat[0] == '.') {
		if (pat.substr(0,2) == "..") {
			if (pat.size() == 2)
				process_pattern(base+".."+PATHSEP, "", fnlist);
			else if (pat[2] == PATHSEP)	// ..\ or ../
				process_pattern(base+".."+PATHSEP, pat.substr(3), fnlist);
			else
				goto PROCESS_PATTERN;
		} else {
			if (pat.size() == 1)
				process_pattern(base, "", fnlist);
			else if (pat[1] == PATHSEP)	// .\ or ./
				process_pattern(base, pat.substr(2), fnlist);
			else
				goto PROCESS_PATTERN;
		}
	} else {

	PROCESS_PATTERN:

		int pos = pat.find(PATHSEP);
		if (pos < 0) {
			listdir_with_pattern(base, pat, fnlist);
		} else {
			std::string subpat;
			subpat = pat.substr(pos);
			pat = pat.substr(0,pos);
			listdir_with_pattern(base, pat, fnlist, subpat);
		}
	}
}

stringlist find_files(const char *pattern)
{
	std::string pat(pattern);
	stringlist fnlist;

	if (pat.size() >= 2 && pat.substr(0,2) == "\\\\") {
		// path contains network name '\\computername\share...'
		int pos = pat.find(PATHSEP, 3);
		if (pos < 0) // error pattern
			return fnlist;
		pos = pat.find(PATHSEP, pos+1);
		if (pos < 0)
			process_pattern(pat+PATHSEP, "", fnlist);
		else
			process_pattern(pat.substr(0,pos+1),
							pat.substr(pos+1), fnlist);
	}
	else if (pat.size() >= 2 && pat[1] == ':') {
		// file name starts with driver name 'x:' ...
		if (pat.size() == 2)
			process_pattern(pat, "", fnlist);
		else if (pat[2] == PATHSEP)
			process_pattern(pat.substr(0,3),
							pat.substr(3), fnlist);
		else
			process_pattern(pat.substr(0,2)+"."+PATHSEP,
							pat.substr(2), fnlist);
	}
	else if (pat.size() >= 1 && pat[0] == PATHSEP) {
		// file name starts with '\'
		process_pattern(std::string("")+PATHSEP, pattern+1, fnlist);
	}
	else {
		// file name does not with '\'
		process_pattern(std::string(".")+PATHSEP, pat, fnlist);
	}

	return fnlist;
}

#endif

} // namespace dicom -----------------------------------------------------
