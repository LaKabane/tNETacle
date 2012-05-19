#include "controler.h"
#include <vector>
#include <iostream>

Controler::Controler()
{
}

Controler::~Controler()
{
}

bool    Controler::addContact(Contact* c) {
    _list.addContact(c);
    return true;
}

bool	Controler::test()
{
  Contact	c1("test1", "keyyyyyyyyyyyyy1");
  Contact	c2("test2", "keyyyyyyyyyyyyy2");
  Contact	c3("test3", "keyyyyyyyyyyyyy3");
  Contact	c4("test4", "keyyyyyyyyyyyyy4");

  _list.addContact(&c1);
  _list.addContact(&c2);
  _list.addContact(&c3);
  _list.addContact(&c4);
  const std::vector<Contact*>&	cs = _list.getContact();

  std::cout << _list;

  std::cout << "erase test1 by name" << std::endl;
  _list.deleteContactByName("test1");

  std::cout << _list << std::endl;

  std::cout << "erase test2 by key" << std::endl;
  _list.deleteContactByKey("keyyyyyyyyyyyyy2");

  std::cout << _list;
}
