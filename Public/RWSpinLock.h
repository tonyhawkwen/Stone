/*
 * Copyright 2015 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//Generated from facebook folly
#ifndef _STONE_RWSPINLOCK_H_
#define _STONE_RWSPINLOCK_H_

#include <thread>
#include <atomic>
#include "Noncopyable.h"
#include "Macro.h"

namespace Stone {
class RWSpinLock : Noncopyable {
public:
    enum : int32_t {
        READER = 4,
        UPGRADED = 2,
        WRITER = 1
    };
    RWSpinLock() : bits_(0), r_(this), w_(this) {}
    // Lockable Concept
    void lock()
    {
        int count = 0;
        while (!LIKELY(try_lock()))
        {
            if (++count > 1000) std::this_thread::yield();
        }
    }

    // Writer is responsible for clearing up both the UPGRADED and WRITER bits.
    void unlock()
    {
        bits_.fetch_and(~(WRITER | UPGRADED), std::memory_order_release);
    }

    // SharedLockable Concept
    void lock_shared()
    {
        int count = 0;
        while (!LIKELY(try_lock_shared())) {
            if (++count > 1000) std::this_thread::yield();
        }
    }

    void unlock_shared()
    {
        bits_.fetch_add(-READER, std::memory_order_release);
    }

    // Downgrade the lock from writer status to reader status.
    void unlock_and_lock_shared()
    {
        bits_.fetch_add(READER, std::memory_order_acquire);
        unlock();
    }

    // UpgradeLockable Concept
    void lock_upgrade()
    {
        int count = 0;
        while (!try_lock_upgrade())
        {
          if (++count > 1000) std::this_thread::yield();
        }
    }

    void unlock_upgrade()
    {
        bits_.fetch_add(-UPGRADED, std::memory_order_acq_rel);
    }

    // unlock upgrade and try to acquire write lock
    void unlock_upgrade_and_lock()
    {
        int64_t count = 0;
        while (!try_unlock_upgrade_and_lock())
        {
            if (++count > 1000) std::this_thread::yield();
        }
    }

    // unlock upgrade and read lock atomically
    void unlock_upgrade_and_lock_shared()
    {
        bits_.fetch_add(READER - UPGRADED, std::memory_order_acq_rel);
    }

    // write unlock and upgrade lock atomically
    void unlock_and_lock_upgrade()
    {
        // need to do it in two steps here -- as the UPGRADED bit might be OR-ed at
        // the same time when other threads are trying do try_lock_upgrade().
        bits_.fetch_or(UPGRADED, std::memory_order_acquire);
        bits_.fetch_add(-WRITER, std::memory_order_release);
    }


    // Attempt to acquire writer permission. Return false if we didn't get it.
    bool try_lock()
    {
        int32_t expect = 0;
        return bits_.compare_exchange_strong(expect, WRITER,
            std::memory_order_acq_rel);
    }

    // Try to get reader permission on the lock. This can fail if we
    // find out someone is a writer or upgrader.
    // Setting the UPGRADED bit would allow a writer-to-be to indicate
    // its intention to write and block any new readers while waiting
    // for existing readers to finish and release their read locks. This
    // helps avoid starving writers (promoted from upgraders).
    bool try_lock_shared()
    {
        // fetch_add is considerably (100%) faster than compare_exchange,
        // so here we are optimizing for the common (lock success) case.
        int32_t value = bits_.fetch_add(READER, std::memory_order_acquire);
        if (UNLIKELY(value & (WRITER|UPGRADED)))
        {
            bits_.fetch_add(-READER, std::memory_order_release);
            return false;
        }
        return true;
    }

    // try to unlock upgrade and write lock atomically
    bool try_unlock_upgrade_and_lock()
    {
        int32_t expect = UPGRADED;
        return bits_.compare_exchange_strong(expect, WRITER,
            std::memory_order_acq_rel);
    }

    // try to acquire an upgradable lock.
    bool try_lock_upgrade()
    {
        int32_t value = bits_.fetch_or(UPGRADED, std::memory_order_acquire);

        // Note: when failed, we cannot flip the UPGRADED bit back,
        // as in this case there is either another upgrade lock or a write lock.
        // If it's a write lock, the bit will get cleared up when that lock's done
        // with unlock().
        return ((value & (UPGRADED | WRITER)) == 0);
    }

    // mainly for debugging purposes.
    int32_t bits() const { return bits_.load(std::memory_order_acquire); }

    class R {
    public:
        R(RWSpinLock* own) : owner_(own){}
        void lock()
        {
            owner_->lock_shared();
        }

        void unlock()
        {
            owner_->unlock_shared();
        }
    private:
        RWSpinLock* owner_;
    };

    class W {
    public:
        W(RWSpinLock* own) : owner_(own){}
        void lock()
        {
            owner_->lock_upgrade();
            owner_->unlock_upgrade_and_lock();
        }

        void unlock()
        {
            owner_->unlock();
        }
    private:
        RWSpinLock* owner_;
    };

    R read()
    {
        return r_;
    };
    W write()
    {
        return w_;
    }
private:
    R r_;
    W w_;
    std::atomic<int32_t> bits_;
};
}

#endif
