
// Copyright (C) Stanislav Cherkasov

#include "HpsConfig.h"
#include "HpsLocker.h"
#include "HpsCommon.h"
#include <sys/stat.h>
#include <wordexp.h>
#include <fstream>
#include <sstream>
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

    std::string Config::GetStrParam(std::string const& name)
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

    int Config::GetIntParam(std::string const& name)
    {
        std::string const s = GetStrParam(name);
        if(s.empty())
            return 0;
        int r = 0;
        std::stringstream ss(s);
        ss >> r;
        return r;
    }

    double Config::GetDblParam(std::string const& name)
    {
        std::string const s = GetStrParam(name);
        if(s.empty())
            return 0;
        double r = 0;
        std::stringstream ss(s);
        ss >> r;
        return r;
    }

    void Config::ReadConfig()
    {
         std::ifstream file(m_configFileName.c_str());
        if(!file.is_open())
        {
            CreateDefaultConfig();

            // try to open config file again
            file.open(m_configFileName.c_str());
        }

        if(file.is_open())
        {
            for(std::string line; getline(file, line);)
            {
                if(!ParseLine(line))
                    GetLog().Msg(Log::Error, "Could not parse line: %s", line.c_str());
            }

        }
        else
        {
            GetLog().Msg(Log::Error,"Could not open config file: %s", m_configFileName.c_str());
        }

        // set log level from config
        ParamList::const_iterator it = m_params.find("LOG_LEVEL");
        if(it != m_params.end())
            GetLog().SetLevel(it->second);
    }

    void Config::CreateDefaultConfig()const
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
            defaultFile << "SERVER_PORT     = 10000" << std::endl;
            defaultFile << "LOG_LEVEL       = error" << std::endl;
            defaultFile << "MAX_EVENT_COUNT = 64"    << std::endl;

            defaultFile.close();
        }
        else
        {
            GetLog().Msg(Log::Error, "Could not create default config file");
        }
    }

    bool Config::ParseLine(std::string const& line)
    {
        size_t p1 = line.find('=');
        if(p1 != std::string::npos)
        {
            std::string name = line.substr(0, p1);
            std::string value = line.substr(p1 + 1);
            m_params.insert(std::make_pair(Trim(name), Trim(value)));
            return true;
        }
        return false;
    }

} // namespace Hps
