#ifndef _CONTACTSLIST_H
# define _CONTACTSLIST_H

#include "contact.h"
#include <vector>
#include <string>
#include <iostream>

class ContactsList
{
  friend std::ostream& operator<<(std::ostream &flux, ContactsList&);
public:
  ContactsList();
  ~ContactsList();

  void	addContact(Contact*);
  bool	deleteContactByName(const std::string&);
  bool	deleteContactByKey(const std::string&);
  const std::vector<Contact*>&	getContact() const;

private:
  std::vector<Contact*>	_list;
};

#endif // !_CONTACTSLIST_H
