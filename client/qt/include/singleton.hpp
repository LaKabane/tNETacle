#ifndef _SINGLETON_HPP__
#define _SINGLETON_HPP__

namespace moost {

template <class T>
class singleton
{
public:
  static T * get_instance()
    {
      static T *inst = new T;
      return inst;
    }
protected:
  singleton(){};
  singleton(singleton<T> const &){};
};

}

#endif
