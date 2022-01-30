SRCDIR = src
OBJDIR = obj

OPENSSL_CFLAGS = $(shell pkg-config --cflags openssl)
OPENSSL_LFLAGS = $(shell pkg-config --libs openssl)

CXX = g++
CXXFLAGS = -std=c++17 $(OPENSSL_CFLAGS)
LFLAGS = -pthread $(OPENSSL_LFLAGS)

_DEPS = $(shell ls $(SRCDIR) | grep .hpp)
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

SRC = $(shell ls $(SRCDIR) | grep .cpp)
_OBJ = $(SRC:.cpp=.o)
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

OUT = ./signature

default: $(OUT)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	mkdir -p $(OBJDIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OUT): $(OBJ)
	$(CXX) -o $(OUT) $(OBJ) $(LFLAGS)

clean:
	rm -rf $(OBJDIR) $(OUT)
