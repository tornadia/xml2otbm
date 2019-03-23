////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __SURVIVALSERV_TASKS_H
#define __SURVIVALSERV_TASKS_H

#include "scheduler.h"
#include "position.h"
#include "map.h"

class MovePlayer : public std::binary_function<Map*, Direction, int> {
		  public:
					 MovePlayer(unsigned long playerid) : _pid(playerid) { }

					 virtual result_type operator()(const first_argument_type& map, const second_argument_type& dir) const {
								SURVIVALSYS_THREAD_LOCK(map->mapLock)
									// get the player we want to move...
									Creature* creature = map->getCreatureByID(_pid);

								Player* player = dynamic_cast<Player*>(creature);
								if (!player) { // player is not available anymore it seems...
									SURVIVALSYS_THREAD_UNLOCK(map->mapLock)
										return -1;
								}
                                if(player->cancelMove){                                             
                                SURVIVALSYS_THREAD_UNLOCK(map->mapLock)                       
                                return 0;
                                }
                                
								Position pos = player->pos;
								switch (dir) {
										  case NORTH:
													 pos.y--;
													 break;
										  case EAST:
													 pos.x++;
													 break;
										  case SOUTH:
													 pos.y++;
													 break;
										  case WEST:
													 pos.x--;
													 break;
								}

#ifdef __DEBUG__
								std::cout << "Move to: " << dir << std::endl;
#endif
								map->thingMove(player, player, pos.x, pos.y, pos.z);
								SURVIVALSYS_THREAD_UNLOCK(map->mapLock)

                return 0;
					 }

		  protected:
					 unsigned long _pid;

};

class StopMovePlayer : public std::unary_function<Map*, bool> {
		  public:
					 StopMovePlayer(unsigned long playerid) : _pid(playerid) { }

					 virtual result_type operator()(const argument_type& map) const {
									// get the player we want to move...
									Creature* creature = map->getCreatureByID(_pid);

								Player* player = dynamic_cast<Player*>(creature);
								if (!player) { // player is not available anymore it seems...
										return false;
								}
                                else{
                                     if(player->cancelMove){
                                     player->cancelMove = false;
                                     player->sendCancelWalk("");
                                     return true;
                                                            }
                                     else
                                     return false;
                                     }
                     return false;
					 }

		  protected:
					 unsigned long _pid;

};
#endif
