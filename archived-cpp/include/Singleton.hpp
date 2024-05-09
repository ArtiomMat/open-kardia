#ifndef SINGLETON_HPP_INCLUDED
#define SINGLETON_HPP_INCLUDED

template <typename Type>
class Singleton
{
  public:
  void setInstance();

  private:
  static Type* instance;
};

#endif // SINGLETON_HPP_INCLUDED
