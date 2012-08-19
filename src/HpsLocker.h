
// Copyright (C) Stanislav Cherkasov

#ifndef HPSLOCKER_H_CF9F15A6_64B3_4894_81A8_F617DED98182
#define HPSLOCKER_H_CF9F15A6_64B3_4894_81A8_F617DED98182

#include "HpsMutex.h"

namespace Hps
{
    class Locker
    {
    public:
        Locker(Mutex &mutex);
        ~Locker();

    private:
        Locker(Locker const&);
        Locker & operator = (Locker const&);

    private:
        Mutex & m_mutex;
    };
} // namespace Hpg

#endif // HPSLOCKER_H_CF9F15A6_64B3_4894_81A8_F617DED98182
