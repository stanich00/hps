
// Copyright (C) Stanislav Cherkasov

#ifndef HPSCOMMON_H_6A86893B_525F_4045_A476_4DCDA2E5CBA1
#define HPSCOMMON_H_6A86893B_525F_4045_A476_4DCDA2E5CBA1

#include "HpsLog.h"
#include "HpsStdOutLogWriter.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>

namespace Hps
{
    // default config dir
    char const* const defaultConfigDir = "~/.hps/";

    // default name of config file
    char const* const defaultConfigFileName = "hps.cfg";

    // max number of events
    int const nMaxEventCount = 64;

    // get log
    Hps::Log & GetLog();

    // trim left
    std::string & TrimLeft(std::string & s);

    // trim rigth
    std::string & TrimRight(std::string & s);

    // trim left and right
    std::string & Trim(std::string & s);

    // get current gmt time in RFC-822 time format
    // thread-safe
    std::string GetTimeGMT();

    // get file contents as string
    std::string GetFile(std::string const& fileName, bool & ok);

    // expand home directory ~/ => /home/username/
    std::string ExpandDir(std::string const&);

    // extract directory from full file name
    std::string GetDirFromFullFileName(std::string const& fullFileName);

} // namespace Hpg

#endif // HPSCOMMON_H_6A86893B_525F_4045_A476_4DCDA2E5CBA1
