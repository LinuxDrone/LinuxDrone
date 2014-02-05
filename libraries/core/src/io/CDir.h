//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#ifndef __CDIR_H__
#define __CDIR_H__

#include "../text/CString.h"
#include <vector>

class CDir
{
public:
	CDir(const CString& path = CString(), const CString& mask = CString());
	~CDir();

// path
	// path to folder
	void setPath(const CString& path);
	CString path() const;
	// mask for search files and folders
	void setMask(const CString& mask);
	CString mask() const;

	CString absolutePath() const;
	bool exists();

// files in folder
	int count() const;
	void refresh();
	void clearFileList();
	CString operator[](int pos) const;

private:
	CString              m_path;
	CString              m_mask;
	mutable std::vector<CString> m_files;
	mutable bool                 m_existFileList;

	void makeFileList() const;
};

#endif // __CDIR_H__
