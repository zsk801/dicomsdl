/* -----------------------------------------------------------------------
 *
 * $Id$
 *
 * Copyright 2010, Kim, Tae-Sung. All rights reserved.
 * See copyright.txt for details
 *
 * -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dicom.h"

void process_file(dicom::dicomdir *dr,
		std::string relpath, std::string fullpath)
{
	char *fileid, *p;

	// replace '/' -> '\\'
	fileid = (char *)malloc(relpath.size()+1);
	strcpy(fileid, relpath.c_str());
	for (p = fileid; *p; p++)
		if (*p == '/') *p = '\\';

	dr->add_dicomfile((char *)(fullpath.c_str()), fileid);

	free(fileid);
}

#ifdef _HAVE_WINDOWS_H
#include <windows.h>
#define PATHSEP	"\\"
void walkdir(dicom::dicomdir *dr,
			 std::string rootpath, std::string relpath="")
{
	HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;

    if (rootpath.length() > 0 && rootpath[rootpath.length()-1] != '\\')
    	rootpath += "\\";

	hFind = FindFirstFile(
			(rootpath+relpath+std::string("*")).c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("in walkdir(): "
				"cannot read directory '%s'",
				(rootpath+relpath).c_str());
		return;
	}

	do {
		if (strcmp(ffd.cFileName, ".") == 0) continue;
		if (strcmp(ffd.cFileName, "..") == 0) continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			walkdir(dr, rootpath, relpath+ffd.cFileName+"\\");
		else
			process_file(dr, relpath+ffd.cFileName,
						 rootpath+relpath+ffd.cFileName);
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}
#endif

#ifdef _HAVE_DIRENT_H
#include <dirent.h>
#define PATHSEP	"/"
void walkdir(dicom::dicomdir *dr,
		 std::string rootpath, std::string relpath="")
{
	DIR *dp;
	struct dirent *dirp;

    if (rootpath.length() > 0 && rootpath[rootpath.length()-1] != '/')
    	rootpath += "/";

	dp = opendir((rootpath+relpath).c_str());
	if (!dp) {
		printf("in walkdir(): "
				"cannot read directory '%s'",
				(rootpath+relpath+std::string("*")).c_str());
		return;
	}

	while (dirp = readdir(dp)) {
		if (strcmp(dirp->d_name, ".") == 0) continue;
		if (strcmp(dirp->d_name, "..") == 0) continue;

		if (dirp->d_type & DT_DIR)
			walkdir(dr, rootpath, relpath+dirp->d_name+"/");
		else if (dirp->d_type & DT_REG)
			process_file(dr, relpath+dirp->d_name,
						 rootpath+relpath+dirp->d_name);
	}
	closedir(dp);
}

#endif

// -----------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (argc < 3) {
		puts("makedicomdir basedirpath dicomdir_name");
		return 0;
	}

	dicom::dicomdir dr;
	walkdir(&dr, std::string(argv[1]));
	dr.save_to_file(argv[2]);

	return 0;
}
