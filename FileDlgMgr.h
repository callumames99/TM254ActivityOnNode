#pragma once

#include "framework.h"
#include <commdlg.h>

class FileDlgMgr
{
public:
    FileDlgMgr()
        : s_{}
        , init_{false}
    {}

    void init(HWND w) noexcept
    {
        static const wchar_t filters[] = L"Task Float Calculator File\0*.PFT\0\0\0";

        if (init_)
        {
            s_.hwndOwner = w;
        }
        else
        {
            s_ = OPENFILENAMEW
            {
                sizeof(OPENFILENAMEW),
                w,
                NULL, // hInstance (for styling purposes)
                filters,
                NULL, // Custom filters
                0, // Number of characters in custom filters string buffer
                0, // Currently selected filter
                NULL, // Output buffer pointer (returned, fill member on call, else stack corruption)
                1024, // Output buffer size, in characters
                NULL, // 'Selected file title' (returned, path omitted)
                0, // Size of file title in characters
                NULL, // Initial directory
                L"Open Task Float Project", // Caption
                OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, // Flags
                0, // Filename/title path offset (when returned) in characters
                0, // Filename/title extension path offset (when returned) in characters
                L".FLT", // Default extension
                NULL, // Application defined (custom) data
                NULL, // Hook procedure
                NULL, // Template name
#ifdef _MAC
                NULL, NULL,
#endif
                NULL, // Reserved
                0, // Reserved
                0, // Extra flags
            };

            init_ = true;
        }
    }

    std::wstring Open() noexcept
    {
        wchar_t filename[1024] {};

        if (!init_) return std::wstring();

        s_.lpstrFile = filename;

        if (GetOpenFileNameW(&s_))
        {
            suffix(filename, 1024);
            return std::wstring(filename);
        }
        else
            return std::wstring();
    }

    std::wstring Save() noexcept
    {
        wchar_t filename[1024] {};

        if (!init_) return std::wstring();

        s_.lpstrFile = filename;

        if (GetSaveFileNameW(&s_))
        {
            suffix(filename, 1024);
            return std::wstring(filename);
        }
        else
            return std::wstring();
    }

    bool suffix(wchar_t *str, size_t sz) noexcept
    {
        wchar_t const *const end = str+sz;
        wchar_t *ext = nullptr, *cur = str;

        /* Find end */
        while (cur < end && *cur != L'\0')
        {
            /* Record last period */
            if (*cur == L'.')
                ext = cur;

            /* Next character */
            cur = cur+1;
        }

        /* Add extension */
        if (!ext) ext = cur;
        if (ext >= end - 5) return false;
        *ext++ = L'.';
        *ext++ = L'P';
        *ext++ = L'F';
        *ext++ = L'T';
        *ext++ = L'\0';

        return true;
    }

protected:
    OPENFILENAMEW s_;
    bool init_;
};
