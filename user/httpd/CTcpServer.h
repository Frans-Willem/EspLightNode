#ifndef HTTPD_CTCPSERVER_H
#define HTTPD_CTCPSERVER_H
#include <sdkfixup.h>
extern "C" {
#include <ip_addr.h>
#include <espconn.h>
}
#include <stdint.h>
#include <set>

class ITcpServerListener;
class CTcpSocket;
class CTcpServer {
	public:
		CTcpServer(int nPort);
		~CTcpServer();
		void fixConnectionParams();
		void addListener(ITcpServerListener *pListener);
		void removeListener(ITcpServerListener *pListener);
		void setTimeout(unsigned int nTimeout);
	private:
		struct espconn m_conn;
		esp_tcp m_tcp;
		std::set<ITcpServerListener*> m_sListeners;

		static void connect_callback(void *arg);
		static void reconnect_callback(void *arg, sint8);
		static void disconnect_callback(void *arg);
};

class ITcpServerListener {
	public:
		virtual void onConnection(CTcpSocket *pSocket) = 0;
};
#endif//HTTPD_CTCPSERVER_H
