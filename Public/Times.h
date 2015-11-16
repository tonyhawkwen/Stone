#ifndef _STONE_TIMES_H_
#define _STONE_TIMES_H_

#include<stdint.h>

namespace Stone{

class TickInternal;

class Tick
{
public:
    Tick();
    ~Tick();
    void Reset();//not thread safe
    int64_t Elapsed() const;
    int64_t ElapsedMicro() const;
    int64_t ElapsedSeconds() const;
    int64_t ElapsedMinutes() const;
    int64_t ElapsedHours() const;

private:
    TickInternal* m_data;

    Tick( const Tick& );
    const Tick& operator=( const Tick& );

};


}


#endif


