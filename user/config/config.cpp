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

class CConfigRunnerDefault : public IConfigRunner {
	void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool pbDefault) {
		*pbValue = pbDefault;
	}
	void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
		strncpy(szValue, szDefault, nSize-1);
		szValue[nSize-1]='\0';
	}
	void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
		switch (nSize) {
			case 1: *(uint8_t *)pValue = nDefault; break;
			case 2: *(uint16_t *)pValue = nDefault; break;
			case 4: *(uint32_t *)pValue = nDefault; break;
		}
	}
};

DEFINE_CONFIG(artnet);
DEFINE_CONFIG(tpm2net);

void config_run(IConfigRunner *runner) {
	artnet_runconfig(runner);
	tpm2net_runconfig(runner);
}

void config_load() {
	CConfigRunnerDefault cRunner;
	config_run(&cRunner);
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

