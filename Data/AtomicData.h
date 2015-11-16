#ifndef _STONE_ATOMICDATA_H_
#define _STONE_ATOMICDATA_H_

#include <type_traits>
#include <atomic>

namespace Stone{

template<class T, class Enable = void>
class AtomicData {
public:
    AtomicData() :
    m_Lock(ATOMIC_FLAG_INIT),
    m_Data(nullptr)
    {
    }

    explicit AtomicData(const T& data) :
    m_Lock(ATOMIC_FLAG_INIT),
    m_Data(new T(data))
    {
    }

    ~AtomicData()
    {
        delete m_Data;
    }

    void Set(const T& data, std::memory_order order = std::memory_order_seq_cst)
    {
        T* dest = new T(data);
        while (m_Lock.test_and_set(order));
        T* prev = m_Data;
        m_Data = dest;
        m_Lock.clear(std::memory_order_release);

        delete prev;
    }

    T Get(std::memory_order order = std::memory_order_seq_cst)
    {
        while (m_Lock.test_and_set(order));
        T data(*m_Data);
        m_Lock.clear(std::memory_order_release);
        return std::move(data);
    }

    bool TryExchange(T& data, std::memory_order order = std::memory_order_seq_cst)
    {
        T* dest = new T(data);
        bool ret = m_Lock.test_and_set(order);

        if(!ret)
        {
            T* prev = m_Data;
            m_Data = dest;
            m_Lock.clear(std::memory_order_release);
            data = *prev;

            delete prev;
        }

        return !ret;
    }

private:
    std::atomic_flag m_Lock;
    T* m_Data;
};


template<class T>
class AtomicData<T, typename std::enable_if<std::is_fundamental<T>::value>::type > {
public:
    AtomicData() :
    m_Data(0)
    {
    }

    explicit AtomicData(const T& data) :
    m_Data(data)
    {
    }

    ~AtomicData()
    {
    }

    void Set(const T& data, std::memory_order order = std::memory_order_seq_cst)
    {
        m_Data.store(data, order);
    }

    T Get(std::memory_order order = std::memory_order_seq_cst)
    {
        return m_Data.load(order);
    }

    void TryExchange(T& data, std::memory_order order = std::memory_order_seq_cst)
    {
        auto cur = m_Data.load(std::memory_order_relaxed);
        return m_Data.compare_exchange_weak(cur, data, order);
    }

private:
    std::atomic<T> m_Data;
};

}

#endif

