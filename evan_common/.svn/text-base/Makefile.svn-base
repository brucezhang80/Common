#****************************************************************************
#
# Makefile for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := YES

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL := YES

#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CFLAGS     := -fPIC -Wall -Wno-format -Wshadow -g -DDEBUG
RELEASE_CFLAGS   := -fPIC -Wall -Wno-unknown-pragmas -Wno-format -O3

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g 
RELEASE_LDFLAGS  := 

LIBPATH := ../lib
LIBS    := -L${LIBPATH} 

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${TINYXML_USE_STL})
  DEFS := -DTIXML_USE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

INCS := -I../steve_common/ -I../../../keyword/src/


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

COMMON_SO := ../../lib/libksscomm.a

all: ${COMMON_SO} 

#****************************************************************************
# Source files
#****************************************************************************

COMMON_SO_SRCS := route.cpp HashTbl.cpp list.cpp

COMMON_SO_OBJS := $(addsuffix .o,$(basename ${COMMON_SO_SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${COMMON_SO}: ${COMMON_SO_OBJS}
	ar -r ${COMMON_SO} ${COMMON_SO_OBJS}  

#****************************************************************************
# common rules
#****************************************************************************


# Rules for compiling source files to object files
%.o : %.cpp  
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@
%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	rm -f core ${COMMON_SO} ${COMMON_SO_OBJS}
	

depend:
	#makedepend ${INCS} ${SRCS}

