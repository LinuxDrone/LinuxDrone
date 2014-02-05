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

#include "CFile.h"
#include <assert.h>
#include <stdio.h>

CFile::CFile(const CString& fileName /*= CString()*/)
{
	m_file      = 0;
	m_openFlags = 0;
#ifdef _WIN32
	m_hFile     = INVALID_HANDLE_VALUE;
	m_hMap      = 0;
#endif // _WIN32
	setFileName( fileName );
}

CFile::~CFile()
{
	if (m_file) {
		close();
	}
}

// return the type of the device
CIODevice::DeviceType CFile::deviceType() const
{
	return DeviceTypeFile;
}

/////////////////////////////////////////////////////////////////////
//                         parameters of file                      //
/////////////////////////////////////////////////////////////////////

void CFile::setFileName(const CString& fileName)
{
	if (isOpen())
		return;
	m_fileName = fileName;
}

CString CFile::fileName()
{
	return m_fileName;
}

/////////////////////////////////////////////////////////////////////
//                           open/close                            //
/////////////////////////////////////////////////////////////////////

bool CFile::open(uint32_t mode)
{
	if( m_fileName.isEmpty() || isOpen() )
		return false;
    
	CString strMod;
	if( (mode & ReadOnly) && (mode & WriteOnly) )
		strMod += "a+";
    //		strMod += "w+";
	else if( mode & ReadOnly )
		strMod += "r";
	else if( mode & WriteOnly )
		strMod += "w";
	strMod += "b";
    
#ifndef _WIN32
	m_file = fopen(m_fileName.data(), strMod.data());
#else
#pragma message("need write conversion to utf16 in class of CString")
	//	// кусок для тупорылой винды - она не понимает человеческую кодировку UTF-8
	//	// а по сему придется юзать припезденные функции!!!!
	//	CByteArray utf16Name = m_fileName.toUtf16();
	//	CByteArray utf16Mod = strMod.toUtf16();
	//	m_file = _wfopen( (wchar_t*)utf16Name.getData(), (wchar_t*)utf16Mod.getData() );
#endif
	if (!m_file){
        //Logger() << __PRETTY_FUNCTION__ << "Error opening file " << m_fileName << ": " << strerror(errno);
		return false;
	}
	m_openFlags = mode;
	return true;
}

void CFile::close()
{
	if (!m_file)
		return;
	fclose(m_file);
	m_file = 0;
}

bool CFile::isOpen() const
{
	if (m_file)
		return true;
	return false;
}

bool CFile::remove()
{
	if (m_fileName.isEmpty())
		return false;			// не могу удалить неизвестный файл
#ifndef _WIN32
	if (!::remove(m_fileName.data()))
		return true;
#else
#pragma message("need write conversion to utf16 in class of CString")
	//	CByteArray utf16Name = fileName().toUtf16();
	//	if( !_wremove((wchar_t*)utf16Name.getData()) )
	//		return true;
#endif
	//	Logger() << "Error removing file " << m_fileName << ", errno = " << strerror(errno);
	return false;
}

bool CFile::rename(const CString& newName)
{
	if( m_fileName.isEmpty() || newName.isEmpty() )
		return false;
#ifndef _WIN32
	if (::rename(m_fileName.data(), newName.data()))
	{
		//		Logger() << "Error renaming file " << m_fileName << " to " << newName << ", errno = " << strerror(errno);
		return false;
	}
#else
#pragma message("need write conversion to utf16 in class of CString")
	//	if (_wrename((wchar_t*)m_fileName.toUtf16().getData(), (wchar_t*)newName.toUtf16().getData()) != 0)
	//	{
	////		Logger() << "Error renaming file " << m_fileName << " to " << newName << ", errno = " << strerror(errno);
	//		return false;
	//	}
#endif
	close();
	setFileName(newName);
    
	return true;
}

/////////////////////////////////////////////////////////////////////
//                                    io                           //
/////////////////////////////////////////////////////////////////////

uint64_t CFile::write(const void* data, uint64_t len)
{
	if( !m_file )
		return 0;
	uint64_t ret = fwrite(data, 1, len, m_file);
	return ret;
}

uint64_t CFile::write(const CByteArray& data)
{
    return write((void*)data.data(), data.size());
}

uint64_t CFile::read(void* data, uint64_t len) const
{
	if (!m_file) {
		return 0;
	}
	int ret = (int)fread(data, 1, len, m_file);
	return ret;
}

CByteArray CFile::readAll()
{
	int64_t pos, size;
	pos = this->pos();
	size = this->size();
	if( 0 == size )
		return CByteArray();
    
	if (!seek(0)) {
		return CByteArray();
	}
	CByteArray data;
	data.setSize( int(size) );
	read((char*)data.data(), data.size());
	seek (pos);
	return data;
}

void CFile::flush()
{
	if (!m_file) {
		return;
	}
	fflush(m_file);
}

/////////////////////////////////////////////////////////////////////
//                                                                 //
/////////////////////////////////////////////////////////////////////

bool CFile::seek(uint64_t pos)
{
	if (!m_file) {
		return false;
	}
#if defined(__APPLE__) || defined(__ANDROID__)
	if (!fseeko(m_file, pos, SEEK_SET) )
		return true;
#elif !defined(_WIN32)
	if (!fseeko64(m_file, pos, SEEK_SET))
		return true;
#else
	if (!_fseeki64(m_file, pos, SEEK_SET))
		return true;
#endif
	return false;
}

uint64_t CFile::pos() const
{
	if (!m_file)
		return (uint64_t)-1;
	uint64_t ret = 0;
#if defined(__APPLE__) || defined(__ANDROID__)
	ret = ftello(m_file);
#elif !defined(_WIN32)
	ret = ftello64(m_file);
#else
	ret = (uint64_t)_ftelli64(m_file);
#endif
	return ret;
}

uint64_t CFile::size() const
{
	if (!m_file) {
		return 0;
	}
	uint64_t currentPos = pos();
	fseek(m_file, 0, SEEK_END);
	uint64_t size = pos();
    
#if defined(__APPLE__) || defined(__ANDROID__)
    int ret = fseeko(m_file, currentPos, SEEK_SET);
	assert(!ret);
#elif !defined(_WIN32)
    /*ret =*/ fseeko64(m_file, currentPos, SEEK_SET);
#else
    /*ret =*/ _fseeki64(m_file, (__int64)currentPos, SEEK_SET);
#endif
	return size;
}
