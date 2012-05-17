#include "contactsList.h"

ContactsList::ContactsList()
{
}

ContactsList::~ContactsList()
{
}

void				ContactsList::addContact(Contact* c)
{
  _list.push_back(c);
}

const std::vector<Contact*>&	ContactsList::getContact() const
{
  return _list;
}

bool				ContactsList::deleteContactByName(const std::string& name)
{
  bool					ret = false;
  std::vector<Contact*>::iterator	it = _list.begin();
  std::vector<Contact*>::const_iterator	ite = _list.end();

  while (it != ite)
  {
    if ((*it)->getName() == name)
    {
      it = _list.erase(it);
      ret = true;
    }
    else
    {
      ++it;
    }
  }
  return ret;
}

bool				ContactsList::deleteContactByKey(const std::string& key)
{
  bool					ret = false;
  std::vector<Contact*>::iterator	it = _list.begin();
  std::vector<Contact*>::const_iterator	ite = _list.end();

  while (it != ite)
  {
    if ((*it)->getPublicKey() == key)
    {
      it = _list.erase(it);
      ret = true;
    }
    else
    {
      ++it;
    }
  }
  return ret;
}

std::ostream&	operator<<(std::ostream& o, ContactsList& c)
{

  std::vector<Contact*>::const_iterator cisb = c._list.begin();
  std::vector<Contact*>::const_iterator cise = c._list.end();
  while (cisb != cise)
  {
    o << *(*cisb) << std::endl;
    ++cisb;
  }
  return o;
}
