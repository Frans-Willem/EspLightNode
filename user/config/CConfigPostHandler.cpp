#include "CConfigPostHandler.h"
#include <string.h>
#include "debug/CDebugServer.h"

bool CConfigPostHandler::handle(CHttpRequest* pRequest) {
	pRequest->addListener(new CConfigPostHandler(pRequest));
	return true;
}

CConfigPostHandler::CConfigPostHandler(CHttpRequest* pRequest) {
	m_pRequest=pRequest;	
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
	DEBUG("onDataDone");
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

	std::vector<char> vData;
	
	//At this point m_mValues seems to be filled
	for (auto kv : m_mValues) {
		DEBUG("Key-value pair");
		vData.insert(vData.end(), kv.first.begin(), kv.first.end());
		vData.push_back(':');
		vData.push_back(' ');
		vData.insert(vData.end(), kv.second.begin(), kv.second.end());
		vData.push_back('\r');
		vData.push_back('\n');
	}
	vData.push_back('!');
	

	pRequest->startHeaders(200,"OK");
	pRequest->sendHeader("Content-Type", "text/plain");
	pRequest->sendHeader("Content-Length", vData.size());
	pRequest->endHeaders();
	pRequest->sendData((const uint8_t *)&vData[0], vData.size());
}

void CConfigPostHandler::onSent(CHttpRequest *pRequest) {
	pRequest->end(false);
}

void CConfigPostHandler::onDisconnected(CHttpRequest *pRequest) {
	delete this;
}
