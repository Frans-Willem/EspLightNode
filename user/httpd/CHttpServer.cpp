#include "CHttpServer.h"
#include "CHttpRequest.h"

CHttpServer::CHttpServer(int nPort) : m_cParent(nPort) {
	m_cParent.addListener(this);
}

void CHttpServer::addListener(IHttpServerListener *pListener) {
	m_sListeners.insert(pListener);
}

void CHttpServer::removeListener(IHttpServerListener *pListener) {
	m_sListeners.erase(pListener);
}

void CHttpServer::onConnection(CTcpSocket *pSocket) {
	new CHttpRequest(this, pSocket);
}

void CHttpServer::onRequest(CHttpRequest *pRequest) {
	for (auto listener : m_sListeners)
		if (listener->onRequest(this, pRequest))
			return;
}
