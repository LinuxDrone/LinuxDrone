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

#include "CDir.h"
#ifndef _WIN32
#include "sys/types.h"
#include "sys/stat.h"
#include "dirent.h"
#include <fnmatch.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#endif
#include "../system/Logger.h"

CDir::CDir(const CString& path /*= CString()*/, const CString& mask /*= CString("*")*/)
{
	m_mask          = mask;
	m_existFileList = false;

	setPath(path);
}

CDir::~CDir()
{
}

/////////////////////////////////////////////////////////////////////
//                              path                               //
/////////////////////////////////////////////////////////////////////

// path to folder
void CDir::setPath(const CString& path)
{
	m_path = path;
	clearFileList();
}

CString CDir::path() const
{
	return m_path;
}

// mask for search files and folders
void CDir::setMask(const CString& mask)
{
	m_mask = mask;
}

CString CDir::mask() const
{
	return m_mask;
}

CString CDir::absolutePath() const
{
	return m_path;
}

bool CDir::exists()
{
	if( m_path.isEmpty() )
		return false;
#if defined __linux__ || defined __ANDROID__ || defined __APPLE__ || defined IOS_PROJECT
	struct stat st;
	if (::stat(m_path.data(), &st) != -1) {
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			return true;
		}
	}
#elif defined _WIN32
	struct _stat st;
	if( _wstat( (wchar_t*)m_path.toUtf16().getData(), &st ) == 0 )
		if ((st.st_mode & S_IFMT) == S_IFDIR)
			return true;
#else
	#error unknown platform
#endif
	return false;
}

/////////////////////////////////////////////////////////////////////
//                        files in folder                          //
/////////////////////////////////////////////////////////////////////

int CDir::count() const
{
	if (!m_existFileList) {
		makeFileList();
	}
	return int (m_files.size());
}

void CDir::refresh()
{
	m_existFileList = false;
	makeFileList();
}

void CDir::clearFileList()
{
	m_files.clear();
	m_existFileList = false;
}

CString CDir::operator[](int pos) const
{
	if (!m_existFileList) {
		makeFileList();
	}
	if (0 > pos || pos >= int(m_files.size())) {
		return CString();
	}
	return m_files[pos].path;
}

bool CDir::isFolder(int pos) const
{
	if (!m_existFileList) {
		makeFileList();
	}
	if (0 > pos || pos >= int(m_files.size())) {
		return false;
	}
	return m_files[pos].isDir;
}

//===================================================================
//  p r i v a t e   f u n c t i o n s
//===================================================================

void CDir::makeFileList() const
{
	m_files.clear();
	m_existFileList = true;
	if (m_path.isEmpty())
		return;
	DIR* dir = opendir(m_path.data());
	if (dir)
	{
		struct dirent* entry = 0;
		while( (entry = readdir(dir)) != 0)
		{
			if (fnmatch(m_mask.data(), entry->d_name, FNM_PATHNAME | FNM_NOESCAPE | FNM_CASEFOLD) == 0 /*match*/) {
				CString name = CString(entry->d_name);
				if (name == "." || name == "..") {
					continue;
				}
				DIR_ENTRY dirEntry;
				if ((entry->d_type & DT_DIR)) {
					dirEntry.isDir = true;
				}
				dirEntry.path = entry->d_name;
				m_files.push_back(dirEntry);
			}
		}
		closedir( dir );
	}
}
