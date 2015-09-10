/*
 * config.h
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_
#include "IConfigRunner.h"

struct HttpdConnectionSlot;
void config_load();
void config_html(struct HttpdConnectionSlot *slot);
void config_submit(struct HttpdConnectionSlot *slot);

#define DEFINE_CONFIG(module) void module ## _runconfig(IConfigRunner *_configrunner);

#define BEGIN_CONFIG(module, description) void module ## _runconfig(IConfigRunner *_configrunner) { _configrunner->beginModule(#module , description);
#define END_CONFIG() _configrunner->endModule(); }

#define CONFIG_SUB(name) module ## _runconfig(_configrunner)
#define CONFIG_BOOLEAN(name, description, address, defvalue) _configrunner->optionBool(name, description, address, defvalue)
#define CONFIG_STRING(name, description, address, len, defvalue) _configrunner->optionString(name, description, address, len, defvalue)
#define CONFIG_INT(name, description, address, min, max, defvalue) _configrunner->optionInt(name, description, address, sizeof(*(address)), min, max, defvalue)

#endif /* CONFIG_CONFIG_H_ */
