#ifndef HTTPD_CTCPSOCKET_H
#define HTTPD_CTCPSOCKET_H
#include <sdkfixup.h>
extern "C" {
	#include <ip_addr.h>
	#include <espconn.h>
}
#include <set>
#include <list>

class CTcpServer;
class ITcpSocketListener;
class CTcpSocket {
	public:
		CTcpSocket(CTcpServer *pServer, struct espconn *conn);
		void addRef();
		void release();
		void addListener(ITcpSocketListener *pListener);
		void removeListener(ITcpSocketListener *pListener);
		bool send(const uint8_t *pData, size_t nLen);
	private:
		~CTcpSocket();
		void setupConnectionParams();

		unsigned int m_nRef;
		CTcpServer *m_pServer;
		struct espconn *m_conn;
		std::set<ITcpSocketListener*> m_sListeners;
		std::list<std::pair<uint8_t*,size_t>> m_lBacklog;

		static void connect_callback(void *arg);
		static void reconnect_callback(void *arg, sint8);
		static void disconnect_callback(void *arg);
		static void recv_callback(void *arg, char *pData, unsigned short nLen);
		static void sent_callback(void *arg);
};
class ITcpSocketListener {
	public:
		virtual void onSocketRecv(CTcpSocket *pSocket, const uint8_t *pData, size_t nLen) = 0;
		virtual void onSocketDisconnected(CTcpSocket *pSocket) = 0;
		virtual void onSocketSent(CTcpSocket *pSocket) = 0;
		virtual ~ITcpSocketListener() {};
};
#endif//HTTPD_CTCPSOCKET_H
