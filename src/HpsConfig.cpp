
// Copyright (C) Stanislav Cherkasov

#include "HpsConfig.h"
#include "HpsLocker.h"
#include "HpsCommon.h"
#include <sys/stat.h>
#include <wordexp.h>
#include <fstream>
#include <cassert>

namespace Hps
{
    Config::Config(std::string const& configFile)
        : m_configFileName(ExpandDir(configFile))
    {
        if(m_configFileName.empty())
        {
            m_configFileName = ExpandDir(defaultConfigDir) + defaultConfigFileName;
        }
    }

    std::string Config::GetParam(std::string const& name)
    {
        Locker lock(m_mutex);

        // check if parameters were read
        if(m_params.empty())
            ReadConfig();

        // find and return parameter
        ParamList::const_iterator it = m_params.find(name);
        if(it != m_params.end())
            return it->second;

        // parameter not found
        GetLog().Msg(Log::Error, "Coulnd not get parameter: %s", name.c_str());
        return "";
    }

    void Config::ReadConfig()
    {
        std::ifstream file(m_configFileName.c_str());
        if(!file.is_open())
        {
            // create config directory
            std::string const dir = GetDirFromFullFileName(m_configFileName);
            struct stat st;
            if(-1 == stat(dir.c_str(), &st))
            {
                if(mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
                {
                    GetLog().Msg(Log::Error, "Could not create directory for config");
                }
            }

            // create default config file
            std::ofstream defaultFile(m_configFileName.c_str());
            if(defaultFile.is_open())
            {
                defaultFile << "SERVER_PORT     = \"10000\"" << std::endl;
                defaultFile.close();

                // try to open config file again
                file.open(m_configFileName.c_str());
            }
            else
            {
                GetLog().Msg(Log::Error, "Could not create default config file");
            }
        }

        if(file.is_open())
        {
            for(std::string line; getline(file, line);)
            {
                // parse line
                size_t p1 = line.find('=');
                if(p1 != std::string::npos)
                {
                    size_t p2 = line.find('"', p1);
                    if(p2 != std::string::npos)
                    {
                        size_t p3 = line.find('"', p2 + 1);
                        if(p3 != std::string::npos)
                        {
                            // do one more check
                            if(p1 < p2 && p2 < p3)
                            {
                                std::string name = line.substr(0, p1);
                                std::string value = line.substr(p2 + 1, p3 - p2 - 1);
                                m_params.insert(std::make_pair(Trim(name), Trim(value)));

                                continue; // success parsing, go to the next line
                            }
                        }
                    }
                }
                // fail to parse current line
                GetLog().Msg(Log::Error, "Could not parse line: %s", line.c_str());
            }

        }
        else
        {
            GetLog().Msg(Log::Error,"Could not open config file");
        }
    }

} // namespace Hps
