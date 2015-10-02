#include <sdkfixup.h>
#include "CConfigPostHandler.h"
#include <string.h>
#include "debug/CDebugServer.h"
#include "config/config.h"
#include "CConfigWriter.h"
#include "config/format.h"
extern "C" {
#include <user_interface.h>
}

bool CConfigPostHandler::handle(CHttpRequest* pRequest) {
	CConfigPostHandler *pHandler = new CConfigPostHandler(pRequest);
	pRequest->addListener(static_cast<IHttpRequestListener *>(pHandler));
	return true;
}

CConfigPostHandler::CConfigPostHandler(CHttpRequest* pRequest) {
	m_pRequest=pRequest;	
}

CConfigPostHandler::~CConfigPostHandler() {

}

void CConfigPostHandler::onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue) {
}

void CConfigPostHandler::onHeadersDone(CHttpRequest *pRequest, size_t nDataLength) {

}

void CConfigPostHandler::onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nLength) {
	for (size_t i = 0; i < nLength; i++) {
		if (m_nEscapeLeft) {
			m_nEscapeLeft--;
			m_cEscape <<= 4;
			if (pData[i] >= '0' && pData[i] <= '9')
				m_cEscape |= (pData[i] - '0');
			else if (pData[i] >= 'a' && pData[i] <= 'z')
				m_cEscape |= (pData[i] - 'a') + 10;
			else if (pData[i] >= 'A' && pData[i] <= 'Z')
				m_cEscape |= (pData[i] - 'A') + 10;
			if (m_nEscapeLeft == 0)
				m_vBuffer.push_back(m_cEscape);
		} else if (pData[i] == '=') {
			DEBUG("Equal sign");
			if (m_bHasKey) {
				m_vBuffer.push_back(pData[i]);
			} else {
				m_szKey = std::string(m_vBuffer.begin(), m_vBuffer.end());
				m_vBuffer.clear();
				m_bHasKey = true;
			}
		} else if (pData[i] == '&') {
			DEBUG("Ampersand");
			if (!m_bHasKey) {
				m_szKey=std::string(m_vBuffer.begin(), m_vBuffer.end());
				m_vBuffer.clear();
				m_bHasKey=true;
			}
			std::string szValue(m_vBuffer.begin(), m_vBuffer.end());
			DEBUG("Insert %s = %s", m_szKey.c_str(), szValue.c_str());
			m_mValues[m_szKey]=szValue;

			m_vBuffer.clear();
			m_bHasKey = false;
		} else if (pData[i] == '%') {
			m_cEscape = 0;
			m_nEscapeLeft = 2;
		} else if (pData[i] == '+') {
			m_vBuffer.push_back(' ');
		} else {
			m_vBuffer.push_back(pData[i]);
		}
	}
}

void CConfigPostHandler::onDataDone(CHttpRequest *pRequest) {
	//Handle last bit of data
	if (!m_bHasKey) {
		m_szKey=std::string(m_vBuffer.begin(), m_vBuffer.end());
		m_vBuffer.clear();
		m_bHasKey=true;
	}
	std::string szValue(m_vBuffer.begin(), m_vBuffer.end());
	m_mValues[m_szKey]=szValue;

	m_vBuffer.clear();
	m_bHasKey = false;

	// Start output
	m_szOutput = "<html><head>";
	m_szOutput += "<title>Saving...</title>";
	m_szOutput += "<meta http-equiv=\"refresh\" content=\"3;url=/\" />";
	m_szOutput += "</head><body>";

	// Write out config
	const uint8_t pHeader[] = CONFIG_HEADER;
	m_pWriter = new CConfigWriter(CONFIG_START_SECTOR, CONFIG_SECTOR_DIRECTION);
	m_pWriter->writeBytes(pHeader,sizeof(pHeader)); // Header
	config_run(this);
	m_pWriter->writeUInt(ConfigSectionEnd);
	m_pWriter->flush(true);
	delete m_pWriter;

	m_szOutput += "<p>Please wait for the unit to reboot...</p>";

	m_szOutput += "</body></html>";

	pRequest->startHeaders(200,"OK");
	pRequest->sendHeader("Content-Type", "text/html");
	pRequest->sendHeader("Content-Length", m_szOutput.length());
	pRequest->sendHeader("Refresh","3;url=/");
	pRequest->endHeaders();
	pRequest->sendData((const uint8_t *)m_szOutput.c_str(), m_szOutput.length());
}

void CConfigPostHandler::onSent(CHttpRequest *pRequest) {
	pRequest->end(false);
}

void CConfigPostHandler::onDisconnected(CHttpRequest *pRequest) {
	delete this;
	system_restart();
}

std::string CConfigPostHandler::createOptionKey(const char *szName) {
	std::string strRetval;
	for (auto section : m_lSections) {
		strRetval += section;
		strRetval += ".";
	}
	strRetval+=szName;
	return strRetval;
}

void CConfigPostHandler::beginModule(const char *szName, const char *szDescription) {
	m_pWriter->writeUInt(ConfigSectionStart);
	m_pWriter->writeString(szName);
	m_lSections.push_back(szName);
}

void CConfigPostHandler::endModule() {
	m_pWriter->writeUInt(ConfigSectionEnd);
	if (!m_lSections.empty())
		m_lSections.pop_back();
}

void CConfigPostHandler::optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault) {
	const auto found = m_mValues.find(createOptionKey(szName));
	bool bValue = (found != m_mValues.end() && found->second.compare("1") == 0);
	if (bValue != bDefault) {
		m_pWriter->writeUInt(ConfigInteger);
		m_pWriter->writeString(szName);
		m_pWriter->writeUInt(bValue?1:0);
	}
}

void CConfigPostHandler::optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
	const auto found = m_mValues.find(createOptionKey(szName));
	if (found != m_mValues.end() && found->second.compare(szDefault)!=0) {
		std::string capped(found->second);
		if (capped.length() + 1 > nSize)
			capped=capped.substr(0,nSize-1);
		m_pWriter->writeUInt(ConfigString);
		m_pWriter->writeString(szName);
		m_pWriter->writeString(capped.c_str());
	}
}

void CConfigPostHandler::optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
	auto found = m_mValues.find(createOptionKey(szName));
	if (found != m_mValues.end()) {
		uint32_t nValue = atoi(found->second.c_str());
		if (nValue < nMin)
			nValue = nMin;
		if (nValue > nMax)
			nValue = nMax;
		if (nValue != nDefault) {
			m_pWriter->writeUInt(ConfigInteger);
			m_pWriter->writeString(szName);
			m_pWriter->writeUInt(nValue);
		}
	}
}
void CConfigPostHandler::optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault) {
	auto found = m_mValues.find(createOptionKey(szName));
	if (found != m_mValues.end()) {
		unsigned int nValue = atoi(found->second.c_str());
		if (nValue != nDefault) {
			m_pWriter->writeUInt(ConfigInteger);
			m_pWriter->writeString(szName);
			m_pWriter->writeUInt(nValue);
		}
	}
}
void CConfigPostHandler::optionSelectItem(const char *szName, unsigned int nValue) {

}
void CConfigPostHandler::optionSelectEnd() {
}
