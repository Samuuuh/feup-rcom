cc = GCC

# ---- Directories
# Source files
SDIR =./src
# Header files
IDIR =./include
# Library files
LDIR =./lib
# Object files
ODIR =./obj
# Executable files
BDIR =./bin

CFLAGS =-Wall

TARGET = reader

.PHONY: all clean

all: $(BDIR)/$(TARGET)

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $(IFLAGS) $< -o $@

# Create library
makelib: makefolders &(LDIR)/lib.a

$(BDIR)/$(TARGET): makelib $(ODIR)/$(MAIN)
	$(CC) $(CFLAGS) -o $@ $(word 2, $^) $(DEPS)

makefolders:
	mkdir -p $(LDIR)
	mkdir -p $(ODIR)
	mkdir -p $(BDIR)