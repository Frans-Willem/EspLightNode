#include <sdkfixup.h>
extern "C" {
#include <mem.h>
#include <osapi.h>
}
#include "CConfigHtmlGenerator.h"
#include "httpd.h"

void CConfigHtmlGenerator::beginModule(const char *szName, const char *szDescription) {
	write("<fieldset><legend>");
	write(szDescription);
	write("</legend>");
	m_lCategories.push_back(szName);
}
void CConfigHtmlGenerator::endModule() {
	write("</fieldset>");
	if (!m_lCategories.empty())
		m_lCategories.pop_back();
}
void CConfigHtmlGenerator::optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault) {
	write(szDescription);
	write(": <input type=\"checkbox\" name=\"");
	write_fieldprefix();
	write(szName);
	write("\" value=\"1\"");
	if (*pbValue)
		write(" checked");
	write("></input><br />");
}
void CConfigHtmlGenerator::optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
	write(szDescription);
	write(": <input type=\"text\" name=\"");
	write_fieldprefix();
	write(szName);
	write("\" value=\"");
	write(szValue);
	write("\"></input><br />");
}
void CConfigHtmlGenerator::optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
	char szTemp[12];
	write(szDescription);
	write(": <input type=\"text\" name=\"");
	write_fieldprefix();
	write(szName);
	write("\" value=\"");
	switch (nSize) {
		case 1: os_sprintf(szTemp,"%d",*(uint8_t *)pValue); break;
		case 2: os_sprintf(szTemp,"%d",*(uint16_t *)pValue); break;
		case 4: os_sprintf(szTemp,"%d",*(uint32_t *)pValue); break;
		default: szTemp[0]='\0'; break;
	}
	write(szTemp);
	write("\"></input><br />");
}
//Only
void CConfigHtmlGenerator::write_header() {
	write("<html><head><title>EspLightNode Configuration</title></head><body><h1>EspLightNode Configuration</h1>");
	write("<form method=\"POST\" action=\"/save\">");
}
void CConfigHtmlGenerator::write_footer() {
	write("<input type=\"submit\" value=\"Save\"></input></form>");
	write("</body></html>");
}
void CConfigHtmlGenerator::start_transfer(struct HttpdConnectionSlot* slot) {
		char szHeaders[256];
		os_sprintf(szHeaders,"200 HTTP/1.0 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n",m_nTotalLen);
		httpd_slot_setsentcb(slot, &chunk_sent_callback, (void*)this);
		httpd_slot_send(slot, (uint8_t*)szHeaders, strlen(szHeaders));
}
void CConfigHtmlGenerator::write_fieldprefix() {
	for (auto name : m_lCategories) {
		write(name);
		write(".");
	}
}
void CConfigHtmlGenerator::write(const uint8_t* pData, size_t nLen) {
	while (nLen) {
		if (m_lChunks.empty() || m_lChunks.back().nLen >= sizeof(Chunk::pData)) {
			m_lChunks.push_back(Chunk());
		}
		Chunk& curChunk = m_lChunks.back();
		size_t nCurLen = std::min(nLen, sizeof(Chunk::pData) - curChunk.nLen);
		memcpy(&curChunk.pData[curChunk.nLen], pData, nCurLen);
		curChunk.nLen += nCurLen;
		m_nTotalLen += nCurLen;
		nLen -= nCurLen;
		pData = &pData[nCurLen];
	}
}
void CConfigHtmlGenerator::write(const char* szData) {
	write((const uint8_t *)szData, strlen(szData));
}
void CConfigHtmlGenerator::chunk_sent(struct HttpdConnectionSlot* slot) {
	if (!m_lChunks.empty()) {
		httpd_slot_setsentcb(slot, &chunk_sent_callback, this);
		httpd_slot_send(slot, m_lChunks.front().pData, m_lChunks.front().nLen);
		m_lChunks.pop_front();
	} else {
		httpd_slot_setdone(slot);
		delete this;
	}
}
void CConfigHtmlGenerator::chunk_sent_callback(struct HttpdConnectionSlot* slot, void *pData) {
	((CConfigHtmlGenerator *)pData)->chunk_sent(slot);
}
