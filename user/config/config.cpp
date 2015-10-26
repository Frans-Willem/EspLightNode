/*
 * config.c
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */
#include <sdkfixup.h>
extern "C" {
#include <mem.h>
#include <osapi.h>
}
#include "config.h"
#include <string.h>
#include <list>
#include "CConfigHtmlGenerator.h"
#include <sstream>
#include "httpd/CHttpServer.h"
#include "config/CConfigPostHandler.h"
#include "config/CConfigLoader.h"
#include "output_protocols/output.h"

DEFINE_CONFIG(artnet);
DEFINE_CONFIG(tpm2net);
DEFINE_CONFIG(wifi);
DEFINE_CONFIG(network);

void config_run(IConfigRunner *_configrunner) {
	CONFIG_SUB(wifi);
	CONFIG_SUB(network);
	CONFIG_SUB(Output::config);
	CONFIG_SUB(artnet);
	CONFIG_SUB(tpm2net);
}

void config_load() {
	CConfigLoader::load();
}

class CConfigHttpServerListener : IHttpServerListener {
public:
	static void attach(CHttpServer *pServer) {
		if (m_gInstance == NULL)
			m_gInstance = new CConfigHttpServerListener();
		pServer->addListener(m_gInstance);
	}
	static void detach(CHttpServer *pServer) {
		pServer->removeListener(m_gInstance);
	}
private:
	static CConfigHttpServerListener* m_gInstance;

	bool onRequest(CHttpServer *pServer, CHttpRequest *pRequest) {
		std::string szUri = pRequest->getUri();
		if (szUri.compare("/") == 0 || szUri.compare("/index.html")==0) {
			CConfigHtmlGenerator::handle(pRequest);
			return true;
		}
		if (szUri.compare("/submit.cgi") == 0) {
			CConfigPostHandler::handle(pRequest);
			return true;
		}
		return false;
	}

};
CConfigHttpServerListener* CConfigHttpServerListener::m_gInstance = NULL;

void config_init(CHttpServer *pServer) {
	CConfigHttpServerListener::attach(pServer);
}

