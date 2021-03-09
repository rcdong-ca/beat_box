TARGET= as3



PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L 
LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB

all: wav main.c audioMixer_template.h audioMixer_template.c i2cHandler.h i2cHandler.c beat_mode.h beat_mode.c joystick_control.h joystick_control.c
	$(CC_C) $(CFLAGS) main.c audioMixer_template.c i2cHandler.c beat_mode.c joystick_control.c -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound

# all: wav audioMixer_template.h audioMixer_template.c beat_mode.h beat_mode.c i2cHandler.c
# 	$(CC_C) $(CFLAGS) audioMixer_template.c beat_mode.c i2cHandler.c -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound


wav:
	mkdir -p $(PUBDIR)/beatbox-wav-files/ 
	cp -R beatbox-wave-files/* $(PUBDIR)/beatbox-wav-files/ 
	

# node:
# 	mkdir -p $(PUBDIR)/beatbox-server-copy/ 
# 	cp -R as3-server/* $(PUBDIR)/beatbox-server-copy/ 
# 	cd $(PUBDIR)/beatbox-server-copy/ && npm install

clean:
	rm -f $(OUTDIR)/$(TARGET)
