
// Copyright (C) Stanislav Cherkasov

#include "HpsCommon.h"
#include "HpsLocker.h"
#include "HpsMutex.h"
#include <fstream>
#include <iterator>
#include <cassert>
#include <time.h>
#include <wordexp.h>

namespace
{
    Hps::Mutex mutex;
}

namespace Hps
{
    std::string & TrimLeft(std::string & s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    std::string & TrimRight(std::string & s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    std::string & Trim(std::string & s)
    {
        return TrimLeft(TrimRight(s));
    }

    std::string GetTimeGMT()
    {
        Locker lock(mutex);

        time_t t = time(0);
        struct tm * gmt = gmtime(&t);

        size_t const bufSize = 64;
        char buf[bufSize];
        strftime(buf, bufSize, "%a, %d %b %Y %H:%M:%S GMT", gmt);

        return buf;
    }

    std::string GetFile(std::string const& fileName, bool & ok)
    {
        ok = false;

        std::ifstream file(fileName);
        if(file.is_open())
        {
            std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            ok = true;
            return s;
        }

        return "";
    }

    std::string ExpandDir(std::string const& dir)
    {
        std::string result;

        wordexp_t expResult;
        wordexp(dir.c_str(), &expResult, 0);

        if(expResult.we_wordc > 0)
        {
            result = expResult.we_wordv[0];
        }
        else
        {
            GetLog().Msg(Log::Error, "Could not expand directory");
        }

        wordfree(&expResult);

        return result;
    }

    std::string GetDirFromFullFileName(std::string const& fullFileName)
    {
        size_t p = fullFileName.rfind('/');
        if(p == std::string::npos)
            return "";
        return fullFileName.substr(0, p);
    }

} // namespace Hps

