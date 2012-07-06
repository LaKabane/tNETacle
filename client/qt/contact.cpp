#include "contact.h"

Contact::Contact(const std::string& name, const std::string& key)
  : _name(name), _publicKey(key)
{
}

Contact::~Contact()
{
}

std::ostream&	operator<<(std::ostream& o, Contact& c)
{
  o << "name: " << c.getName() << " Public key: " << c.getPublicKey() << " Real I.P.: " << c.getRealIP() << " Virtual I.P.: " << c.getVirtualIP();
  return o;
}
