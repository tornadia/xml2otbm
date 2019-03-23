////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#include "status.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "luascript.h"
#include <sstream>

extern LuaScript g_config;

Status* Status::_Status = NULL;

Status* Status::instance(){
	if(_Status == NULL)
		_Status = new Status();
	return _Status;
}

Status::Status(){
	this->playersonline=0;
	this->playersmax=0;
	this->start=SURVIVALSYS_TIME();
}

void Status::addPlayer(){
	this->playersonline++;
	if(playersmax < playersonline)
	  playersmax=playersonline;
}
void Status::removePlayer(){
	this->playersonline--;
}

std::string Status::getStatusString(){
	std::string xml;

	std::stringstream ss;

	xmlDocPtr doc;
	xmlNodePtr p, root;

	doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"tsqp", NULL);
	root=doc->children;
	
	xmlSetProp(root, (const xmlChar*) "version", (const xmlChar*)"1.0");
	
	
	p=xmlNewNode(NULL,(const xmlChar*)"serverinfo");
	uint64_t running = (SURVIVALSYS_TIME() - this->start)/1000;
	ss << running;
	xmlSetProp(p, (const xmlChar*) "uptime", (const xmlChar*)ss.str().c_str());
	ss.str("");
	xmlSetProp(p, (const xmlChar*) "ip", (const xmlChar*)g_config.getGlobalString("ip", "").c_str());
	xmlSetProp(p, (const xmlChar*) "servername", (const xmlChar*)g_config.getGlobalString("servername", "").c_str());
	xmlSetProp(p, (const xmlChar*) "port", (const xmlChar*)g_config.getGlobalString("port", "").c_str());
	xmlSetProp(p, (const xmlChar*) "location", (const xmlChar*)g_config.getGlobalString("location", "").c_str());
	xmlSetProp(p, (const xmlChar*) "url", (const xmlChar*)g_config.getGlobalString("url", "").c_str());
	xmlSetProp(p, (const xmlChar*) "server", (const xmlChar*)"survivalserv");
	xmlSetProp(p, (const xmlChar*) "version", (const xmlChar*)"0.3.0");
	xmlAddChild(root, p);

	p=xmlNewNode(NULL,(const xmlChar*)"owner");
	xmlSetProp(p, (const xmlChar*) "name", (const xmlChar*)g_config.getGlobalString("ownername", "").c_str());
	xmlSetProp(p, (const xmlChar*) "email", (const xmlChar*)g_config.getGlobalString("owneremail", "").c_str());
	xmlAddChild(root, p);

	p=xmlNewNode(NULL,(const xmlChar*)"players");
	ss << this->playersonline;
	xmlSetProp(p, (const xmlChar*) "online", (const xmlChar*)ss.str().c_str());
	ss.str("");
	xmlSetProp(p, (const xmlChar*) "max", (const xmlChar*)g_config.getGlobalString("maxplayers", "").c_str());
	ss << this->playersmax;
	xmlSetProp(p, (const xmlChar*) "peak", (const xmlChar*)ss.str().c_str());
	ss.str("");
	xmlAddChild(root, p);

	p=xmlNewNode(NULL,(const xmlChar*)"map");
	xmlSetProp(p, (const xmlChar*) "name", (const xmlChar*)this->mapname.c_str());
	xmlSetProp(p, (const xmlChar*) "author", (const xmlChar*)this->mapauthor.c_str());
	xmlSetProp(p, (const xmlChar*) "width", (const xmlChar*)"");
	xmlSetProp(p, (const xmlChar*) "height", (const xmlChar*)"");
	xmlAddChild(root, p);

	xmlNewTextChild(root, NULL, (const xmlChar*)"motd", (const xmlChar*)g_config.getGlobalString("motd", "").c_str());

	char* s=NULL;
	int len=0;
	xmlDocDumpMemory(doc, (xmlChar**)&s, &len);

	xml=std::string(s, len);

	xmlFree(s);

	return xml;
}
