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


#pragma once

#include <memory>

class CStringAnsiPrivate;

class CStringAnsi
{
public:
    CStringAnsi();
    CStringAnsi(const CStringAnsi& other);
    CStringAnsi(const char *str, size_t len = -1, bool referenceOnly = false);
    ~CStringAnsi();

	bool isEmpty() const;
    const char* data() const;
    size_t length() const;
    CStringAnsi copy() const;

    CStringAnsi& operator=(const CStringAnsi& other);
    CStringAnsi& operator=(const char* str);

    bool operator==(const CStringAnsi& str) const;
    bool operator==(const char* str) const;
    bool operator!=(const CStringAnsi& str) const;
    bool operator!=(const char* str) const;
    bool operator<(const CStringAnsi& str) const;
    bool operator<(const char* str) const;

    CStringAnsi& operator+=(const CStringAnsi& str);
    CStringAnsi& operator+=(const char* str);

private:
    CStringAnsiPrivate * d;

    void makeNewDataIfNeed();
};

const CStringAnsi operator+(const CStringAnsi &s1, const CStringAnsi &s2);
const CStringAnsi operator+(const CStringAnsi &s1, const char* s2);
