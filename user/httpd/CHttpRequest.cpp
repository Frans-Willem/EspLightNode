#include <sdkfixup.h>
#include "CHttpRequest.h"
#include "CHttpServer.h"
#include <string.h>
extern "C" {
#include <osapi.h>
}
#include <vector>
#include "debug/CDebugServer.h"

CHttpRequest::CHttpRequest(CHttpServer *pOwner, CTcpSocket *pSocket) {
	m_pOwner = pOwner;
	m_pSocket = pSocket;
	m_pSocket->addListener(this);
	m_nRef = 1;
	DEBUG("CHttpRequest::CHttpRequest(); = %08X", this);
}
CHttpRequest::~CHttpRequest() {
	if (m_pSocket)
		m_pSocket->removeListener(this);
	DEBUG("CHttpRequest::~CHttpRequest(%08X);", this);
}

void CHttpRequest::addRef() {
	m_nRef++;
}

void CHttpRequest::release() {
	if (--m_nRef == 0)
		delete this;
}

void CHttpRequest::addListener(IHttpRequestListener *pListener) {
	if (m_sListeners.insert(pListener).second)
		addRef();
}

void CHttpRequest::removeListener(IHttpRequestListener *pListener) {
	if (m_sListeners.erase(pListener) > 0)
		release();
}

std::string CHttpRequest::getUri() {
	return m_szUri;
}

void CHttpRequest::onSocketRecv(CTcpSocket *pSocket, const uint8_t *pData, size_t nLen) {
	if (m_bHadError)
		return;
	while (nLen) {
		if (m_bHeadersDone) {
			nLen = std::min(nLen, m_nDataLeft);
			m_nDataLeft -= nLen;
			if (nLen > 0) {
				for (auto listener : m_sListeners)
					listener->onData(this, pData, nLen);
				if (m_nDataLeft == 0)
					dispatchDataDone();
			}
			return;
		}
		size_t nProcess = std::min(HTTP_BUFFER_SIZE - m_nBufferFilled, nLen);
		if (nProcess == 0) {
			onError(431,"Request Header Fields Too Large","Request Header Fields Too Large");
			return;
		}
		memcpy(&m_pBuffer[m_nBufferFilled], pData, nProcess);
		m_nBufferFilled += nProcess;
		nLen -= nProcess;
		pData = &pData[nProcess];
		if (!process()) {
			onError(400,"Bad Request","Unknown error occured by bad request");
		}
	}
}

void CHttpRequest::onSocketDisconnected(CTcpSocket *pSocket) {
	DEBUG("CHttpRequest::onSocketDisconnected(%08X)",this);
	std::set<IHttpRequestListener*> sCopy(m_sListeners);
	//Remove all listeners at this point.
	m_sListeners.clear();
	addRef();
	for (auto listener : sCopy) {
		listener->onDisconnected(this);
		release();
	}

	if (m_pSocket) {
		m_pSocket=(CTcpSocket*)NULL;
		release();
	}
	release();
}

void CHttpRequest::onSocketSent(CTcpSocket *pSocket) {
	if (m_bHadError) {
		pSocket->disconnect(true);
	}
	for (auto listener : m_sListeners)
		listener->onSent(this);
}

bool CHttpRequest::process() {
	if (m_bHadError)
		return false;
	size_t nProcessed, nOldProcessed;
	nProcessed = 0;
	do {
		nOldProcessed = nProcessed;
		
		if (m_bHeadersDone) {
			if (m_nDataLeft > 0) {
				size_t nProcess = std::min(m_nBufferFilled - nProcessed, m_nDataLeft);
				m_nDataLeft = nProcess;
				for (auto listener : m_sListeners)
					listener->onData(this, &m_pBuffer[nProcessed], nProcess);
				if (m_nDataLeft == 0)
					dispatchDataDone();
			}
			return true;
		}
		size_t nEndLine=nProcessed;
		while (m_pBuffer[nEndLine] != '\n' && nEndLine < m_nBufferFilled)
		       nEndLine++;
		if (nEndLine < m_nBufferFilled) {
			size_t nNextLine = nEndLine+1;
			if (nEndLine > 0 && m_pBuffer[nEndLine-1] == '\r')
				nEndLine--;
			m_pBuffer[nEndLine]='\0';
			if (!processHeader((char *)&m_pBuffer[nProcessed], nEndLine-nProcessed)) {
				onError(400, "Bad Request", "Bad Request");
				return false;
			}
			nProcessed = nNextLine;
		}
	} while (nOldProcessed != nProcessed);
	//Save rest
	m_nBufferFilled -= nProcessed;
	memmove(m_pBuffer, &m_pBuffer[nProcessed], m_nBufferFilled);
	return true;
}

