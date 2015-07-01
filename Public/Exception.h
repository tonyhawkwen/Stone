#ifndef _ESSIE_EXCEPTION_H_
#define _ESSIE_EXCEPTION_H_
#include <sstream>
#include <errno.h>
#include <stdexcept>
#include <system_error>
#include <string.h>

namespace Essie{

void throwSystemError(const char*);
inline void throwSystemError(const char* msg)
{
	throw std::system_error(errno, std::system_category(), msg);
}

void throwSystemError(int err, const char*);
inline void throwSystemError(int err, const char* msg)
{
	throw std::system_error(err, std::system_category(), msg);
}

class Exception : public std::exception {
 public:
	explicit Exception(const std::string& value) : value_(value) {}
	explicit Exception(const std::string&& value) : value_(std::move(value)) {}
	virtual ~Exception(void) throw() {}

	virtual const char *what(void) const throw() 
	{
		return value_.c_str();
	}

private:
	std::string value_;
};


class RunException : public std::runtime_error {
public:
	enum ExceptionType
	{
		UNKNOWN = 0,
	};

	RunException(const std::string& message) : std::runtime_error(t_GetMessage(message, UNKNOWN, 0)), m_Type(UNKNOWN), m_Errno(0){}
	RunException(ExceptionType type, const std::string& message) : std::runtime_error(t_GetMessage(message, type, 0)), m_Type(type), m_Errno(0){}
	RunException(ExceptionType type, const std::string& message, int err) : std::runtime_error(t_GetMessage(message, type, err)), m_Type(type), m_Errno(err){}

	ExceptionType getType() const noexcept { return m_Type; }
	int getErrno() const noexcept { return m_Errno; }

protected:
	std::string t_GetMessage(const std::string &message, ExceptionType type, int err)
	{
		std::stringstream ss;
		ss << "Exception [" << message << "]type[" << (int)type << "]errno[" << err << "]";
		if (err != 0) 
		{
			char buffer[256];
			ss <<"errinfo[" << strerror_r(err, buffer, 256) << "]";
		}
		
		return ss.str();
	}

private:
	ExceptionType m_Type;
	int m_Errno;
};

}

#endif


