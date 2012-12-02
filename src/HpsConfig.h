
// Copyright (C) Stanislav Cherkasov

#ifndef HPSCONFIG_H_F590EE0F_29C2_417D_9EA1_4978DCD9BB75
#define HPSCONFIG_H_F590EE0F_29C2_417D_9EA1_4978DCD9BB75

#include "HpsMutex.h"
#include <string>
#include <map>
#include <memory>

namespace Hps
{
    // class HpsConfig
    // read and parse configuration file
    // thread-safe
    class Config
    {
    public:
        // c-tor
        Config(std::string const& configFile);

        // get parameter by name
        std::string GetStrParam(std::string const& name);
        int GetIntParam(std::string const& name);
        double GetDblParam(std::string const& name);

    private:
        void ReadConfig();
        void CreateDefaultConfig()const;
        bool ParseLine(std::string const& line);

    private:
        typedef std::map<std::string, std::string> ParamList;

        // name of configuration file
        std::string m_configFileName;

        // list of parameters
        ParamList m_params;

        Mutex m_mutex;
    };

    typedef std::shared_ptr<Config> ConfigPtr;

} // namespace Hps

#endif // HPSCONFIG_H_F590EE0F_29C2_417D_9EA1_4978DCD9BB75
