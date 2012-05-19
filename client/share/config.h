#ifndef _CONFIG_H
# define _CONFIG_H

#include <string>
#include <map>

class Config
{
public:
  Config();
  ~Config();

  void	change();
  void	save();
  void	read();

private:
  std::map<std::string, std::string>	_conf;
};

#endif // !_CONFIG_H
