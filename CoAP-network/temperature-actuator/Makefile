CONTIKI_PROJECT = temperature-actuator

all: $(CONTIKI_PROJECT)

CONTIKI = ../../../..
MAKE_ROUTING = MAKE_ROUTING_RPL_LITE

MODULES += os/net/app-layer/coap

MODULES_REL += /home/user/.local/lib/python3.10/site-packages/emlearn
TARGET_LIBFILES += -lm
INC += /home/user/.local/lib/python3.10/site-packages/emlearn
CFLAGS += -I/home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn
CFLAGS  += -DCOAP_OBSERVE_CLIENT_STATEFUL=1

include $(CONTIKI)/Makefile.include



