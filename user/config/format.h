#ifndef CONFIG_FORMAT_H
#define CONFIG_FORMAT_H

#define CONFIG_START_SECTOR 63
#define CONFIG_SECTOR_DIRECTION -1

#define CONFIG_HEADER	"ELN0"

enum ConfigCommand {
	ConfigSectionStart,
	ConfigSectionEnd,
	ConfigString,
	ConfigInteger
};

#endif//CONFIG_FORMAT_H
