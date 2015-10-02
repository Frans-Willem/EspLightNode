#ifndef CONFIG_CCONFIGPOSTHANDLER_H
#define CONFIG_CCONFIGPOSTHANDLER_H
#include "httpd/CHttpRequest.h"
#include "config/IConfigRunner.h"
#include <map>
#include <vector>

class CConfigWriter;
class CConfigPostHandler : IHttpRequestListener, IConfigRunner {
	public:
		static bool handle(CHttpRequest* pRequest);
	private:
		CConfigPostHandler(CHttpRequest* pRequest);
		~CConfigPostHandler();
		//IHttpRequestListener
		void onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue);
		void onHeadersDone(CHttpRequest *pRequest, size_t nDataLen);
		void onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nLength);
		void onDataDone(CHttpRequest *pRequest);
		void onSent(CHttpRequest *pRequest);
		void onDisconnected(CHttpRequest *pRequest);
		//IConfigRunner
		void beginModule(const char *szName, const char *szDescription);
		void endModule();
		void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault);
		void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault);
		void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault);
		void optionSelectBegin(const char *szName, const char *szDescription, unsigned int* pnValue, unsigned int nDefault);
		void optionSelectItem(const char *szName, unsigned int nValue);
		void optionSelectEnd();

		std::string createOptionKey(const char *szName);
		

		CHttpRequest *m_pRequest;
		std::map<std::string,std::string> m_mValues;
		CConfigWriter* m_pWriter;
		std::string m_szOutput;
		std::list<const char *> m_lSections;

		size_t m_nEscapeLeft;
		char m_cEscape;
		std::vector<char> m_vBuffer;
		std::string m_szKey;
		bool m_bHasKey;
};
#endif//CONFIG_CCONFIGPOSTHANDLER_H
