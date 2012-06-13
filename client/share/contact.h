#ifndef _CONTACT_H
# define _CONTACT_H

#include <string>
#include <iostream>

class Contact
{
public:
  Contact(const std::string& name, const std::string& key);
  ~Contact();

  const std::string& getName() {return _name;}
  const std::string& getPublicKey() {return _publicKey;}
  const std::string& getRealIP() {return _realIP;}
  const std::string& getVirtualIP() {return _virtualIP;}
  void setName(std::string& name) {_name = name;}
  void setPublicKey(std::string& publicKey) {_publicKey = publicKey;}
  void setRealIP(std::string& realIP) {_realIP = realIP;}
  void setVirtualIP(std::string& virtualIP) {_virtualIP = virtualIP;}

private:
  std::string	_name;
  std::string	_publicKey;
  std::string	_realIP;
  std::string	_virtualIP;
};

std::ostream& operator<<(std::ostream &flux, Contact&);

#endif // !_CONTACT_H
