////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "definitions.h"
#include "tile.h"
#include "survivalsystem.h"
#include "protocol.h"

class Player;

extern Map gmap;


Protocol::Protocol()
{
}


Protocol::~Protocol()
{
}


void Protocol::setPlayer(Player* p)
{
	player = p;
  map    = &gmap;
}

unsigned long Protocol::GetProtocolIp()
{
    sockaddr_in sain;
    socklen_t salen = sizeof(sockaddr_in);
    getpeername(s, (sockaddr*)&sain, &salen);
    return *(unsigned long*)&sain.sin_addr;     
}         

void Protocol::sleepTillMove(){
	int ground = map->getTile(player->pos.x, player->pos.y, player->pos.z)->ground.getID();
	long long delay = ((long long)player->lastmove + (long long)player->getStepDuration(Item::items[ground].speed)) -
				((long long)SURVIVALSYS_TIME());
    
	if(delay > 0){
             
        //#if __DEBUG__     	
		//#endif
		
		SURVIVALSYS_SLEEP((uint32_t)delay);
	}
	
	/*long long movedelay = (long long)SURVIVALSYS_TIME()-(long long)player->lastmove;
	if((long long)movedelay != (long long)SURVIVALSYS_TIME() && (movedelay < player->movedelay || player->movedelay <= 0))
       player->movedelay = movedelay;
         
    std::cout 
    << "Delay: "
     << player->movedelay
      << " Step: "
       << (long long)player->getStepDuration(Item::items[ground].speed)
        << " Ground speed: " << (long long)Item::items[ground].speed
          << " Delay2: "
           << delay
            << "." 
             << 
              std::endl;*/
	
    player->lastmove = SURVIVALSYS_TIME();   
}
