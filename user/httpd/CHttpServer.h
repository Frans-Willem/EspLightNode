#ifndef HTTPD_CHTTPSERVER_H
#define HTTPD_CHTTPSERVER_H
#include "CTcpServer.h"
#include <set>

class CHttpRequest;
class IHttpServerListener;
class CHttpServer : ITcpServerListener {
	public:
		CHttpServer(int nPort);
		void addListener(IHttpServerListener *pListener);
		void removeListener(IHttpServerListener *pListener);
	protected:
		friend class CHttpRequest;
		void onRequest(CHttpRequest *pRequest);
	private:
		void onConnection(CTcpSocket *pSocket);

		CTcpServer m_cParent;
		std::set<IHttpServerListener*> m_sListeners;
};

class IHttpServerListener {
	public:
		virtual ~IHttpServerListener() {};
		virtual void onRequest(CHttpServer *pServer, CHttpRequest *pRequest) = 0;
};
#endif//HTTPD_CHTTPSERVER_H
