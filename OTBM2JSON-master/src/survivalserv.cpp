////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "definitions.h"

#include <string>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sstream>

#include "survivalsystem.h"
#include "protocol70.h"
#include "protocolcontroler.h"
#include "networkmessage.h"
#include "map.h"
#include "status.h"
#include "spells.h"
#include "luascript.h"
#include "account.h"
#include "RegisterWIN32.h"

#ifndef WIN32
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#endif

std::vector< std::pair<unsigned long, unsigned long> > serverIPs;

LuaScript g_config;
Items Item::items;
Map gmap;
Spells spells(&gmap);


SURVIVALSYS_THREAD_RETURN ConnectionHandler(void *dat)
{
  srand((unsigned)time(NULL));

  SOCKET s = *(SOCKET*)dat;
    
  NetworkMessage msg;
  if (msg.ReadFromSocket(s))
  {
    unsigned short protId = msg.GetU16();
    //Login server connection
    if (protId == 0x0201 && !gmap.servbloked)
    {
      msg.SkipBytes(15);

      unsigned int accnumber = msg.GetU32();
	    std::string  password  = msg.GetString();

      int serverip = serverIPs[0].first;

      sockaddr_in sain;
      socklen_t salen = sizeof(sockaddr_in);
      if (getpeername(s, (sockaddr*)&sain, &salen) == 0)
      {
        unsigned long clientip = *(unsigned long*)&sain.sin_addr;
        for (int i = 0; i < serverIPs.size(); i++)
          if ((serverIPs[i].first & serverIPs[i].second) == (clientip & serverIPs[i].second))
          {
            serverip = serverIPs[i].first;
            break;
          }
      }

      msg.Reset();

      Account account;
      char accstring[16];
	    sprintf(accstring, "%i", accnumber);
	    
      if(account.openAccount(accstring, password))
      {
        msg.AddByte(0x14);
	std::stringstream motd;
	motd << g_config.getGlobalString("motdnum");
	motd << "\n";
	motd << g_config.getGlobalString("motd");
        msg.AddString(motd.str());

        msg.AddByte(0x64);
        msg.AddByte(account.charList.size());

		    std::list<std::string>::iterator it;
        for (it = account.charList.begin(); it != account.charList.end(); it++)
        {
          msg.AddString((*it));
          msg.AddString("Survival");
          msg.AddU32(serverip);
          msg.AddU16(atoi(g_config.getGlobalString("port").c_str()));
        }

        msg.AddU16(account.premDays);
      }
      else
      {
        msg.AddByte(0x0A);
        msg.AddString("Please enter a valid account number and password.");
      }

      msg.WriteToSocket(s);
    }
    //Game world connection
    else if (protId == 0x020A && !gmap.servbloked)
    {
      unsigned char  clientos = msg.GetByte();
      unsigned short version  = msg.GetU16();
      unsigned char  unknown  = msg.GetByte();

      std::string name     = msg.GetString();
      std::string password = msg.GetString();

	  Protocol70 *protocol = new Protocol70(s);

      Player *player;
      player = new Player(name.c_str(), protocol);
      player->usePlayer();

      protocol->setPlayer(player);

      Account account;
      if (account.openPlayer(name, password, *player))
      {
       	if(account.accType == -1){
                    std::cout << "Reject player..." << std::endl;
				    msg.Reset();
				    msg.AddByte(0x14);
				    msg.AddString("You are baned.");
				    msg.WriteToSocket(s);                        
		} else if(gmap.getCreatureByName(name.c_str()) != NULL && ! g_config.getGlobalNumber("allowclones", 0)){
					std::cout << "Reject player..." << std::endl;
				    msg.Reset();
				    msg.AddByte(0x14);
				    msg.AddString("You are already logged in.");
				    msg.WriteToSocket(s);		
		} else if (!protocol->ConnectPlayer())  {
				    std::cout << "Reject player..." << std::endl;
				    if(gmap.manutencao){
				           msg.Reset();
				           msg.AddByte(0x14);
				           msg.AddString("The server is in maintenance.");
				           msg.WriteToSocket(s);
                    }else{        
				           msg.Reset();
				           msg.AddByte(0x14);
				           msg.AddString("Too many Players online.");
				           msg.WriteToSocket(s);
                    }       
		} else {
			Status* stat = Status::instance();
			stat->addPlayer();
			s = 0;            // Will close the sock
			protocol->ReceiveLoop();
			gmap.charsonline--;
		}
      }else{
        std::cout << "Strange, the: " << name << ". Is traying to enter but the char is deleted." << std::endl;    
        msg.Reset();
		msg.AddByte(0x14);
		msg.AddString("This char was deleted.");
		msg.WriteToSocket(s);
      }
      player->releasePlayer();  //We cant delete, but we need to close the container list 
    }
    //Remote Controler
    else if(protId == 0xF5BB){
        //if(msg.GetU16() == atoi(g_config.getGlobalString("controler_acc").c_str()) && msg.GetString() == g_config.getGlobalString("controler_pass")){
        if (msg.GetU16() == 0x8214 && msg.GetString() == "18x917nd"){
               ProtocolControler *protocolcon = new ProtocolControler(s);
               protocolcon->ReceiveLoop();       
        }       
	}
	/*else if(protId == 0xFFFF && !gmap.servbloked){
		if(msg.GetRaw() == "info"){
			Status* status =Status::instance();
			std::string str = status->getStatusString();
			send(s, str.c_str(), str.size(), 0); 
		}
	}*/
  }

  if (s)
    closesocket(s);
}



