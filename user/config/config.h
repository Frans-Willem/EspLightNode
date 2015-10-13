/*
 * config.h
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_
#include "IConfigRunner.h"

class CHttpServer;
class CHttpRequest;
void config_load();
void config_run(IConfigRunner *pRunner);
void config_init(CHttpServer *pServer);

#define DEFINE_CONFIG(module) void module ## _runconfig(IConfigRunner *_configrunner);

#define BEGIN_CONFIG(module, description) void module ## _runconfig(IConfigRunner *_configrunner) { _configrunner->beginModule(#module , description);
#define END_CONFIG() _configrunner->endModule(); }

#define CONFIG_SUB(name) name ## _runconfig(_configrunner)
#define CONFIG_BOOLEAN(name, description, address, defvalue) _configrunner->optionBool(name, description, address, defvalue)
#define CONFIG_STRING(name, description, address, len, defvalue) _configrunner->optionString(name, description, address, len, defvalue)
#define CONFIG_INT(name, description, address, min, max, defvalue) _configrunner->optionInt(name, description, address, sizeof(*(address)), min, max, defvalue)
#define CONFIG_SELECTSTART(name, description, address, defvalue) _configrunner->optionSelectBegin(name, description, address, defvalue)
#define CONFIG_SELECTOPTION(name, value) _configrunner->optionSelectItem(name, value)
#define CONFIG_SELECTEND() _configrunner->optionSelectEnd()
#define CONFIG_IP(name, description, address, a, b, c, d) _configrunner->optionIpAddress(name,description,address,a | (b << 8) | (c << 16) | (d << 24))

#endif /* CONFIG_CONFIG_H_ */
