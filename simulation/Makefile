##############################################################################
#                                                                            #
# Simulation                                                                 #
#                                                                            #
##############################################################################

include ../Makefile

vpath %.c ..
vpath %.h ..
vpath config.h.dist ..

ARCH =
DEFS += -DSIMULATION -DGLOBAL_CLOCK
SOURCES += simulation.c
OBJ = $(patsubst %.c,%.o,${SOURCES})
CFLAGS = -g -Wall -Wstrict-prototypes -Os $(DEFS) -std=gnu99 -funsigned-char -funsigned-bitfields -fshort-enums -I.
LDFLAGS = 
LIBS = -lrt

sim: $(OBJ)
	@echo "  LINK      $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

