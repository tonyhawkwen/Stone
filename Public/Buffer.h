#ifndef _STONE_BUFFER_H_
#define _STONE_BUFFER_H_

#include <iostream>
#include <assert.h>
#include <sys/uio.h>

namespace Stone {

class Buffer {
public:
    //conflict with cut(), make sure that buffer is not modified when using iterator
    class const_iterator {
    public:
        explicit const_iterator() : Pointer_(nullptr), Index_(0), Mask_(0){};
        explicit const_iterator(const unsigned char* pointer,
                                size_t index,
                                size_t mask) :
                Pointer_(pointer),
                Index_(index),
                Mask_(mask){
        }

        const_iterator(const const_iterator& rhs) :
                Pointer_(rhs.Pointer_),
                Index_(rhs.Index_),
                Mask_(rhs.Mask_){
        }

        const_iterator& operator= (const const_iterator& rhs)
        {
            if(this == &rhs)
            {
                return *this;
            }

            Pointer_ = rhs.Pointer_;
            Index_ = rhs.Index_;
            Mask_ = rhs.Mask_;

            return *this;
        }

        const_iterator(const_iterator&& rhs) :
                Pointer_(rhs.Pointer_),
                Index_(rhs.Index_),
                Mask_(rhs.Mask_)
        {
            rhs.Pointer_ = nullptr;
            rhs.Index_ = 0;
            rhs.Mask_ = 0;
        }

        const_iterator& operator= (const_iterator&& rhs)
        {
            if(this == &rhs)
            {
                return *this;
            }

            Pointer_ = rhs.Pointer_;
            Index_ = rhs.Index_;
            Mask_ = rhs.Mask_;
            rhs.Pointer_ = nullptr;
            rhs.Index_ = 0;
            rhs.Mask_ = 0;

            return *this;
        }

        ~const_iterator(){
        }

        const_iterator& operator++()
        {
            Index_ = (Index_ + 1) & Mask_;
            return *this;
        }

        const const_iterator operator++(int)
        {
            const_iterator old(*this);
            Index_ = (Index_ + 1) & Mask_;

            return old;
        }

        const_iterator& operator--() = delete;
        const const_iterator& operator--(int) = delete;

        const_iterator operator+(const size_t offset)
        {
            return const_iterator(Pointer_, (Index_ + offset) & Mask_, Mask_);
        }

        const_iterator operator+=(const size_t offset)
        {
            Index_ = (Index_ + offset) & Mask_;
            return *this;
        }

        bool operator==(const const_iterator& rhs)
        {
            return Index_ == rhs.Index_ && Pointer_ == rhs.Pointer_ && Mask_ == rhs.Mask_;
        }

        bool operator!=(const const_iterator& rhs)
        {
            return Index_ != rhs.Index_ ||Pointer_ != rhs.Pointer_ || Mask_ != rhs.Mask_;
        }

        unsigned char operator*()
        {
            return Pointer_[Index_];
        }

    private:
        const unsigned char* Pointer_;
        size_t Index_;
        size_t Mask_;
    };

    static const size_t DefaultInitialSize = 1024;

    explicit Buffer(size_t size) :
            ReadIndex_(0),
            WriteIndex_(0),
            Capacity_(size),
            Mask_(size - 1),
            RingBuffer_(static_cast<unsigned char*>(std::malloc(sizeof(unsigned char) * size))),
            ReadBuffer_(nullptr)
    {
        assert(!(Capacity_ & Mask_));
    }

    explicit Buffer() :
            ReadIndex_(0),
            WriteIndex_(0),
            Capacity_(DefaultInitialSize),
            Mask_(DefaultInitialSize - 1),
            RingBuffer_(static_cast<unsigned char*>(std::malloc(sizeof(unsigned char) * DefaultInitialSize))),
            ReadBuffer_(nullptr)
    {
    }

    Buffer(const Buffer& rhs):
            ReadIndex_(rhs.ReadIndex_),
            WriteIndex_(rhs.WriteIndex_),
            Capacity_(rhs.Capacity_),
            Mask_(rhs.Mask_),
            RingBuffer_(static_cast<unsigned char*>(std::malloc(sizeof(unsigned char) * Capacity_))),
            ReadBuffer_(nullptr)
    {
        memmove(RingBuffer_, rhs.RingBuffer_, Capacity_);
    }

    Buffer& operator= (const Buffer& rhs)
    {
        if(this == &rhs)
        {
            return *this;
        }
        ReadIndex_ = rhs.ReadIndex_;
        WriteIndex_ = rhs.WriteIndex_;
        Capacity_ = rhs.Capacity_;
        Mask_ = rhs.Mask_;
        if(RingBuffer_)
        {
            std::free(RingBuffer_);
        }
        RingBuffer_ = static_cast<unsigned char*>(std::malloc(sizeof(unsigned char) * Capacity_));
        memmove(RingBuffer_, rhs.RingBuffer_, Capacity_);
        if(ReadBuffer_)
        {
            std::free(ReadBuffer_);
        }

        return *this;
    }

