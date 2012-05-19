#ifndef _CORE_H
# define _CORE_H

#include "contactsList.h"
#include "config.h"

class Controler
{
public:
  Controler();
  ~Controler();

  bool	test();
  bool  addContact(Contact*);
private:
  ContactsList	_list;
  Config		_config;
};

#endif // !_CORE_H
