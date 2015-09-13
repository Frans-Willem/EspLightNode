#include "CHttpRequest.h"
#include "CHttpServer.h"
#include <string.h>
extern "C" {
#include <osapi.h>
}
#include <vector>

CHttpRequest::CHttpRequest(CHttpServer *pOwner, CTcpSocket *pSocket) {
	m_pOwner = pOwner;
	m_pSocket = pSocket;
	m_pSocket->addListener(this);
	m_nRef = 1;
}
CHttpRequest::~CHttpRequest() {
	m_pSocket->removeListener(this);
}

void CHttpRequest::addRef() {
	m_nRef++;
}

void CHttpRequest::release() {
	m_nRef--;
	//TODO: delete
}

void CHttpRequest::addListener(IHttpRequestListener *pListener) {
	if (m_sListeners.insert(pListener).second)
		addRef();
}

void CHttpRequest::removeListener(IHttpRequestListener *pListener) {
	if (m_sListeners.erase(pListener) > 0)
		release();
}

CTcpSocket *CHttpRequest::getSocket() {
	return m_pSocket;
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
				if (m_nDataLeft == 0) {
					for (auto listener : m_sListeners)
						listener->onDataDone(this);
				}
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
	for (auto listener : m_sListeners)
		listener->onDisconnected(this);
	delete this;
}

void CHttpRequest::onSocketSent(CTcpSocket *pSocket) {
	if (m_bHadError) {
		//TODO: Close the socket?
	}
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
					for (auto listener : m_sListeners)
						listener->onDataDone(this);
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

		size_t nUriLength = nSpaces[1] - nSpaces[0];
		if (nUriLength == 0) {
			onError(400, "Bad Request", "Empty URI");
			return false;
		}
		m_szUri = std::string(&szHeader[nSpaces[0]], nUriLength);
		//Trigger onRequest on the server
		m_pOwner->onRequest(this);
		return true;
	} else if (nLength == 0) {
		m_bHeadersDone = true;
		for (auto listener : m_sListeners)
			listener->onHeadersDone(this, m_nDataLeft);
		if (m_nDataLeft == 0)
			for (auto listener : m_sListeners)
				listener->onDataDone(this);
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
	onError(400,"Bad Request","Unknown error");
	return false;
}

void CHttpRequest::onError(unsigned int nCode, const char *szType, const char *szDescription) {
	if (m_bHadError)
		return;
	char szTemp[512];
	m_bHadError = true;
	os_sprintf(szTemp,"HTTP/1.0 %d %s\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s",nCode,szType,strlen(szDescription),szDescription);
	m_pSocket->send((uint8_t *)szTemp,strlen(szTemp));
}
