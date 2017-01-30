CC := g++

SRCDIR := Source
BUILDDIR := Build
TARGET := renderer
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=c++11 -g -Wall -Wextra -pedantic -Ofast -fopenmp
LIB := -L/usr/local/lib/ -lSDL -lSDLmain -L/usr/lib/x86_64-linux-gnu/ -lgomp
INC := -Ilib/glm/ -I/usr/local/include/SDL/ 

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB) 2> link_error.log

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< 2> build_error.log

clean:
	@echo " Cleaning..."; 
	@echo " find $(BUILDDIR) ! -type d -delete"; find $(BUILDDIR) ! -type d -delete; rm $(TARGET)
#alt for linux systems that don't support -delete: "find $(BUILDDIR) ! -type d -exec rm '{}' \;"

run:
	export LD_LIBRARY_PATH=$(PWD)/libs/lib/; \
	$(TARGET)

# Tests
tester:
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

.PHONY: clean
