#ifndef DEBUG_CDEBUGSERVER_H
#define DEBUG_CDEBUGSERVER_H
#include "httpd/CTcpServer.h"
#include "httpd/CTcpSocket.h"
#include <set>

#define DEBUG(f,args...)	CDebugServer::get()->send("%s:%d: " f "\r\n", __FILE__, __LINE__, ##args)

class CDebugServer : ITcpServerListener, ITcpSocketListener {
	public:
		static CDebugServer* get();
		void send(const char *szFormat, ...);
	private:
		CDebugServer(int nPort);

		static CDebugServer *g_pInstance;
		CTcpServer m_cServer;
		std::set<CTcpSocket*> m_sSockets;
		unsigned int m_nLostConnections;

		// ITcpServerListener
		void onConnection(CTcpSocket *pSocket);

		// ITcpSocketListener
		void onSocketRecv(CTcpSocket *pSocket, const uint8_t *pData, size_t nLen);
		void onSocketDisconnected(CTcpSocket *pSocket);
		void onSocketSent(CTcpSocket *pSocket);
};
#endif//DEBUG_CDEBUGSERVER_H
