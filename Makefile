FW_OFFSETS:=0x00000 0x40000
OBJ_DIR:=objs
SRC_DIR:=user
OUTPUT_DIR:=firmware
OBJS:= main \
	output_protocols/ws2801 \
	output_protocols/ws2812 \
	input_protocols/tpm2net \
	input_protocols/artnet \
	config/config \
	config/httpd

SRCS:= $(OBJS:%.o=%.c)
PREFIX:=xtensa-lx106-elf-
SDK:=/home/frans-willem/esp8266/SDK/esp_iot_sdk_v1.3.0
DEFINES:=-DENABLE_TPM2NET -DENABLE_ARTNET -DENABLE_WS2812
CFLAGS:=-Os -ggdb -std=c99 -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
	-nostdlib -mlongcalls -mtext-section-literals -Wno-address
# -nostdlib -mlongcalls -mtext-section-literals -Wl,-static -Wl,--no-check-sections -I$(SDK)/include -I./include -I./user -L$(SDK)/lib -T$(SDK)/ld/eagle.app.v6.ld
LDFLAGS:= -nostdlib -Wl,--no-check-sections -Wl,-static -L$(SDK)/lib -T$(SDK)/ld/eagle.app.v6.ld

LIBS:= c gcc phy pp net80211 wpa main lwip

all: $(addprefix $(OUTPUT_DIR)/,$(addsuffix .bin, $(FW_OFFSETS)))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	xtensa-lx106-elf-gcc -I./include/ -I./$(SRC_DIR) -I$(SDK)/include $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/firmware.elf: $(addprefix objs/,$(addsuffix .o, $(OBJS)))
	xtensa-lx106-elf-gcc -o $@ $(LDFLAGS) -Wl,--start-group $^ $(addprefix -l,$(LIBS)) -Wl,--end-group

$(addprefix $(OUTPUT_DIR)/,$(addsuffix .bin, $(FW_OFFSETS))): $(OBJ_DIR)/firmware.elf
	@mkdir -p $(OUTPUT_DIR)
	esptool.py elf2image --output $(OUTPUT_DIR)/ $^

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OUTPUT_DIR)

