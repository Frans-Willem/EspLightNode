#include <sdkfixup.h>
extern "C" {
#include <mem.h>
#include <osapi.h>
}
#include "CConfigHtmlGenerator.h"
#include "httpd/CHttpRequest.h"
#include "config/config.h"

void CConfigHtmlGenerator::handle(CHttpRequest *pRequest) {
	pRequest->addListener(new CConfigHtmlGenerator(pRequest));
}

CConfigHtmlGenerator::CConfigHtmlGenerator(CHttpRequest *pRequest) {
	m_pRequest = pRequest;
}

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
void CConfigHtmlGenerator::optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault) {
	write(szDescription);
	write(": <select name=\"");
	write_fieldprefix();
	write(szName);
	write("\">");
	m_nCurrentSelectItem = *pnValue;
}
void CConfigHtmlGenerator::optionSelectItem(const char *szName, unsigned int nValue) {
	char szTemp[32];
	write("<option value=\"");
	os_sprintf(szTemp, "%u", nValue);
	write(szTemp);
	if (nValue == m_nCurrentSelectItem)
		write("\" selected>");
	else
		write("\">");
	write(szName);
	write("</option>");
}
void CConfigHtmlGenerator::optionSelectEnd() {
	write("</select><br />");
}
void CConfigHtmlGenerator::optionIpAddress(const char *szName, const char *szDescription, uint32_t *pAddress, uint32_t nDefault) {
	char szTemp[32];
	write(szDescription);
	write(": ");
	for (unsigned int i=0; i<4; i++) {
		if (i != 0)
			write(".");
		write("<input type=\"text\" size=\"3\" name=\"");
		write_fieldprefix();
		write(szName);
		write("[");
		os_sprintf(szTemp,"%u", i);
		write(szTemp);
		write("]\" value=\"");
		os_sprintf(szTemp,"%u",(*pAddress >> (i*8)) & 0xFF);
		write(szTemp);
		write("\"></input>");
	}
	write("<br />");
}
void CConfigHtmlGenerator::onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue) {
}
void CConfigHtmlGenerator::onHeadersDone(CHttpRequest *pRequest, size_t nDataLength) {
}
void CConfigHtmlGenerator::onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nLength) {
}
void CConfigHtmlGenerator::onDataDone(CHttpRequest *pRequest) {
	write_header();
	config_run(this);
	write_footer();
	pRequest->startHeaders(200, "OK");
	pRequest->sendHeader("Content-Type", "text/html");
	pRequest->sendHeader("Content-Length", m_nTotalLen);
	pRequest->endHeaders();
	//Actual data will be sent in onSent
}
void CConfigHtmlGenerator::onSent(CHttpRequest *pRequest) {
	if (m_lChunks.empty()) {
		pRequest->end(false);
	} else {
		pRequest->sendData(m_lChunks.front().pData, m_lChunks.front().nLen);
		m_lChunks.pop_front();
	}
}
void CConfigHtmlGenerator::onDisconnected(CHttpRequest *pRequest) {
	delete this;
}
//Only
void CConfigHtmlGenerator::write_header() {
	write("<html><head><title>EspLightNode Configuration</title></head><body><h1>EspLightNode Configuration</h1>");
	write("<form method=\"POST\" action=\"/submit.cgi\">");
}
void CConfigHtmlGenerator::write_footer() {
	write("<input type=\"submit\" value=\"Save\"></input></form>");
	write("</body></html>");
}
void CConfigHtmlGenerator::start_transfer(CHttpRequest *pRequest) {
	pRequest->startHeaders(200, "OK");
	pRequest->sendHeader("Content-Type", "text/html");
	pRequest->sendHeader("Content-Length", m_nTotalLen);
	for (auto chunk : m_lChunks)
		pRequest->sendData(chunk.pData, chunk.nLen);
	pRequest->end(false);
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
