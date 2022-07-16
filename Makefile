CCFLAGS = -m32 -static -nodefaultlibs -nostdlib -s -m32 -mwindows -O2 -w -march=i486
LIBS = -luser32 -lkernel32 -lgdi32 -lgdiplus -ladvapi32 -lshell32
ENTRYPOINT = -Wl,-e_start
SOURCE_FILES = dialog.c main.c resource.res

resource.res:
	windres resource.rc -O coff -F pe-i386 -o $@

winemine.exe: $(SOURCE_FILES)
	$(CC) $(CCFLAGS) -o $@ $(SOURCE_FILES) $(LIBS) $(ENTRYPOINT)

clean:
	rm winemine.exe resource.res

all: resource.res winemine.exe

.PHONY: clean all
