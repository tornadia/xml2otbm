////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __THING_H__
#define __THING_H__

#include "definitions.h"
#include "position.h"

class Tile;

class Thing
{
public:
  Thing();
  virtual ~Thing();

  virtual bool canMovedTo(Tile *tile);

  int throwRange;
  Position pos;

};


#endif