    Buffer(Buffer&& rhs):
            ReadIndex_(rhs.ReadIndex_),
            WriteIndex_(rhs.WriteIndex_),
            Capacity_(rhs.Capacity_),
            Mask_(rhs.Mask_),
            RingBuffer_(rhs.RingBuffer_),
            ReadBuffer_(rhs.ReadBuffer_)
    {
        rhs.ReadIndex_ = 0;
        rhs.WriteIndex_ = 0;
        rhs.Capacity_ = 0;
        rhs.Mask_ = 0;
        rhs.RingBuffer_ = nullptr;
        rhs.ReadBuffer_ = nullptr;
    }

    Buffer& operator= (Buffer&& rhs)
    {
        if(this == &rhs)
        {
            return *this;
        }
        ReadIndex_ = rhs.ReadIndex_;
        WriteIndex_ = rhs.WriteIndex_;
        Capacity_ = rhs.Capacity_;
        Mask_ = rhs.Mask_;
        if(RingBuffer_)
        {
            std::free(RingBuffer_);
        }
        RingBuffer_ = rhs.RingBuffer_;
        if(ReadBuffer_)
        {
            std::free(ReadBuffer_);
        }
        ReadBuffer_ = rhs.ReadBuffer_;

        rhs.ReadIndex_ = 0;
        rhs.WriteIndex_ = 0;
        rhs.Capacity_ = 0;
        rhs.Mask_ = 0;
        rhs.RingBuffer_ = nullptr;
        rhs.ReadBuffer_ = nullptr;

        return *this;
    }

    ~Buffer()
    {
        if(RingBuffer_)
        {
            std::free(RingBuffer_);
            RingBuffer_ = nullptr;
        }

        if(ReadBuffer_)
        {
            std::free(ReadBuffer_);
            ReadBuffer_ = nullptr;
        }
    }

    void swap(Buffer& rhs)
    {
        if(&rhs == this)
        {
            return;
        }

        std::swap(RingBuffer_, rhs.RingBuffer_);
        std::swap(ReadIndex_, rhs.ReadIndex_);
        std::swap(WriteIndex_, rhs.WriteIndex_);
        std::swap(Capacity_, rhs.Capacity_);
        std::swap(Mask_, rhs.Mask_);
    }

    size_t ReadableBytes() const
    {
        return (Capacity_ + WriteIndex_ - ReadIndex_) & Mask_;
    }

    size_t size() const
    {
        return ReadableBytes();
    }

    size_t capacity() const
    {
        return Mask_;
    }

    void clear()
    {
        ReadIndex_ = WriteIndex_ = 0;
    }

    const_iterator begin() const
    {
        return std::move(const_iterator(RingBuffer_, ReadIndex_, Mask_));
    }

    const_iterator end() const
    {
        return std::move(const_iterator(RingBuffer_, WriteIndex_, Mask_));
    }

    std::pair<const unsigned char*, size_t> Str() const
    {
        std::pair<const unsigned char*, size_t> ret;
        if(ReadIndex_ <= WriteIndex_)
        {
            ret.first = RingBuffer_ + ReadIndex_;
            ret.second = WriteIndex_ - ReadIndex_;
        }
        /*else if(ReadIndex_ == WriteIndex_)//merge with upper case
        {
            ret.first = RingBuffer_;
            ret.second = 0;
        }
        else if(ReadIndex_ == Capacity_) //impossible case
        {
            ret.first = RingBuffer_;
            ret.second = WriteIndex_;
        }*/
        else
        {
            /// +-------------------+------------------+------------------+
            /// |   readable bytes  |  writable bytes  |  readable bytes  |
            /// |     (CONTENT)     |                  |   (CONTENT)      |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      writerIndex   <=   readerIndex    <=     size
            if(!ReadBuffer_)
            {
                ReadBuffer_ = static_cast<unsigned char*>(std::malloc(sizeof(unsigned char) * Capacity_));
            }

            memmove(ReadBuffer_, RingBuffer_ + ReadIndex_, Capacity_ - ReadIndex_);
            memmove(ReadBuffer_ + Capacity_ - ReadIndex_, RingBuffer_, WriteIndex_);
            ret.first = ReadBuffer_;
            ret.second = ReadableBytes();
        }

        return std::move(ret);
    }

    size_t WritableBytes() const
    {
        return (Mask_ + ReadIndex_ - WriteIndex_) & Mask_;
    }

    size_t Append(const std::string& other)
    {
        return Append((const unsigned char *)other.c_str(), other.length());
    }

    size_t Append(const Buffer& other)
    {
        //TO DO:not efficient when in ReadBuffer_ case
        std::pair<const unsigned char*, size_t> ref = other.Str();
        return Append(ref.first, ref.second);
    }