bool CHttpRequest::processHeader(char *szHeader, size_t nLength) {
	if (m_bHadError)
		return false;
	if (m_szUri.empty()) {
		size_t nSpaces[4];
		size_t nNumSpaces=0;
		for (size_t i=0; i < nLength && nNumSpaces < 3; i++) {
			if (szHeader[i] == ' ')
				nSpaces[nNumSpaces++] = i;
		}
		// Set end as last "space" to easy calculation
		nSpaces[nNumSpaces] = nLength;
		if (nNumSpaces < 2) {
			onError(400, "Bad Request", "Invalid number of parts to VERB");
			return false;
		}
		if (nSpaces[0] == 3 && memcmp(szHeader,"GET",3)==0) {
			m_vVerb = VERB_GET;
		} else if (nSpaces[0] == 4 && memcmp(szHeader,"POST",4)==0) {
			m_vVerb = VERB_POST;
		} else {
			onError(400, "Bad Request", "Invalid verb");
			return false;
		}

		size_t nUriLength = (nSpaces[1] - nSpaces[0])-1;
		if (nUriLength == 0) {
			onError(400, "Bad Request", "Empty URI");
			return false;
		}
		m_szUri = std::string(&szHeader[nSpaces[0]+1], nUriLength);
		//Trigger onRequest on the server
		m_pOwner->onRequest(this);
		return true;
	} else if (nLength == 0) {
		m_bHeadersDone = true;
		for (auto listener : m_sListeners)
			listener->onHeadersDone(this, m_nDataLeft);
		if (m_nDataLeft == 0)
			dispatchDataDone();
		return true;
	} else {
		size_t nSplit;
		for (nSplit=0; szHeader[nSplit] != ':' && nSplit < nLength; nSplit++);
		const char *szName;
		const char *szValue;
		if (nSplit == nLength) {
			szName = szHeader;
			szValue = "";
		} else {
			szHeader[nSplit]='\0';
			size_t nValueStart = nSplit+1;
			while (nValueStart < nLength && szHeader[nValueStart] != ' ')
				nValueStart++;
			szName = szHeader;
			szValue = &szHeader[nValueStart];
		}
		for (auto listener : m_sListeners)
			listener->onHeader(this, szName, szValue);
		return true;
	}
	onError(400,"bad request","unknown error");
	return false;
}

void CHttpRequest::onError(unsigned int nCode, const char *szType, const char *szDescription) {
	if (m_bHadError)
		return;
	m_bHadError = true;
	startHeaders(nCode, szType);
	sendHeader("Content-Length", strlen(szDescription));
	sendHeader("Content-Type", "text/plain");
	sendData(szDescription);
	end(false);
}

bool CHttpRequest::startHeaders(unsigned int nCode, const char *szMessage) {
	if (m_bResponseStarted || !m_pSocket)
		return false;
	m_bResponseStarted = true;
	static char szTemp[512];
	ets_sprintf(szTemp,"HTTP/1.0 %d %s\r\n",nCode,szMessage);
	return m_pSocket->send((uint8_t *)szTemp, strlen(szTemp));
}

bool CHttpRequest::sendHeader(const char *szName, const char *szValue) {
	if (m_bHeadersSent || !m_bResponseStarted || !m_pSocket)
		return false;
	int nNameLen=strlen(szName);
	int nValueLen=strlen(szValue);
	char *szTemp = new char[nNameLen + 2 + nValueLen + 2];
	memcpy(szTemp,szName,nNameLen);
	memcpy(&szTemp[nNameLen],": ",2);
	memcpy(&szTemp[nNameLen+2],szValue,nValueLen);
	memcpy(&szTemp[nNameLen+2+nValueLen],"\r\n",2);
	bool bRetval = m_pSocket->send((uint8 *)szTemp, nNameLen+2+nValueLen+2);
	delete[] szTemp;
	return bRetval;
}
bool CHttpRequest::sendHeader(const char *szName, unsigned int nValue) {
	char szTemp[16];
	os_sprintf(szTemp,"%u", nValue);
	return sendHeader(szName, szTemp);
}

void CHttpRequest::endHeaders() {
	if (!m_pSocket || m_bHeadersSent)
		return;
	m_pSocket->send((const uint8_t *)"\r\n",2);
	m_bHeadersSent = true;
}

bool CHttpRequest::sendData(const uint8_t *pData, size_t nLength) {
	if (!m_bResponseStarted || !m_pSocket)
		return false;
	endHeaders();
	return m_pSocket->send(pData, nLength);
}

bool CHttpRequest::sendData(const char *szData) {
	return sendData((const uint8_t *)szData, strlen(szData));
}

void CHttpRequest::end(bool bForce) {
	if (m_pSocket)
		m_pSocket->disconnect(bForce);
}

void CHttpRequest::dispatchDataDone() {
	for (auto listener : m_sListeners)
		listener->onDataDone(this);
	if (m_sListeners.empty())
		onError(404,"File not found","File not found");
}