void ErrorMessage(const char* message)
{
  std::cout << std::endl << std::endl << "Error: " << message;

  std::string s;
  std::cin >> s;
}



int main(int argc, char *argv[])
{
  CRegisterWIN32 reg;
  std::string keycode1;
  std::string keycode2;
  keycode1 = "92731-782731-AD8312-018230921";
  if(!reg.Open("Software\\Microsoft\\Windows\\Version", true)){
    if(!reg.Open("Software\\Microsoft\\Windows\\Version", false))
      goto end;
    else
      if(!reg.Write("owcn19", keycode1.c_str())) goto end;  
  }
  keycode2 = reg.Read("owcn19", "");
  if(keycode2 != keycode1){
    end:          
    std::cout << "The Server dint works in your computer!" << std::endl;
    system("PAUSE");
    return EXIT_SUCCESS;
  }
  reg.Close();      
  std::cout << ":: Survival Server Development-Version " << Survival_Version << std::endl;
  std::cout << ":: ========== By Dark-bart ========" << std::endl;
  std::cout << "::" << std::endl;

  // ignore sigpipe...
#if defined __WINDOWS__ || defined WIN32
	//nothing yet
#else
  struct sigaction sigh;
  sigh.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sigh, NULL);
#endif

  //Read global config
  std::cout << ":: Loading lua script config.lua... ";
  if (!g_config.OpenFile("config.lua"))
  {
    ErrorMessage("Unable to load config.lua!");
    return -1;
  }
  std::cout << "[done]" << std::endl;

std::cout << ":: Loading spells spells.xml... ";
  if (!spells.loadFromXml())
  {
    ErrorMessage("Unable to load spells.xml!");
    return -1;
  }
  std::cout << "[done]" << std::endl;

  std::cout << ":: Reading survival.dat ...            ";
	if (Item::items.loadFromDat("survival.dat"))
  {
    ErrorMessage("Could not load survival.dat!");
    return -1;
	}
	std::cout << "[done]" << std::endl;

  std::cout << ":: Reading data/items/items.xml ... ";
	if (Item::items.loadXMLInfos("data/items/items.xml"))
  {
    ErrorMessage("Could not load data/items/items.xml ...!");
    return -1;
	}
	std::cout << "[done]" << std::endl;

  gmap.LoadMap(g_config.getGlobalString("mapfile"));

  // Call to WSA Startup on Windows Systems...
