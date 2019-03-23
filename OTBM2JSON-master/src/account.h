////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <list>
#include <string>

class Player;

class Account
{
public:
  Account();
  ~Account();


  bool openAccount(const std::string &account, const std::string &givenpassword);
  bool openPlayer(const std::string &name, const std::string &givenpassword, Player &player);

  short int accType;
  unsigned short int premDays;

  std::string name;
  std::string password;

  std::list<std::string> charList;



protected:
  bool parseAccountFile(std::string filename);
  bool AddContainer(xmlNodePtr container, Item* item);
};


#endif




