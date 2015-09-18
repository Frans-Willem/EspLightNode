#ifndef HTTPD_CHTTPREQUEST_H
#define HTTPD_CHTTPREQUEST_H
#include "CTcpSocket.h"
#include <string>
#include <map>

#define HTTP_BUFFER_SIZE 512
class CHttpServer;
class IHttpRequestListener;
class CHttpRequest : ITcpSocketListener {
	public:
		CHttpRequest(CHttpServer *pOwner, CTcpSocket *pSocket);
		void addRef();
		void release();
		void addListener(IHttpRequestListener *pListener);
		void removeListener(IHttpRequestListener *pListener);
		std::string getUri();
		bool startHeaders(unsigned int nCode, const char *szMessage);
		bool sendHeader(const char *szName, const char *szValue);
		bool sendHeader(const char *szName, unsigned int nValue);
		void endHeaders();
		bool sendData(const uint8_t *pData, size_t nLength);
		bool sendData(const char *szData);
		void end(bool bForce);
	private:
		~CHttpRequest();
		void onSocketRecv(CTcpSocket *pSocket, const uint8_t *pData, size_t nLen);
		void onSocketDisconnected(CTcpSocket *pSocket);
		void onSocketSent(CTcpSocket *pSocket);

		void dispatchDataDone();

		bool process();
		//szHeader[nLength] = '\0';
		bool processHeader(char *szHeader, size_t nLength);
		void onError(unsigned int nCode, const char *szType, const char *szDescription);

		enum Verb {
			VERB_GET,
			VERB_POST
		};

		CHttpServer *m_pOwner;
		CTcpSocket *m_pSocket;
		std::set<IHttpRequestListener*> m_sListeners;
		//m_bHadError: if this is true, an error occured somewhere, and the socket should be closed ASAP.
		bool m_bHadError;
		unsigned int m_nRef;
		uint8_t m_pBuffer[HTTP_BUFFER_SIZE];
		size_t m_nBufferFilled;
		Verb m_vVerb;
		std::string m_szUri;
		bool m_bHeadersDone;
		size_t m_nDataLeft;
		bool m_bResponseStarted;
		bool m_bHeadersSent;
};

class IHttpRequestListener {
	public:
		virtual ~IHttpRequestListener() {};
		virtual void onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue) = 0;
		virtual void onHeadersDone(CHttpRequest *pRequest, size_t nDataLength) = 0;
		virtual void onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nData) = 0;
		virtual void onDataDone(CHttpRequest *pRequest) = 0;
		virtual void onSent(CHttpRequest *pRequest) = 0;
		virtual void onDisconnected(CHttpRequest *pRequest) = 0;
};
#endif//HTTPD_CHTTPREQUEST_H
