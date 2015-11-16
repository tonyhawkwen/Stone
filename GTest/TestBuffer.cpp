#include <gtest/gtest.h>
#include "../Public/Buffer.h"

/*g++ -o testBuffer TestBuffer.cpp --std=c++11 -lgtest*/

using namespace Stone;

TEST(Buffer, DefaultConstructor)
{
    const Buffer s;
    EXPECT_EQ(1023, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(1023, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());
    auto str_pair = s.Str();
    EXPECT_EQ(0, str_pair.second);
}

TEST(Buffer, Constructor)
{
    const Buffer s(4);
    EXPECT_EQ(3, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());
    auto str_pair = s.Str();
    EXPECT_EQ(0, str_pair.second);
}

TEST(Buffer, CopyConstructor)
{
    const Buffer src(4);
    const Buffer s(src);
    EXPECT_EQ(3, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());

    EXPECT_EQ(3, src.capacity());
    EXPECT_EQ(0, src.size());
    EXPECT_EQ(true, src.begin() == src.end());
    EXPECT_EQ(false, src.begin() == s.begin());
    EXPECT_EQ(0, src.ReadableBytes());
    EXPECT_EQ(3, src.WritableBytes());
    EXPECT_EQ(0, src.ReadIndex());
    EXPECT_EQ(0, src.WriteIndex());
}

TEST(Buffer, MoveConstructor)
{
    Buffer src(4);
    Buffer s(std::move(src));
    EXPECT_EQ(3, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());

    EXPECT_EQ(0, src.capacity());
    EXPECT_EQ(0, src.size());
    EXPECT_EQ(true, src.begin() == src.end());
    EXPECT_EQ(false, s.begin() == src.begin());
    EXPECT_EQ(0, src.ReadableBytes());
    EXPECT_EQ(0, src.WritableBytes());
    EXPECT_EQ(0, src.ReadIndex());
    EXPECT_EQ(0, src.WriteIndex());
}

TEST(Buffer, CopyAssignOperator)
{
    Buffer src(4);
    Buffer s;
    s = src;
    EXPECT_EQ(3, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());

    EXPECT_EQ(3, src.capacity());
    EXPECT_EQ(0, src.size());
    EXPECT_EQ(true, src.begin() == src.end());
    EXPECT_EQ(false, src.begin() == s.begin());
    EXPECT_EQ(0, src.ReadableBytes());
    EXPECT_EQ(3, src.WritableBytes());
    EXPECT_EQ(0, src.ReadIndex());
    EXPECT_EQ(0, src.WriteIndex());
}

TEST(Buffer, MoveAssignOperator)
{
    Buffer src(4);
    Buffer s;
    s = std::move(src);
    EXPECT_EQ(3, s.capacity());
    EXPECT_EQ(0, s.size());
    EXPECT_EQ(true, s.begin() == s.end());
    EXPECT_EQ(0, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());

    EXPECT_EQ(0, src.capacity());
    EXPECT_EQ(0, src.size());
    EXPECT_EQ(true, src.begin() == src.end());
    EXPECT_EQ(false, s.begin() == src.begin());
    EXPECT_EQ(0, src.ReadableBytes());
    EXPECT_EQ(0, src.WritableBytes());
    EXPECT_EQ(0, src.ReadIndex());
    EXPECT_EQ(0, src.WriteIndex());
}

TEST(Buffer, Append)
{
    Buffer s(4);
    const char* data = "12345678";
    EXPECT_EQ(3, s.Append((const unsigned char*)data, 4));
    EXPECT_EQ(0, s.Append((const unsigned char*)data, 8));
    EXPECT_EQ(3, s.ReadableBytes());
    EXPECT_EQ(0, s.WritableBytes());
    s.clear();
    EXPECT_EQ(3, s.Append((const unsigned char*)data, 3));
    EXPECT_EQ(0, s.Append((const unsigned char*)data, 3));
    s.clear();
    EXPECT_EQ(1, s.Append((const unsigned char*)data, 1));
    EXPECT_EQ(2, s.Append((const unsigned char*)data, 8));
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(3, s.WriteIndex());
    EXPECT_EQ(0, s.Append((const unsigned char*)data, 1));
    EXPECT_EQ(1, s.Cut(1));
    EXPECT_EQ(2, s.ReadableBytes());
    EXPECT_EQ(1, s.WritableBytes());
    EXPECT_EQ(1, s.ReadIndex());
    EXPECT_EQ(3, s.WriteIndex());
    EXPECT_EQ(1, s.Append((const unsigned char*)data, 1));
    EXPECT_EQ(1, s.ReadIndex());
    EXPECT_EQ(0, s.WriteIndex());
}

TEST(Buffer, Cut)
{
    Buffer s(8);
    const char* data = "1234567";
    EXPECT_EQ(7, s.Append((const unsigned char*)data, 7));
    EXPECT_EQ(3, s.Cut(3));
    EXPECT_EQ(4, s.ReadableBytes());
    EXPECT_EQ(3, s.WritableBytes());
    EXPECT_EQ(4, s.Cut(7));
    EXPECT_EQ(7, s.Append((const unsigned char*)data, 7));
    EXPECT_EQ(6, s.Cut(6));
    EXPECT_EQ(4, s.Append((const unsigned char*)data, 4));
    EXPECT_EQ(5, s.ReadableBytes());
    EXPECT_EQ(2, s.WritableBytes());
    EXPECT_EQ(5, s.Cut(10));
}

TEST(Buffer, Str)
{
    Buffer s(8);
    const char* data = "1234567";
    EXPECT_EQ(7, s.Append((const unsigned char*)data, 7));
    auto str_pair = s.Str();
    EXPECT_EQ(7, str_pair.second);
    unsigned char base1[] = {'1', '2', '3', '4', '5', '6', '7'};
    for(auto i = 0; i < str_pair.second; ++i)
    {
        EXPECT_EQ(base1[i], str_pair.first[i]);
    }
    EXPECT_EQ(4, s.Cut(4));
    str_pair = s.Str();
    EXPECT_EQ(3, str_pair.second);
    unsigned char base2[] = {'5', '6', '7'};
    for(auto i = 0; i < str_pair.second; ++i)
    {
        EXPECT_EQ(base2[i], str_pair.first[i]);
    }
    EXPECT_EQ(4, s.ReadIndex());
    EXPECT_EQ(7, s.WriteIndex());
    EXPECT_EQ(3, s.Append((const unsigned char*)data, 3));
    EXPECT_EQ(4, s.ReadIndex());
    EXPECT_EQ(2, s.WriteIndex());
    /*for(auto itr = s.begin(); itr != s.end(); ++itr)
    {
        std::cout <<"itr:" << *itr << std::endl;
    }*/
    str_pair = s.Str();
    EXPECT_EQ(6, str_pair.second);
    unsigned char base3[] = {'5', '6', '7', '1', '2', '3'};
    for(auto i = 0; i < str_pair.second; ++i)
    {
        EXPECT_EQ(base3[i], str_pair.first[i]);
    }
    EXPECT_EQ(1, s.Append((const unsigned char*)data, 3));
    str_pair = s.Str();
    EXPECT_EQ(7, str_pair.second);
    unsigned char base4[] = {'5', '6', '7', '1', '2', '3', '1'};
    for(auto i = 0; i < str_pair.second; ++i)
    {
        EXPECT_EQ(base4[i], str_pair.first[i]);
    }
    EXPECT_EQ(4, s.Cut(4));
    EXPECT_EQ(0, s.ReadIndex());
    EXPECT_EQ(3, s.WriteIndex());
    unsigned char base5[] = {'2', '3', '1'};
    str_pair = s.Str();
    EXPECT_EQ(3, str_pair.second);
    for(auto i = 0; i < str_pair.second; ++i)
    {
        EXPECT_EQ(base5[i], str_pair.first[i]);
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
