#ifndef CONFIG_CCONFIGPOSTHANDLER_H
#define CONFIG_CCONFIGPOSTHANDLER_H
#include "httpd/CHttpRequest.h"
#include <map>
#include <vector>

class CConfigPostHandler : IHttpRequestListener {
	public:
		static bool handle(CHttpRequest* pRequest);
	private:
		CConfigPostHandler(CHttpRequest* pRequest);
		//IHttpRequestListener
		void onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue);
		void onHeadersDone(CHttpRequest *pRequest, size_t nDataLen);
		void onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nLength);
		void onDataDone(CHttpRequest *pRequest);
		void onSent(CHttpRequest *pRequest);
		void onDisconnected(CHttpRequest *pRequest);

		CHttpRequest *m_pRequest;
		std::map<std::string,std::string> m_mValues;

		size_t m_nEscapeLeft;
		char m_cEscape;
		std::vector<char> m_vBuffer;
		std::string m_szKey;
		bool m_bHasKey;
};
#endif//CONFIG_CCONFIGPOSTHANDLER_H
