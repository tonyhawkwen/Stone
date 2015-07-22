#ifndef _STONE_NONCOPYABLE_H_
#define _STONE_NONCOPYABLE_H_


namespace Stone{

class Noncopyable
{
protected:
	Noncopyable() = default;
	~Noncopyable() = default;
	Noncopyable(const Noncopyable&) = delete;
	const Noncopyable& operator=(const Noncopyable&) = delete;
};


}


#endif


