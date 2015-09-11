#ifndef CCONFIGHTMLGENERATOR_H
#define CCONFIGHTMLGENERATOR_H
#include "IConfigRunner.h"
#include <list>

struct HttpdConnectionSlot;
class CConfigHtmlGenerator : public IConfigRunner {
	public:
//IConfigRunner
		void beginModule(const char *szName, const char *szDescription);
		void endModule();
		void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool bDefault);
		void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault);
		void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault);
//Only
		void write_header();
		void write_footer();
		void start_transfer(struct HttpdConnectionSlot* slot);
	private:
		struct Chunk {
			uint8_t pData[512];
			size_t nLen;
		};
		std::list<const char *> m_lCategories;
		size_t m_nTotalLen;
		std::list<Chunk> m_lChunks;

		void write_fieldprefix();
		void write(const uint8_t* pData, size_t nLen);
		void write(const char* szData);
		void chunk_sent(struct HttpdConnectionSlot* slot);
		static void chunk_sent_callback(struct HttpdConnectionSlot* slot, void *pData);
};
#endif//CCONFIGHTMLGENERATOR_H

