#ifndef ERRORS_HPP_INCLUDED
#define ERRORS_HPP_INCLUDED

// Lightweight errors module

struct Error
{
  const char* str;

  Error(const char* _str) { str = _str; }
  Error() : Error("") {}
};

// Reading or writing error, it may also mean just general issues with a bad file format
struct FileError : Error
{
  FileError() : Error("") {}
  FileError(const char* _str) : Error(_str) {}
};

// Anything that is system specific, originates from unknown issues on the kernel or operating system level.
struct SystemError : Error
{
  SystemError() : Error("") {}
  SystemError(const char* _str) : Error(_str) {}
};

// Allocation/Deallocation error
struct MemoryError : Error
{
  MemoryError() : Error("") {}
  MemoryError(const char* _str) : Error(_str) {}
};



#endif // ERRORS_HPP_INCLUDED
