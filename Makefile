
#name of output binary file
BINARY := DS2Pong

#folder of sdk root
DSTWOSDK := /mnt/DSTwo
BAGLIB := $(DSTWOSDK)/BAGLib

#folders with sources
SOURCES = src
INCLUDES = include


# CROSS :=#
CROSS := /opt/mipsel-4.1.2-nopic/bin/

CC = $(CROSS)mipsel-linux-gcc
AR = $(CROSS)mipsel-linux-ar rcsv
LD	= $(CROSS)mipsel-linux-ld
OBJCOPY	= $(CROSS)mipsel-linux-objcopy
NM	= $(CROSS)mipsel-linux-nm
OBJDUMP	= $(CROSS)mipsel-linux-objdump


FS_DIR = $(DSTWOSDK)/libsrc/fs
CONSOLE_DIR = $(DSTWOSDK)/libsrc/console
KEY_DIR = $(DSTWOSDK)/libsrc/key

#custom lib
FT2PATH := $(BAGLIB)/src/freetype/

CSRC := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
SSRC := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.S))


LIBS := $(DSTWOSDK)/lib/libds2b.a -lc -lm -lgcc
EXTLIBS := $(DSTWOSDK)/lib/libds2a.a $(BAGLIB)/baglib.a

INC := -I$(DSTWOSDK)/include -I$(SOURCES) -I$(FS_DIR) -I$(CONSOLE_DIR) -I$(KEY_DIR) \
			-I$(BAGLIB)/include -l$(FT2PATH) -l$(FT2PATH)/include\
			$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \


CFLAGS := -mips32 -O3 -mno-abicalls -fno-pic -fno-builtin \
	   -fno-exceptions -ffunction-sections -mlong-calls\
	   -fomit-frame-pointer -msoft-float -G 4  
	 
LINKS := $(DSTWOSDK)/specs/link.xn
STARTS := $(DSTWOSDK)/specs/start.S
STARTO := start.o


COBJS	:= $(CSRC:.c=.o)
SOBJS	:= $(SSRC:.S=.o)


APP	:= $(BINARY).elf


all: $(APP)
	$(OBJCOPY) -O binary $(APP) $(BINARY).bin
	$(OBJDUMP) -d $(APP) > $(BINARY).dump
	$(NM) $(APP) | sort > $(BINARY).sym
	$(OBJDUMP) -h $(APP) > $(BINARY).map
	../tools/makeplug $(BINARY).bin $(BINARY).plg

$(APP):	depend $(SOBJS) $(COBJS) $(STARTO) $(LINKS) $(EXTLIBS)
	$(CC) -nostdlib -static -T $(LINKS) -o $@ $(STARTO) $(SOBJS) $(COBJS) $(EXTLIBS) $(LIBS) 

$(EXTLIBS): 
	make -C ../source/

$(STARTO):
	$(CC) $(CFLAGS) $(INC) -o $@ -c $(STARTS)

.c.o:
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
.cpp.o:
	$(CC) $(CFLAGS) $(INC) -fno-rtti -fvtable-gc -o $@ -c $<
.S.o:
	$(CC) $(CFLAGS) $(INC) -D_ASSEMBLER_ -D__ASSEMBLY__ -o $@ -c $<
	
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	
clean:
	rm -fr *.o $(OBJS) $(OTHER) *.bin *.sym *.map *.dump *.elf *.plg
	rm depend

depend:	Makefile
	$(CC) -MM $(CFLAGS) $(INC) $(SSRC) $(CSRC) > $@

include depend

