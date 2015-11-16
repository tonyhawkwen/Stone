#ifndef _STONE_TCP_PROTOCAL_H_
#define _STONE_TCP_PROTOCAL_H_

namespace Stone {

class Protocal {
public:
    Protocal(char* buf, size_t length):
        Buffer_(nullptr),
        Length_(length),
        Type_(0),
        Device_(0),
        Data_(nullptr),
        DataSize_(0)
    {
        if(Length_ > 0)
        {
            Buffer_ = new char[Length_];
            memmove(Buffer_, buf, length);
        }
        decode();
    }

    Protocal(int device, int type, char* data, unsigned int size):
        Buffer_(nullptr),
        Length_(0),
        Type_(type),
        Device_(device),
        Data_(nullptr),
        DataSize_(size)
    {
        if(DataSize_ > 0)
        {
            Data_ = new char[DataSize_];
            memmove(Data_, data, size);
        }
        encode();
    }

    ~Protocal()
    {
        delete [] Buffer_;
        delete [] Data_;
    }

    //TO DO: copy construction and move construction

    int GetType(){
        return Type_;
    }
    int GetDevice(){
        return Device_;
    }

    unsigned int GetData(char* data) {
        memmove(data, Data_, DataSize_);
        return DataSize_;
    }

    unsigned int GetBuffer(char* buffer) {
        memmove(buffer, Buffer_, Length_);
        return Length_;
    }

    unsigned int GetData(const char* &data) {
        data = Data_;
        return DataSize_;
    }

    unsigned int GetBuffer(const char* &buffer) {
        buffer = Buffer_;
        return Length_;
    }

private:
    void encode()
    {
        Length_ = 4 + DataSize_;
        Buffer_ = new char[Length_];
        char* buf = Buffer_;
        unsigned int head = (Device_ << 30) | (Type_ << 24) | DataSize_;
        memmove(buf, &head, 4);
        if(DataSize_ == 0)
        {
            return;
        }
        buf += 4;
        memmove(buf, Data_, DataSize_);
    }

    void decode()
    {
        if(Length_ < 4)
        {
            return;
        }
        unsigned int head = 0;
        memmove(&head, Buffer_, 4);
        Device_ = head >> 30;
        Type_ = (head << 2) >> 26;
        DataSize_ = head & 0xFFFFFF;

        if(DataSize_ == 0)
        {
            return;
        }

        Data_ = new char[DataSize_];
        memmove(Data_, Buffer_ + 4, DataSize_);
    }

    char* Buffer_;
    size_t Length_;
    unsigned int Type_;
    unsigned int Device_;
    char* Data_;
    unsigned int DataSize_;
};

}

#endif
