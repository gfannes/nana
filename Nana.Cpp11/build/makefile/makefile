#Nana C++ Library
#
#Makefile created by Jinhao(cnjinhao@hotmail.com)

.PHONY: help clean lib build-tests run-tests

help:
	@echo "make (help|clean|lib|build-tests|run-tests)"

BINDIR = ../bin

LIB	= $(BINDIR)/libnana.a
lib: $(LIB)

UNIT_TESTS	= $(BINDIR)/ut_nana.exe
build-tests: $(UNIT_TESTS)
run-tests: build-tests
	$(UNIT_TESTS)

#Directories
INCROOT	 = ../../include
SRCROOT	 = ../../source
TESTROOT = ../../test

#Include options
INCS4LIB	= -I$(INCROOT) -I/usr/include/freetype2
INCS4TEST	= -I$(INCROOT) -I$(TESTROOT)

#Source files for the nana library
SRCS4LIB = $(wildcard $(SRCROOT)/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/detail/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/filesystem/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/audio/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/audio/detail/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/gui/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/gui/detail/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/gui/widgets/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/gui/widgets/skeletons/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/paint/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/paint/detail/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/system/*.cpp)
SRCS4LIB += $(wildcard $(SRCROOT)/threads/*.cpp)
#Object files for the library
OBJS4LIB  = $(SRCS4LIB:.cpp=.lib.o)

#Source files for the tests
SRCS4TEST = $(wildcard $(TESTROOT)/*.cpp)
#Object files for the tests
OBJS4TEST = $(SRCS4TEST:.cpp=.test.o)

#Instruments needed
GCC	= g++
AR = ar
RANLIB = ranlib
MKDIR = mkdir
RM = rm

#Compilation rules
COMPILE_OPTIONS = -c -g -std=c++0x -Wall
%.lib.o: %.cpp
	$(GCC) $(COMPILE_OPTIONS) $(INCS4LIB) $< -o $@
%.test.o: %.cpp
	$(GCC) $(COMPILE_OPTIONS) $(INCS4TEST) $< -o $@

#Linking a library
$(LIB): $(OBJS4LIB)
	-$(MKDIR) $(BINDIR)
	$(AR) r $(LIB) $(OBJS4LIB)
	$(RANLIB) $(LIB)

#Linking an executable
LINK_OPTIONS = 
$(UNIT_TESTS): $(LIB) $(OBJS4TEST)
	$(GCC) -o $@ $(OBJS4TEST) $(LINK_OPTIONS)

clean:
	$(RM) -f $(OBJS4LIB)
	$(RM) -f $(LIB)
	$(RM) -f $(OBJS4TEST)
	$(RM) -f $(UNIT_TESTS)

