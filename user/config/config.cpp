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
#include "httpd.h"
#include <list>

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

#define CONFIGHTMLCHUNKSIZE	512

class CConfigHtmlRunner : public IConfigRunner {
	std::list<const char *> m_lCategories;
	struct Chunk {
		uint8_t pData[CONFIGHTMLCHUNKSIZE];
		size_t nLen;
	};
	size_t m_nTotalLen;
	std::list<Chunk> m_lChunks;

	void write(const uint8_t *pData, size_t nLen) {
		while (nLen) {
			if (m_lChunks.empty() || m_lChunks.back().nLen >= CONFIGHTMLCHUNKSIZE) {
				m_lChunks.push_back(Chunk());
			}
			Chunk& curChunk = m_lChunks.back();
			size_t nCurLen = std::min(nLen, CONFIGHTMLCHUNKSIZE - curChunk.nLen);
			memcpy(&curChunk.pData[curChunk.nLen], pData, nCurLen);
			curChunk.nLen += nCurLen;
			m_nTotalLen += nCurLen;
			nLen -= nCurLen;
			pData = &pData[nCurLen];
		}
	}
	void write_string(const char *szData) {
		write((const uint8_t *)szData, strlen(szData));
	}
	void write_fieldprefix() {
		for (auto name : m_lCategories) {
			write_string(name);
			write_string(".");
		}
	}

	void beginModule(const char *szName, const char *szDescription) {
		write_string("<fieldset><legend>");
		write_string(szDescription);
		write_string("</legend>");
		m_lCategories.push_back(szName);
	}
	void endModule() {
		write_string("</fieldset>");
		if (!m_lCategories.empty())
			m_lCategories.pop_back();
	}
	void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault) {
		write_string(szDescription);
		write_string(": <input type=\"checkbox\" name=\"");
		write_fieldprefix();
		write_string(szName);
		write_string("\" value=\"1\"");
		if (*pbValue)
			write_string(" checked");
		write_string("></input><br />");
	}
	void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
		write_string(szDescription);
		write_string(": <input type=\"text\" name=\"");
		write_fieldprefix();
		write_string(szName);
		write_string("\" value=\"");
		write_string(szValue);
		write_string("\"></input><br />");
	}
	void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
		char szTemp[12];
		write_string(szDescription);
		write_string(": <input type=\"text\" name=\"");
		write_fieldprefix();
		write_string(szName);
		write_string("\" value=\"");
		switch (nSize) {
			case 1: os_sprintf(szTemp,"%d",*(uint8_t *)pValue); break;
			case 2: os_sprintf(szTemp,"%d",*(uint16_t *)pValue); break;
			case 4: os_sprintf(szTemp,"%d",*(uint32_t *)pValue); break;
			default: szTemp[0]='\0'; break;
		}
		write_string(szTemp);
		write_string("\"></input><br />");
	}
	public:
	void write_header() {
		write_string("<html><head><title>EspLightNode Configuration</title></head><body><h1>EspLightNode Configuration</h1>");
		write_string("<form method=\"POST\" action=\"/save\">");
	}

	void write_footer() {
		write_string("<input type=\"submit\" value=\"Save\"></input></form>");
		write_string("</body></html>");
	}

	void start_transfer(struct HttpdConnectionSlot* slot) {
		char szHeaders[256];
		os_sprintf(szHeaders,"200 HTTP/1.0 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n",m_nTotalLen);
		httpd_slot_setsentcb(slot, &chunk_sent_cb, (void*)this);
		httpd_slot_send(slot, (uint8_t*)szHeaders, strlen(szHeaders));
	}

	void chunk_sent(struct HttpdConnectionSlot* slot) {
		if (!m_lChunks.empty()) {
			httpd_slot_setsentcb(slot, &chunk_sent_cb, this);
			httpd_slot_send(slot, m_lChunks.front().pData, m_lChunks.front().nLen);
			m_lChunks.pop_front();
		} else {
			httpd_slot_setdone(slot);
			delete this;
		}
	}

	static void chunk_sent_cb(struct HttpdConnectionSlot* slot, void *pData) {
		((CConfigHtmlRunner *)pData)->chunk_sent(slot);
	}
};

void config_html(struct HttpdConnectionSlot *slot) {
	CConfigHtmlRunner* pRunner = new CConfigHtmlRunner();
	pRunner->write_header();
	config_run(pRunner);
	pRunner->write_footer();
	pRunner->start_transfer(slot);
}

void config_submit(struct HttpdConnectionSlot *slot) {
	const char *response = "<html><head><title>Settings saving</title></head><body>Settings saving...</body></html>";
	char data[512];
	os_sprintf(data,"200 HTTP/1.0 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",strlen(response),response);
	httpd_slot_send(slot, (uint8_t *)data, strlen(data));
}
