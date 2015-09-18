/*
 * config.c
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */
#include <sdkfixup.h>
extern "C" {
#include <mem.h>
#include <osapi.h>
}
#include "config.h"
#include <string.h>
#include <list>
#include "CConfigHtmlGenerator.h"
#include <sstream>
#include "httpd/CHttpRequest.h"

class CConfigRunnerDefault : public IConfigRunner {
void optionBool(const char *szName, const char *szDescription, bool *pbValue, bool pbDefault) {
	*pbValue = pbDefault;
}
void optionString(const char *szName, const char *szDescription, char *szValue, size_t nSize, const char *szDefault) {
	strncpy(szValue, szDefault, nSize-1);
	szValue[nSize-1]='\0';
}
void optionInt(const char *szName, const char *szDescription, void *pValue, size_t nSize, uint32_t nMin, uint32_t nMax, uint32_t nDefault) {
	switch (nSize) {
		case 1: *(uint8_t *)pValue = nDefault; break;
		case 2: *(uint16_t *)pValue = nDefault; break;
		case 4: *(uint32_t *)pValue = nDefault; break;
	}
}
};

DEFINE_CONFIG(artnet);
DEFINE_CONFIG(tpm2net);

void config_run(IConfigRunner *runner) {
	artnet_runconfig(runner);
	tpm2net_runconfig(runner);
}

void config_load() {
	CConfigRunnerDefault cRunner;
	config_run(&cRunner);
}

void config_html(CHttpRequest *pRequest) {
	CConfigHtmlGenerator::handle(pRequest);
}

class CBufferStream {
	public:
		CBufferStream(size_t nBlockSize) {
			m_nLastLength = m_nBlockSize = nBlockSize;
		}
		~CBufferStream() {
			for (auto block : m_lBuffers)
				delete[] block;
		}
		CBufferStream& operator<<(const char *szData) {
			write((const uint8_t *)szData, strlen(szData));
			return *this;
		}
		CBufferStream& operator<<(int nNumber) {
			char szTemp[16];
			os_sprintf(szTemp,"%d", nNumber);
			write((const uint8_t *)szTemp, strlen(szTemp));
			return *this;
		}
		size_t getLength() {
			if (m_lBuffers.empty())
				return 0;
			return ((m_lBuffers.size() - 1) * m_nBlockSize) + m_nLastLength;
		}
		void copyTo(uint8_t *pData, size_t nSize) {
			for (auto block : m_lBuffers) {
				size_t nBlockSize = (block == m_lBuffers.back()) ? m_nLastLength : m_nBlockSize;
				size_t nCopy = std::min(nSize, nBlockSize);
				memcpy(pData, block, nCopy);
				nSize -= nCopy;
				pData = &pData[nCopy];
			}
		}
	private:
		void write(const uint8_t *pData, size_t nSize) {
			while (nSize) {
				if (m_nLastLength >= m_nBlockSize) {
					m_lBuffers.push_back(new uint8_t[m_nBlockSize]);
					m_nLastLength = 0;
				}
				size_t nCurrent = std::min(nSize, m_nBlockSize-m_nLastLength);
				memcpy(&m_lBuffers.back()[m_nLastLength], pData, nCurrent);
				m_nLastLength += nCurrent;
				nSize -= nCurrent;
				pData = &pData[nCurrent];
			}
		}

		std::list<uint8_t *> m_lBuffers;
		size_t m_nLastLength;
		size_t m_nBlockSize;
};

void config_submit(struct HttpdConnectionSlot *slot) {
	CBufferStream colData(64), colHeaders(64);
	colData << "Hello world!<br />";
	colHeaders << "200 HTTP/1.0 OK\r\n";
	colHeaders << "Content-Type: text/html\r\n";
	colHeaders << "Content-Length: " << colData.getLength() << "\r\n\r\n";
	uint8_t *pAll = new uint8_t[colHeaders.getLength() + colData.getLength()];
	colHeaders.copyTo(pAll, colHeaders.getLength());
	colData.copyTo(&pAll[colHeaders.getLength()], colData.getLength());

	//httpd_slot_send(slot, pAll, colHeaders.getLength() + colData.getLength());
}
