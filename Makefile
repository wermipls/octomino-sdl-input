.POSIX:
NAME     = Octomino's SDL Input
COMMIT   = $(shell git describe --always --dirty --match 'NOT A TAG')
VERSION  = wermi-$(COMMIT)
REPO     = https://github.com/wermipls/octomino-sdl-input

BIN      = octomino-sdl-input.dll
SRC      = $(wildcard src/*.c)

DBURL    = https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt

ZIPNAME  = $(BIN:.dll=)-$(VERSION).zip
ZIPFILES = $(BIN) LICENSE README.md gamecontrollerdb.txt sources.zip
ZIPSRC   = $(wildcard src/*.c) $(wildcard src/*.h) $(wildcard src/*.inl) Makefile

CC       = i686-w64-mingw32-gcc
OPTFLAGS = -O2 -flto
CFLAGS   = -std=c11 -MMD -fvisibility=hidden \
           -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter \
           -DPLUGIN_NAME=\""$(NAME)"\" \
           -DPLUGIN_VERSION=\""$(VERSION)"\" \
           -DPLUGIN_REPO=\""$(REPO)"\"
LDFLAGS  = -shared -static-libgcc -static \
           -lshlwapi `sdl2-config --static-libs` \
		   -lopengl32

$(BIN): $(SRC:.c=.o)
	$(CC) $(OPTFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

-include $(SRC:.c=.d)

$(ZIPNAME): $(ZIPFILES)

sources.zip: $(ZIPSRC)

%.zip:
	rm -f $@
	zip $@ $^

.PHONY: all update-db clean

all: $(BIN) $(ZIPNAME)

update-db:
	rm -f gamecontrollerdb.txt
	wget $(DBURL)

clean:
	rm -f $(BIN) $(SRC:.c=.o) $(SRC:.c=.d) $(ZIPNAME) sources.zip

debug: OPTFLAGS = -g

debug: $(BIN)