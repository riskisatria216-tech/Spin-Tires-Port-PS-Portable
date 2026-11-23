TARGET = EBOOT
OBJS = src/main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lGU -lpspdebug -lpspdisplay -lpspge -lpspctrl -lm

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = MudRunner PSP 0.1

include $(PSPSDK)/lib/build.mak
