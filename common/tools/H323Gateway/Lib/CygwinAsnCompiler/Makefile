# default build configuration
CFG ?= config.mk

include $(CFG)
include proj.mk

# Generate object file names from source file names
# C and C++ objects
COBJ = $(SRC:.c=.${O}) $(CXX_SRC:.cpp=.${O})
# Assembler objects
ASOBJ = $(AS_SRC:.S=.${O})
# All objects
OBJ = $(COBJ) $(ASOBJ)

# frontend object files
FRONTEND_SRC = lex.yy.c y.tab.c y.tab.h
FRONTEND_OBJ = lex.yy.$(O) y.tab.$(O) 

.PHONY : all debug clean test

all: $(OUT)

lex.yy.c : $(LEXER) $(CXX_HEADERS)
	@echo "LEX" $<
	@$(LEX) $(LEXER)

y.tab.h y.tab.c : $(PARSER) $(CXX_HEADERS)
	@echo "YACC" $<
	@$(YACC) $(PARSER)

$(FRONTEND_OBJ) : $(FRONTEND_SRC)
	@echo "FRONTEND: " $(FRONTEND_SRC)
	@$(CXX) $(CXXFLAGS) $(DEFINES) -c y.tab.c lex.yy.c

.S.$(O) :
	@echo "AS" $<
	@$(AS) $(ASFLAGS) -o $@ $<

.c.$(O) :
	@echo "CC" $<
	@$(CC) $(CFLAGS) $(DEFINES) -c $<

.cpp.$(O) :
	@echo "CXX" $<
	@$(CXX) $(CXXFLAGS) $(DEFINES) -c $<

$(COBJ) : $(HEADERS) $(CXX_HEADERS)

$(ASOBJ) : $(AS_INC)

$(OUT) : $(FRONTEND_OBJ) $(OBJ)
	@echo "LD" $@
	@$(LD) $(LDFLAGS) -o $(OUT) $(OBJ) $(FRONTEND_OBJ) $(LIBS)

clean :
	@rm -f $(OBJ)
	@rm -f $(FRONTEND_OBJ)
	@rm -f $(OUT) $(OUT).exe
	@rm -f $(FRONTEND_SRC)
	@rm -f itog.cpp itog.h lex.log

itog.cpp itog.h lex.log : all test.asn
	@echo "ASN" $<
	@./$(OUT) test.asn

test : itog.cpp itog.h lex.log

# Debug target
debug :
	@echo "DEBUG build"
	@DEBUG="1" $(MAKE) -s

