
// Copyright (C) Stanislav Cherkasov

#ifndef HPSMUTEX_H_2558CD30_3C41_4654_ABE6_D8AB8411CBB7
#define HPSMUTEX_H_2558CD30_3C41_4654_ABE6_D8AB8411CBB7

#include <pthread.h>

namespace Hps
{
    class Mutex
    {
    public:
        // c-tor/d-tor
        Mutex(); // throw(std::runtime_error)
        ~Mutex();

        void Lock();
        void UnLock();

    private:
        // prevent from copying
        Mutex(Mutex const&);
        Mutex & operator = (Mutex const&);

    private:
        pthread_mutex_t m_mutex;
    };
} // namespace Hpg

#endif // HPSMUTEX_H_2558CD30_3C41_4654_ABE6_D8AB8411CBB7
