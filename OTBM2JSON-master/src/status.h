////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_STATUS_H
#define __SURVIVALSERV_STATUS_H

#include <string>
#include "survivalsystem.h"
#include "definitions.h"


class Status{
  public:
	void addPlayer();
	void removePlayer();
	static Status* instance();
	std::string getStatusString();
	int playersonline, playersmax, playerspeak;
	std::string ownername, owneremail;
	std::string motd;
	std::string mapname, mapauthor;
	int mapsizex, mapsizey;
	std::string servername, location, url;
	std::string version;
	uint64_t start;

  private:
	Status();
	static Status* _Status;
};

#endif
