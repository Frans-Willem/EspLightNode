FW_OFFSETS:=0x00000 0x40000
# Directories to store or retrieve things from
OBJ_DIR:=objs
SRC_DIR:=user
OUTPUT_DIR:=firmware
# Object files
OBJS:= cppcompat main \
	output_protocols/ws2801 \
	output_protocols/ws2812 \
	input_protocols/tpm2net \
	input_protocols/artnet \
	config/config \
	config/httpd

SDK:=/home/frans-willem/esp8266/SDK/esp_iot_sdk_v1.3.0
PORT:=/dev/ttyUSB0

CC:=xtensa-lx106-elf-gcc
CXX:=xtensa-lx106-elf-g++
ESPTOOL:=esptool.py

DEFINES:=-DENABLE_TPM2NET -DENABLE_ARTNET -DENABLE_WS2812
CFLAGS:=-Os -ggdb -std=c99 -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
	-nostdlib -mlongcalls -mtext-section-literals -Wno-address \
	-I./include/ -I./$(SRC_DIR) -I$(SDK)/include
CXXFLAGS:=-Os -ggdb -std=c++0x -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
	-nostdlib -mlongcalls -mtext-section-literals -Wno-address \
	-I./include/ -I./$(SRC_DIR) -I$(SDK)/include
LDFLAGS:= -nostdlib -Wl,--no-check-sections -Wl,-static -L$(SDK)/lib -T$(SDK)/ld/eagle.app.v6.ld

LIBS:= c gcc phy pp net80211 wpa main lwip

all: $(FW_OFFSETS:%=$(OUTPUT_DIR)/%.bin)

-include $(OBJS:%=$(OBJ_DIR)/%.d)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM $< -MT $@ > $(@:%.o=%.d)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM $< -MT $@ > $(@:%.o=%.d)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/firmware.elf: $(OBJS:%=$(OBJ_DIR)/%.o) $(OBJSXX:%=$(OBJ_DIR)/%.o)
	$(CC) -o $@ $(LDFLAGS) -Wl,--start-group $^ $(addprefix -l,$(LIBS)) -Wl,--end-group

$(FW_OFFSETS:%=$(OUTPUT_DIR)/%.bin): $(OBJ_DIR)/firmware.elf
	@mkdir -p $(OUTPUT_DIR)
	$(ESPTOOL) elf2image --output $(OUTPUT_DIR)/ $^

flash: $(FW_OFFSETS:%=$(OUTPUT_DIR)/%.bin)
	$(ESPTOOL) --port $(PORT) --baud 460800 write_flash $(foreach x,$(FW_OFFSETS),$(x) $(OUTPUT_DIR)/$(x).bin)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OUTPUT_DIR)

