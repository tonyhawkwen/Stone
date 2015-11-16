#include <chrono>
#include <string>
#include "Times.h"
#include "Macro.h"

namespace Stone{

std::string GetCurrentTime(void)
{
    auto current = std::chrono::system_clock::now();
    auto tt =std::chrono::system_clock::to_time_t(current);
    int microseconds = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(current.time_since_epoch()).count() % 1000000);

    char buff1[20] = {0,};
    char buff2[8] = {0,};
    std::strftime (buff1,20,"%Y%m%d %H:%M:%S.",std::localtime(&tt)); //std::gmtime
    std::snprintf(buff2, sizeof(buff2), "%06d", microseconds);
    std::string strCur = buff1;
    strCur.append(buff2);

    return std::move(strCur);
}


class TickInternal
{
public:
    std::chrono::steady_clock::time_point m_Begin;
};


Tick::Tick() : m_data(new TickInternal)
{
    m_data->m_Begin = std::chrono::steady_clock::now();
}

Tick::~Tick()
{
    delete m_data;
}

void Tick::Reset()
{
    m_data->m_Begin = std::chrono::steady_clock::now();
}

int64_t Tick::Elapsed() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_data->m_Begin).count();
}

int64_t Tick::ElapsedMicro() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_data->m_Begin).count();
}

int64_t Tick::ElapsedSeconds() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_data->m_Begin).count();
}

int64_t Tick::ElapsedMinutes() const
{
    return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - m_data->m_Begin).count();
}

int64_t Tick::ElapsedHours() const
{
    return std::chrono::duration_cast<std::chrono::hours>(std::chrono::steady_clock::now() - m_data->m_Begin).count();
}


}