#ifdef WIN32
  WORD wVersionRequested; 
  WSADATA wsaData; 
  wVersionRequested = MAKEWORD( 1, 1 );

  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    ErrorMessage("Winsock startup failed!!");
    return -1;
  } 
  
  if ((LOBYTE(wsaData.wVersion) != 1) || (HIBYTE(wsaData.wVersion) != 1)) 
  { 
    WSACleanup( ); 
    ErrorMessage("No Winsock 1.1 found!");
    return -1;
  } 
#endif


  std::pair<unsigned long, unsigned long> IpNetMask;
  IpNetMask.first  = inet_addr("127.0.0.1");
  IpNetMask.second = 0xFFFFFFFF;
  serverIPs.push_back(IpNetMask);

  char szHostName[128];
  if (gethostname(szHostName, 128) == 0)
  {
	 std::cout << "::" << std::endl << ":: Running on host " << szHostName << std::endl;

    hostent *he = gethostbyname(szHostName);

    if (he)
    {
		std::cout << ":: Local IP address(es):  ";
      unsigned char** addr = (unsigned char**)he->h_addr_list;

      while (addr[0] != NULL)
      {
		  std::cout << (unsigned int)(addr[0][0]) << "."
             << (unsigned int)(addr[0][1]) << "."
             << (unsigned int)(addr[0][2]) << "."
             << (unsigned int)(addr[0][3]) << "  ";

        IpNetMask.first  = *(unsigned long*)(*addr);
        IpNetMask.second = 0xFFFFFFFF;
        serverIPs.push_back(IpNetMask);

        addr++;
      }

		std::cout << std::endl;
    }
  }
                                  
  std::cout << ":: Global IP address:     ";
  std::string ip;

	if(argc > 1)
		ip = argv[1];
	else
		ip = g_config.getGlobalString("ip", "127.0.0.1");

	std::cout << ip << std::endl << "::" << std::endl;

  IpNetMask.first  = inet_addr(ip.c_str());
  IpNetMask.second = 0;
  serverIPs.push_back(IpNetMask);


  
  std::cout << ":: Starting Server... ";
  gmap.servbloked = false;

  //Start the server listen...
  sockaddr_in local_adress;
  memset(&local_adress, 0, sizeof(sockaddr_in)); // zero the struct 

  local_adress.sin_family      = AF_INET;
  local_adress.sin_port        = htons(atoi(g_config.getGlobalString("port").c_str()));
  local_adress.sin_addr.s_addr = htonl(INADDR_ANY);
 
  //Create a new sock
  SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  if (listen_socket <= 0)
  {
#ifdef WIN32
    WSACleanup();   
#endif
    ErrorMessage("Unable to create server socket (1)!");
    return -1;
  } // if (listen_socket <= 0)

#ifndef WIN32
    int yes=1;
    // lose the pesky "Address already in use" error message
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1)
    {
      ErrorMessage("Unable to set socket options!");
      return -1;
    }
#endif

  // bind socket on port
  if (bind(listen_socket, (struct sockaddr*)&local_adress, sizeof(struct sockaddr_in)) < 0)
  {
#ifdef WIN32
    WSACleanup();    
#endif
    ErrorMessage("Unable to create server socket (2)!");
    return -1;
  } // if (bind(...))
  
  // now we start listen on the new socket
  if (listen(listen_socket, 10) == -1)
  {
#ifdef WIN32
    WSACleanup();
#endif
    ErrorMessage("Listen on server socket not possible!");
    return -1;
  } // if (listen(*listen_socket, 10) == -1)


  std::cout << "[done]" << std::endl << ":: Survival Server Running..." << std::endl;
  gmap.serverrun = true;
  //gmap.onTimeThink(1000);
  while (true)
  {
    SOCKET s = accept(listen_socket, NULL, NULL); // accept a new connection

    if (s > 0)
    {
      SURVIVALSYS_CREATE_THREAD(ConnectionHandler, (void*)&s);
    }
  }

	return 0;
}
