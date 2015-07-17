/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __TSTRING_H__
#define __TSTRING_H__

#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include "Logger.h"
#include <vec234.h>


//log error if cap is hit... drop the string too
#define APPEND_CAP 1024

//string MUST have null terminator
template<typename T>
class TString
{    
private:
    
    T      *data;  //contiguous array of type T
    size_t  count; //like its ever going to get that high.
    bool    char8; //true if 8 bit char  -- wchar_t on linux is 32 bit

    //constructor calls this
    void init()
    {
        data  = 0;
        count = 0;
        char8 = true;
        if (sizeof(T) > 1)
            char8 = false;
    }

    //returns str length not including null terminator
    size_t getLen(const T *str)
    {
        if (str == 0)
            return 0;
        
        size_t c = 0;
        while(str[c] != 0)
        {
            c++;
        }
        return c;
    }

    //sequential write, does not write over end.
    void write(T *str, size_t start, size_t end)
    {
        if (str == 0)
        {
            LogError("TString::write trying to write to a null pointer");
            return;
        }
        //no bounds check(private function), life on the edge. 
        size_t c = 0;
        //TODO make sure it works well before you get fancy
        //memcpy(&data[start], str, );
        for (size_t i = start; i < end; i++)
        {
            data[i] = str[c++];
        }
            
    }

public:
    
    TString()                               { init();                           }
    TString(const T *str)                   { init(); fill(str);                }
    TString(T *str)                         { init(); fill(str);                }
    ~TString()                              { if (data) delete[] data; data = 0;}
    TString(const TString<T> &other)        { init(); fill(other.c_str());      }
    TString operator=(const TString<T> &rhs){ fill(rhs.c_str()); return *this;  }

    //const pointer to the string data
    const T *c_str()  const { return data;  }
    size_t   length() const { return count; }
    size_t   size()   const { return count; }

    //copy n bytes from buffer. 0 count == all
    inline void fill(const TString<T> &str, unsigned int _count = 0) { fill(str.c_str(), _count); }
    //copy n bytes from buffer. 0 count == all
    void fill(const T *str, unsigned int _count = 0)
    {
        if (data) delete[] data;
        data = 0;
        size_t max = getLen(str);
        if (_count == 0 || _count > max)
            count = max;
        else
            count = _count;
        if (count > 0)
        {
            data = new T[count+1]; //include null terminator
            for (size_t i = 0; i < count; i++)
                data[i] = str[i];
            data[count] = 0;      //null terminator
            //memcpy(data, str, count+1 * sizeof(T));
        }
    }

    //add to the end of string
    inline void append(const TString<T> &str) { append(str.c_str()); }
    inline void append(const T *str) { append(const_cast<T*>(str)); }
    void append(T *str)
    {
        if (str == 0)
            return;
        if (count > 0)
        {
            size_t len = getLen(str);
            //capping the size incase any strings somehow lose a null terminator somehow
            if (len+count >= APPEND_CAP)
            {
                LogError("TString::append() - no null terminator found. see APPEND_CAP in TString.cpp");
                return;
            }
            T *old = data;
            data = new T[len + count + 1];
            write(old, 0, count); //refill old data
            write(str, count, count+len+1); //append new + terminate
            count = count + len;
        }
        else
            fill(str);
    }

    //reset string to initial state
    void clear()
    {
        if (data) delete[] data;
        data  = 0;
        count = 0;
    }

    //trying to limit the amount of wasteful copies, so just append away.    

    void append(double d)
    {
        if (char8)
            dappend_8(d);
        else
            dappend_w(d);
    }

    void append(float f)
    {
        if (char8)
            fappend_8(f);
        else
            fappend_w(f);
    }

    void append(int i)
    {
        if (char8)
            iappend_8(i);
        else
            iappend_w(i);
    }

    void append(unsigned int n)
    {
        if (char8)
            iappend_8(n);
        else
            iappend_w(n);
    }

    void appendHex(uintptr_t addr)
    {
        if (char8)
            happend_8(addr);
        else
            happend_w(addr);
    }

private:

    //different appends for 8 bit, and non 8bit strings
    //NOTE swprintf kind of relies on wchar_t
    //so i have no idea what would happen it type != wchar_t
    
     // *** hexadecemal ***
    void happend_w(uintptr_t addr)
    {
        T str[32];
        swprintf((wchar_t *)str, sizeof(str) / sizeof(*str), L"%a", addr);
        append(str);
    }
    void happend_8(uintptr_t addr)
    {
        T str[32];
        sprintf((char *)str, "%a", addr);
        append(str);
    }

    // *** unsigned int ***
    void iappend_w(unsigned int n)
    {
        T str[32];
        swprintf((wchar_t *)str, sizeof(str) / sizeof(*str), L"%d", n);
        append(str);
    }
    void iappend_8(unsigned int n)
    {
        T str[32];
        sprintf((char *)str, "%d", n);
        append(str);
    }

    // *** signed int ***
    void iappend_w(int n)
    {
        T str[32];
        swprintf((wchar_t *)str, sizeof(str) / sizeof(*str), L"%d", n);
        append(str);
    }
    void iappend_8(int n)
    {
        T str[32];
        sprintf((char *)str, "%d", n);
        append(str);
    }

    // *** floats ***
    void fappend_w(float f)
    {
        T str[32];
        swprintf((wchar_t *)str, sizeof(str) / sizeof(*str), L"%f", f);
        append(str);
    }
    void fappend_8(float f)
    {
        T str[32];
        sprintf((char *)str, "%f", f);
        append(str);
    }

     // *** doubles ***
    void dappend_w(double d)
    {
        T str[32];
        swprintf((wchar_t *)str, sizeof(str) / sizeof(*str), L"%f", d);
        append(str);
    }
    void dappend_8(double d)
    {
        T str[32];
        sprintf((char *)str, "%f", d);
        append(str);
    }
    
};

#endif
