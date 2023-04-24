TARGET = funkin
TYPE = ps-exe

SRCS = src/main.c \
       src/mutil.c \
       src/random.c \
       src/archive.c \
       src/font.c \
       src/trans.c \
       src/loadscr.c \
       src/menu.c \
       src/demu.c \
       src/stage.c \
       src/psx/psx.c \
       src/psx/io.c \
       src/psx/gfx.c \
       src/psx/audio.c \
       src/psx/pad.c \
       src/psx/timer.c \
       src/psx/movie.c \
       src/strplay.c \
       src/stime.c \
       src/stage/dummy.c \
       src/stage/fplace.c \
       src/stage/flames.c \
       src/stage/old.c \
       src/stage/space.c \
       src/stage/chop.c \
       src/stage/flamec.c \
       src/animation.c \
       src/character.c \
       src/character/bf.c \
       src/character/speaker.c \
       src/character/gf.c \
       src/character/titlegf.c \
       src/character/psychic.c \
       src/character/senpaib.c \
       src/character/titlepsy.c \
       src/character/bfm.c \
       src/character/psychm.c \
       src/character/gfm.c \
       src/character/sendai.c \
       src/character/bfspirit.c \
       src/character/bft.c \
       src/portraits/psyport.c \
       src/portraits/bfport.c \
       src/portraits/senport.c \
       src/object.c \
       src/object/combo.c \
       src/object/splash.c \
       mips/common/crt0/crt0.s

CPPFLAGS += -Wall -Wextra -pedantic -mno-check-zero-division
LDFLAGS += -Wl,--start-group
# TODO: remove unused libraries
LDFLAGS += -lapi
#LDFLAGS += -lc
LDFLAGS += -lc2
#LDFLAGS += -lcard
LDFLAGS += -lcd
#LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
#LDFLAGS += -lgs
LDFLAGS += -lgte
#LDFLAGS += -lgun
#LDFLAGS += -lhmd
#LDFLAGS += -lmath
#LDFLAGS += -lmcrd
#LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
#LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
#LDFLAGS += -ltap
LDFLAGS += -flto -Wl,--end-group

include mips/common.mk
