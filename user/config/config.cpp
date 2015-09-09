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
#include "httpd.h"

void config_load_default_bool(struct ConfigRunner *runner, const char* name, const char* description, uint8_t *ptr, uint8_t def) {
	*ptr=def;
}

void config_load_default_string(struct ConfigRunner *runner, const char* name, const char* description, char *ptr, uint16_t len, const char *def) {
	strncpy(ptr,def,len-1);
	ptr[len-1]='\0';
}

void config_load_default_int(struct ConfigRunner *runner, const char *name, const char *description, void *ptr, uint8_t size, uint32_t minvalue, uint32_t maxvalue, uint32_t defvalue) {
	switch (size) {
	case 1: *(uint8_t *)ptr = defvalue; break;
	case 2: *(uint16_t *)ptr = defvalue; break;
	case 4: *(uint32_t *)ptr = defvalue; break;
	}
}

DEFINE_CONFIG(artnet);
DEFINE_CONFIG(tpm2net);

void config_run(struct ConfigRunner *runner) {
	artnet_runconfig(runner);
	tpm2net_runconfig(runner);
}

void config_load() {
	struct ConfigRunner runner;
	memset(&runner,0,sizeof(struct ConfigRunner));
	runner.booloption = config_load_default_bool;
	runner.stringoption = config_load_default_string;
	runner.intoption = config_load_default_int;

	config_run(&runner);
}

struct ConfigRunnerCategory {
	const char *szCategoryName;
	struct ConfigRunnerCategory *next;
	struct ConfigRunnerCategory *prev;
};

struct ConfigRunnerHtml {
	struct ConfigRunner base;
	struct ConfigRunnerHtmlChunk *first;
	struct ConfigRunnerHtmlChunk *current;
	struct ConfigRunnerCategory *catFirst;
	struct ConfigRunnerCategory *catLast;
	uint16_t len;
};

#define CONFIGHTMLCHUNKSIZE	512

struct ConfigRunnerHtmlChunk {
	uint8_t data[CONFIGHTMLCHUNKSIZE];
	uint16_t len;
	struct ConfigRunnerHtmlChunk *next;
};

void config_html_write(struct ConfigRunnerHtml *runner, const uint8_t *data, uint16_t len) {
	struct ConfigRunnerHtmlChunk *next;
	uint16_t copy;
	while (len && runner->current) {
		if (runner->current->len >= CONFIGHTMLCHUNKSIZE) {
			next = (struct ConfigRunnerHtmlChunk *)os_zalloc(sizeof(struct ConfigRunnerHtmlChunk));
			if (next == NULL) {
				runner->current = NULL;
				break;
			}
			next->len = 0;
			next->next = NULL;
			runner->current->next = next;
			runner->current = next;
		}
		copy = CONFIGHTMLCHUNKSIZE - runner->current->len;
		if (copy > len) copy = len;
		memcpy(&runner->current->data[runner->current->len], data, copy);
		runner->current->len += copy;
		runner->len += copy;
		len -= copy;
		data += copy;
	}
}

void config_html_write_string(struct ConfigRunnerHtml *runner, const char *str) {
	config_html_write(runner, (const uint8_t *)str, strlen(str));
}

void config_html_beginmodule(struct ConfigRunner *runner, const char *name, const char *description) {
	struct ConfigRunnerHtml *htmlrunner = (struct ConfigRunnerHtml *)runner;
	config_html_write_string(htmlrunner, "<fieldset><legend>");
	config_html_write_string(htmlrunner, description);
	config_html_write_string(htmlrunner, "</legend>");

	struct ConfigRunnerCategory *cat = (struct ConfigRunnerCategory *)os_zalloc(sizeof(struct ConfigRunnerCategory));
	cat->szCategoryName = name;
	cat->prev = htmlrunner->catLast;
	cat->next = NULL;
	if (cat->prev)
	       cat->prev->next = cat;
	htmlrunner->catLast = cat;
	if (htmlrunner->catFirst == NULL)
		htmlrunner->catFirst = cat;
}

