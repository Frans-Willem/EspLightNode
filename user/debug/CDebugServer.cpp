#include <sdkfixup.h>
#include <stdarg.h>
#include "CDebugServer.h"
#include <string.h>

CDebugServer* CDebugServer::g_pInstance = 0;

CDebugServer* CDebugServer::get() {	
	if (!g_pInstance)
		g_pInstance = new CDebugServer(8888);
	return g_pInstance;
}

CDebugServer::CDebugServer(int nPort) : m_cServer(nPort) {
	m_cServer.addListener(this);
	m_cServer.setTimeout(0);
}

void CDebugServer::send(const char *szFormat, ...) {
	static char szTemp[512];
	va_list args;
	__builtin_va_start(args, szFormat);
	ets_vsnprintf(szTemp, (sizeof(szTemp)/sizeof(szTemp[0])) -1, szFormat, args);
	__builtin_va_end(args);
	for (auto s : m_sSockets)
		s->send((const uint8_t *)szTemp, strlen(szTemp));
}

void CDebugServer::onConnection(CTcpSocket *pSocket) {
	m_sSockets.insert(pSocket);
	pSocket->addListener(this);
	pSocket->setTimeout(0);
	DEBUG("New connection (%08X)", pSocket);
	DEBUG("Lost connections: %d", m_nLostConnections);
}

void CDebugServer::onSocketRecv(CTcpSocket *pSocket, const uint8_t *pData, size_t nLen) {
	//Do nothing
}

void CDebugServer::onSocketDisconnected(CTcpSocket *pSocket) {
	m_sSockets.erase(pSocket);
	pSocket->removeListener(this);
	DEBUG("Lost connection (%08X)", pSocket);
	m_nLostConnections++;
}

void CDebugServer::onSocketSent(CTcpSocket *pSocket) {

}
