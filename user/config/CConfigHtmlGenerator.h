#ifndef CCONFIGHTMLGENERATOR_H
#define CCONFIGHTMLGENERATOR_H
#include "IConfigRunner.h"
#include "httpd/CHttpRequest.h"
#include <list>

class CHttpRequest;
class CConfigHtmlGenerator : public IConfigRunner, IHttpRequestListener {
	public:
		static void handle(CHttpRequest *pRequest);
//IConfigRunner
		void beginModule(const char *szName, const char *szDescription);
		void endModule();
		void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault);
		void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault);
		void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault);
//IHttpRequestListener
		void onHeader(CHttpRequest *pRequest, const char *szName, const char *szValue);
		void onHeadersDone(CHttpRequest *pRequest, size_t nDataLength);
		void onData(CHttpRequest *pRequest, const uint8_t *pData, size_t nData);
		void onDataDone(CHttpRequest *pRequest);
		void onSent(CHttpRequest *pRequest);
		void onDisconnected(CHttpRequest *pRequest);

//Only
	private:
		CHttpRequest *m_pRequest;
		struct Chunk {
			uint8_t pData[512];
			size_t nLen;
		};
		std::list<const char *> m_lCategories;
		size_t m_nTotalLen;
		std::list<Chunk> m_lChunks;

		CConfigHtmlGenerator(CHttpRequest *pRequest);
		void write_header();
		void write_footer();
		void start_transfer(CHttpRequest *pRequest);
		void write_fieldprefix();
		void write(const uint8_t* pData, size_t nLen);
		void write(const char* szData);
};
#endif//CCONFIGHTMLGENERATOR_H