void config_html_endmodule(struct ConfigRunner *runner) {
	struct ConfigRunnerHtml *htmlrunner = (struct ConfigRunnerHtml *)runner;
	config_html_write_string(htmlrunner, "</fieldset>");
	struct ConfigRunnerCategory *cat = htmlrunner->catLast;
	htmlrunner->catLast = cat->prev;
	if (cat->prev)
		cat->prev->next = NULL;
	if (htmlrunner->catFirst == cat)
		htmlrunner->catFirst = NULL;
}
void config_html_write_fieldprefix(struct ConfigRunnerHtml *runner) {
	for (struct ConfigRunnerCategory *cat = runner->catFirst; cat != NULL; cat = cat->next) {
		config_html_write_string(runner, cat->szCategoryName);
		config_html_write_string(runner,".");
	}
}
void config_html_booloption(struct ConfigRunner *runner, const char *name, const char *description, uint8_t *ptrvalue, uint8_t defvalue) {
	struct ConfigRunnerHtml *htmlrunner = (struct ConfigRunnerHtml *)runner;
	config_html_write_string(htmlrunner, description);
	config_html_write_string(htmlrunner, ": <input type=\"checkbox\" name=\"");
	config_html_write_fieldprefix(htmlrunner);
	config_html_write_string(htmlrunner, name);
	config_html_write_string(htmlrunner, "\" value=\"1\"");
	if (*ptrvalue)
		config_html_write_string(htmlrunner," checked");
	config_html_write_string(htmlrunner, "></input><br />");
}
void config_html_stringoption(struct ConfigRunner *runner, const char *name, const char *description, char *ptrvalue, uint16_t len, const char *defvalue) {
	struct ConfigRunnerHtml *htmlrunner = (struct ConfigRunnerHtml *)runner;
	config_html_write_string(htmlrunner, description);
	config_html_write_string(htmlrunner, ": <input type=\"text\" name=\"");
	config_html_write_fieldprefix(htmlrunner);
	config_html_write_string(htmlrunner, name);
	config_html_write_string(htmlrunner, "\" value=\"");
	config_html_write_string(htmlrunner, ptrvalue);
	config_html_write_string(htmlrunner, "\"></input><br />");
}
void config_html_intoption(struct ConfigRunner *runner, const char *name, const char *description, void *ptrvalue, uint8_t size, uint32_t minvalue, uint32_t maxvalue, uint32_t defvalue) {
	char szTemp[32];
	struct ConfigRunnerHtml *htmlrunner = (struct ConfigRunnerHtml *)runner;
	config_html_write_string(htmlrunner, description);
	config_html_write_string(htmlrunner, ": <input type=\"text\" name=\"");
	config_html_write_fieldprefix(htmlrunner);
	config_html_write_string(htmlrunner, name);
	config_html_write_string(htmlrunner, "\" value=\"");
	switch (size) {
		case 1: os_sprintf(szTemp,"%d",*(uint8_t *)ptrvalue); break;
		case 2: os_sprintf(szTemp,"%d",*(uint16_t *)ptrvalue); break;
		case 4: os_sprintf(szTemp,"%d",*(uint32_t *)ptrvalue); break;
		default: szTemp[0]='\0'; break;
	}
	config_html_write_string(htmlrunner, szTemp);
	config_html_write_string(htmlrunner, "\"></input><br />");
}

void config_html_sendchunk(struct HttpdConnectionSlot *slot, void *data) {
	struct ConfigRunnerHtmlChunk *chunk = (struct ConfigRunnerHtmlChunk *)data;
	httpd_slot_send(slot,chunk->data,chunk->len);
	if (chunk->next) {
		httpd_slot_setsentcb(slot, config_html_sendchunk, (void *)chunk->next);
	} else {
		httpd_slot_setdone(slot);
	}
	os_free((void*)chunk);
}

void config_html(struct HttpdConnectionSlot *slot) {
	struct ConfigRunnerHtmlChunk *firstchunk = (struct ConfigRunnerHtmlChunk *)os_zalloc(sizeof(struct ConfigRunnerHtmlChunk));
	char headers[256];
	firstchunk->len = 0;
	firstchunk->next = NULL;

	struct ConfigRunnerHtml runner;
	memset(&runner, 0, sizeof(struct ConfigRunnerHtml));
	runner.first = firstchunk;
	runner.current = firstchunk;
	runner.len = 0;
	runner.catFirst = runner.catLast = NULL;

	config_html_write_string(&runner, "<html><head><title>EspLightNode Configuration</title></head><body><h1>EspLightNode Configuration</h1>");
	config_html_write_string(&runner, "<form method=\"POST\" action=\"/save\">");
	runner.base.beginmodule = config_html_beginmodule;
	runner.base.endmodule = config_html_endmodule;
	runner.base.booloption = config_html_booloption;
	runner.base.stringoption = config_html_stringoption;
	runner.base.intoption = config_html_intoption;
	config_run(&runner.base);
	config_html_write_string(&runner, "<input type=\"submit\" value=\"Save\"></input></form>");
	config_html_write_string(&runner, "</body></html>");

	os_sprintf(headers,"200 HTTP/1.0 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n",runner.len);
	httpd_slot_setsentcb(slot, config_html_sendchunk,(void *)firstchunk);
	httpd_slot_send(slot, (uint8_t *)headers, strlen(headers));
}

void config_submit(struct HttpdConnectionSlot *slot) {
	const char *response = "<html><head><title>Settings saving</title></head><body>Settings saving...</body></html>";
	char data[512];
	os_sprintf(data,"200 HTTP/1.0 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",strlen(response),response);
	httpd_slot_send(slot, (uint8_t *)data, strlen(data));
}
