#include <sdkfixup.h>
#include "CTcpServer.h"
#include <string.h>
#include <cstddef>
extern "C" {
#include <osapi.h>
}
#include "CTcpSocket.h"

CTcpServer::CTcpServer(int nPort) {
	memset(&m_conn, 0, sizeof(struct espconn));	
	memset(&m_tcp, 0, sizeof(esp_tcp));	
	m_conn.type = ESPCONN_TCP;
	m_conn.state = ESPCONN_NONE;
	m_conn.proto.tcp = &m_tcp;
	m_tcp.local_port = nPort;
	fixConnectionParams();
	espconn_accept(&m_conn);
}

CTcpServer::~CTcpServer() {
	// Maybe espconn_disconnect is better? unsure about this.
	espconn_delete(&m_conn);
}

void CTcpServer::fixConnectionParams() {
	// On connect, the server properties are copied to the client connection
	// However, on disconnect and reconnect, the properties are copied from client to server.
	// This function should be called from those callbacks to fix things up.
	m_conn.reverse = (void*)this;
	espconn_regist_connectcb(&m_conn, connect_callback);
	espconn_regist_disconcb(&m_conn, disconnect_callback);
	//Somehow register_reconcb isn't always defined
	m_tcp.reconnect_callback = reconnect_callback; 
}

void CTcpServer::addListener(ITcpServerListener *pListener) {
	m_sListeners.insert(pListener);
}

void CTcpServer::removeListener(ITcpServerListener *pListener) {
	m_sListeners.erase(pListener);
}

void CTcpServer::connect_callback(void *arg) {
	// arg points to the new connection,
	// but "reverse" should be copied along.
	// So at this point, we can use that to get a pointer to our CTcpServer,
	// And then change it to a CHttpConnection
	struct espconn *conn = (struct espconn *)arg;
	CTcpServer *pServer = (CTcpServer *)conn->reverse;
	pServer->fixConnectionParams(); //Just in case
	CTcpSocket *pSocket = new CTcpSocket(pServer, conn);
	for (auto listener : pServer->m_sListeners)
		listener->onConnection(pSocket);
	pSocket->release();
}

void CTcpServer::disconnect_callback(void *arg) {
	struct espconn *conn = (struct espconn *)arg;
	//Note:
	// this assumes the ->reverse of the client still points to the server.
	// If you change ->reverse of the client, make sure to set different connect/reconnect/disconnect callbacks too!
	// And be sure to call fixConnectionParams() on the server just after extracting the needed ->reverse
	CTcpServer *pServer = (CTcpServer *)conn->reverse;
	pServer->fixConnectionParams();
}

void CTcpServer::reconnect_callback(void *arg, sint8) {
	struct espconn *conn = (struct espconn *)arg;
	//Note:
	// this assumes the ->reverse of the client still points to the server.
	// If you change ->reverse of the client, make sure to set different connect/reconnect/disconnect callbacks too!
	// And be sure to call fixConnectionParams() on the server just after extracting the needed ->reverse
	CTcpServer *pServer = (CTcpServer *)conn->reverse;
	pServer->fixConnectionParams();
}
