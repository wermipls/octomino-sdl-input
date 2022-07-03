.POSIX:
NAME     = Octomino's SDL Input
COMMIT   = $(shell git describe --always --dirty --match 'NOT A TAG')
VERSION  = wermi-$(COMMIT)
REPO     = https://github.com/wermipls/octomino-sdl-input

BIN      = octomino-sdl-input.dll
SRC      = $(wildcard src/*.cpp)

DBURL    = https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt

ZIPNAME  = $(BIN:.dll=)-$(VERSION).zip
ZIPFILES = $(BIN) LICENSE README.md gamecontrollerdb.txt sources.zip
ZIPSRC   = $(wildcard src/*.cpp) $(wildcard src/*.hpp) $(wildcard src/*.h) Makefile

CC       = i686-w64-mingw32-g++
OPTFLAGS = -Os -s -flto
CPPFLAGS = -std=c++17 -MMD -fvisibility=hidden \
           -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter \
           -DPLUGIN_NAME=\""$(NAME)"\" \
           -DPLUGIN_VERSION=\""$(VERSION)"\" \
           -DPLUGIN_REPO=\""$(REPO)"\"
LDFLAGS  = -shared -static-libgcc -static \
           -lshlwapi `sdl2-config --static-libs` \

$(BIN): $(SRC:.cpp=.o)
	$(CC) $(OPTFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

-include $(SRC:.cpp=.d)

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
	rm -f $(BIN) $(SRC:.cpp=.o) $(SRC:.cpp=.d) $(ZIPNAME) sources.zip

debug: OPTFLAGS = -g

debug: $(BIN)