    size_t Append(const unsigned char* data, size_t size)
    {
        size_t nw = WritableBytes();
        if(nw > size)
        {
            nw = size;
        }

        if(nw == 0)
        {
            return nw;
        }

        if(ReadIndex_ < WriteIndex_)
        {
            /// +-------------------+------------------+------------------+
            /// |   writable bytes  |  readable bytes  |  writable bytes  |
            /// |                   |     (CONTENT)    |                  |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      ReadIndex_   <=   WriteIndex_    <=     size
            if(Capacity_ - WriteIndex_ >= nw)
            {
                memmove(RingBuffer_ + WriteIndex_/*writeBegin1()*/, data, nw);
            }
            else
            {
                size_t writen = Capacity_ - WriteIndex_;
                memmove(RingBuffer_ + WriteIndex_/*writeBegin1()*/, data, writen);
                memmove(RingBuffer_, data + writen, nw - writen);
            }
        }
        else
        {
            /// +-------------------+------------------+------------------+
            /// |   readable bytes  |  writable bytes  |  readable bytes  |
            /// |     (CONTENT)     |                  |   (CONTENT)      |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      WriteIndex_   <=   ReadIndex_     <=     size
            memmove(RingBuffer_ + WriteIndex_, data, nw);
        }

        WriteIndex_ = (WriteIndex_ + nw) & Mask_;
        return nw;
    }

    size_t Cut(size_t size)
    {
        size_t nr = ReadableBytes();
        if(size < ReadableBytes())
        {
            nr = size;
        }
        ReadIndex_ = (ReadIndex_ + nr) & Mask_;

        return nr;
    }

    ssize_t AppendFromFileDiscriptor(int fd, size_t size)
    {
        iovec vec[2];
        int nvec = 0;
        size_t nw = WritableBytes();
        if(nw > size)
        {
            nw = size;
        }

        if(nw == 0)
        {
            return nw;
        }

        if(ReadIndex_ < WriteIndex_)
        {
            /// +-------------------+------------------+------------------+
            /// |   writable bytes  |  readable bytes  |  writable bytes  |
            /// |                   |     (CONTENT)    |                  |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      ReadIndex_   <=   WriteIndex_    <=     size
            if(Capacity_ - WriteIndex_ >= nw)
            {
                vec[0].iov_base = RingBuffer_ + WriteIndex_;
                vec[0].iov_len = nw;
                nvec = 1;
            }
            else
            {
                size_t writen = Capacity_ - WriteIndex_;
                vec[0].iov_base = RingBuffer_ + WriteIndex_;
                vec[0].iov_len = writen;
                vec[1].iov_base = RingBuffer_;
                vec[1].iov_len = nw - writen;
                nvec = 2;
            }
        }
        else
        {
            /// +-------------------+------------------+------------------+
            /// |   readable bytes  |  writable bytes  |  readable bytes  |
            /// |     (CONTENT)     |                  |   (CONTENT)      |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      WriteIndex_   <=   ReadIndex_     <=     size
            vec[0].iov_base = RingBuffer_ + WriteIndex_;
            vec[0].iov_len = nw;
            nvec = 1;
        }

        auto ret = readv(fd, vec, nvec);
        if(ret > 0)
        {
            WriteIndex_ = (WriteIndex_ + nw) & Mask_;
        }

        return ret;
    }

    ssize_t CutIntoFileDescriptor(int fd, size_t size)
    {
        iovec vec[2];
        int nvec = 0;
        size_t nr = ReadableBytes();
        if(nr > size)
        {
            nr = size;
        }

        if(ReadIndex_ < WriteIndex_)
        {
            /// +-------------------+------------------+------------------+
            /// |   writable bytes  |  readable bytes  |  writable bytes  |
            /// |                   |     (CONTENT)    |                  |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      ReadIndex_   <=   WriteIndex_    <=     size
            vec[0].iov_base = RingBuffer_ + ReadIndex_;
            vec[0].iov_len = nr;
            nvec = 1;
        }
        else
        {
            /// +-------------------+------------------+------------------+
            /// |   readable bytes  |  writable bytes  |  readable bytes  |
            /// |     (CONTENT)     |                  |   (CONTENT)      |
            /// +-------------------+------------------+------------------+
            /// |                   |                  |                  |
            /// 0      <=      WriteIndex_   <=   ReadIndex_     <=     size
            if(Capacity_ - ReadIndex_ > nr)
            {
                vec[0].iov_base = RingBuffer_ + ReadIndex_;
                vec[0].iov_len = nr;
                nvec = 1;
            }
            else
            {
                auto read = Capacity_ - ReadIndex_;
                vec[0].iov_base = RingBuffer_ + ReadIndex_;
                vec[0].iov_len = read;
                vec[1].iov_base = RingBuffer_;
                vec[1].iov_len = nr - read;
                nvec = 2;
            }
        }

        auto ret = writev(fd, vec, nvec);
        if(ret > 0)
        {
            ReadIndex_ = (ReadIndex_ + nr) & Mask_;
        }

        return ret;
    }

    friend inline std::ostream& operator<< (std::ostream& os, const Buffer& buf)
    {
        for(auto itr = buf.begin(); itr != buf.end(); ++itr)
        {
            os << *itr << " ";
        }
        return os;
    }

    //for gtest
    size_t ReadIndex() const{return  ReadIndex_;}
    size_t WriteIndex() const{return WriteIndex_;}

private:
    size_t ReadIndex_;
    size_t WriteIndex_;
    size_t Capacity_;
    size_t Mask_;
    unsigned char* RingBuffer_;
    mutable unsigned char* ReadBuffer_;
};

}

#endif

