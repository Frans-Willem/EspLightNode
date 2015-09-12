#include "CTcpSocket.h"
#include "CTcpServer.h"

CTcpSocket::CTcpSocket(CTcpServer *pServer, struct espconn *conn) {
	m_pServer = pServer;
	m_conn = conn;
	m_nRef = 1;
	setupConnectionParams();
}
CTcpSocket::~CTcpSocket() {
	//If all is well no cleanup is needed at this point.
}

void CTcpSocket::setupConnectionParams() {
	if (!m_conn)
		return;
	m_conn->reverse = this;
	espconn_regist_disconcb(m_conn, disconnect_callback);
	espconn_regist_connectcb(m_conn, connect_callback);
	m_conn->proto.tcp->reconnect_callback = reconnect_callback;
	espconn_regist_recvcb(m_conn, recv_callback);
	espconn_regist_sentcb(m_conn, sent_callback);
}

void CTcpSocket::addListener(ITcpSocketListener *pListener) {
	if (m_sListeners.insert(pListener).second)
		addRef();
}
void CTcpSocket::removeListener(ITcpSocketListener *pListener) {
	if (m_sListeners.erase(pListener) > 0)
		release();
}

void CTcpSocket::addRef() {
	m_nRef++;
}
void CTcpSocket::release() {
	if (--m_nRef == 0) {
		if (!m_conn) {
			delete this;
		} else {
			espconn_disconnect(m_conn);
		}
	}
}

bool CTcpSocket::send(const uint8_t *pData, size_t nLen) {
	if (!m_conn)
		return false;
	return (ESPCONN_OK == espconn_sent(m_conn, (uint8_t *)pData, nLen));
}

void CTcpSocket::connect_callback(void *arg) {
	struct espconn *conn = (struct espconn *)arg;
	CTcpSocket *pSocket = (CTcpSocket *)conn->reverse;
	if (pSocket->m_conn != conn) {
		//Assume ESP SDK fuckup
		if (pSocket->m_pServer)
			pSocket->m_pServer->fixConnectionParams();
	}
}

void CTcpSocket::disconnect_callback(void *arg) {
	struct espconn *conn = (struct espconn *)arg;
	CTcpSocket *pSocket = (CTcpSocket *)conn->reverse;
	if (pSocket->m_conn != conn) {
		//Assume ESP SDK fuckup
		if (pSocket->m_pServer)
			pSocket->m_pServer->fixConnectionParams();
	}
	pSocket->m_conn = (struct espconn *)NULL;
	std::set<ITcpSocketListener*> sListeners(pSocket->m_sListeners);
	pSocket->addRef();
	pSocket->m_sListeners.clear();
	for (auto listener : sListeners) {
		listener->onSocketDisconnected(pSocket);
		pSocket->release();
	}
	//If this drops off to zero, the socket should be deallocated
	pSocket->release();
}
void CTcpSocket::reconnect_callback(void *arg, sint8) {
	struct espconn *conn = (struct espconn *)arg;
	CTcpSocket *pSocket = (CTcpSocket *)conn->reverse;
	if (pSocket->m_conn != conn) {
		//Assume ESP SDK fuckup
		if (pSocket->m_pServer)
			pSocket->m_pServer->fixConnectionParams();
	}
}
void CTcpSocket::recv_callback(void *arg, char *pData, unsigned short nLen) {
	struct espconn *conn = (struct espconn *)arg;
	CTcpSocket *pSocket = (CTcpSocket *)conn->reverse;
	for (auto listener : pSocket->m_sListeners)
		listener->onSocketRecv(pSocket, (const uint8_t *)pData, nLen);
}
void CTcpSocket::sent_callback(void *arg) {
	struct espconn *conn = (struct espconn *)arg;
	CTcpSocket *pSocket = (CTcpSocket *)conn->reverse;
	for (auto listener : pSocket->m_sListeners)
		listener->onSocketSent(pSocket);
}
