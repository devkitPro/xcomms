.SUFFIXES:
#.SILENT:

SOURCES		:=	source resource data
INCLUDES	:=	include resource
BUILD		:=	build
TARGET		:=	xcomms

LIBDIRS	:=	$(DEVKITPRO)/libgba

ASFLAGS		:=
CXXFLAGS	:= -O2  -Wall  -Wno-multichar -Wunused -fno-rtti -fno-exceptions $(shell fltk-config --cxxflags)
LDFLAGS		:= -s $(shell fltk-config --ldflags)

UNAME := $(shell uname -s)

ifneq (,$(findstring MINGW,$(UNAME)))
	export PATH:=$(PATH):/mingw/bin
	PLATFORM	:= win32
	EXE		:= .exe
	EXTRAS		:= UserPort
	LDFLAGS		+= -lWinmm
endif

BINARIES	:=	$(TARGET)$(EXE)

ifneq (,$(findstring Linux,$(UNAME)))
	PLATFORM	:= linux
	EXE		:=
	BINARIES	+= $(TARGET)_static
	LDFLAGS		+= -lpthread
endif

#CXX		:= g++


#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))


export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

CXXFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cxx)))
ASMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifneq (,$(findstring MINGW,$(UNAME)))
	RESOURCEFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.rc)))
endif

ifneq (,$(findstring Linux,$(UNAME)))
export	STATIC		:= $(OUTPUT)_static
endif


export OFILES	:=	$(ASMFILES:.s=.o) $(CXXFILES:.cxx=.o) $(RESOURCEFILES:.rc=.o)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export DEPENDS	:=	$(OFILES:.o=.d)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo cleaning ...
	@rm -fr $(BUILD)

run:
	$(OUTPUT)$(EXE)

#---------------------------------------------------------------------------------
dist:
#---------------------------------------------------------------------------------
	@tar --exclude=*svn* -cvjf $(OUTPUT)_$(PLATFORM).tar.bz2 docs $(BINARIES) $(EXTRAS)

#---------------------------------------------------------------------------------
else

$(OUTPUT)$(EXE):	$(STATIC) $(OFILES)
	@echo Linking $(notdir $@)...
	$(CXX) $(OFILES) $(LDFLAGS) -o $@

$(STATIC):	$(OFILES)
	@echo "Linking $(notdir $@)..."
	$(CXX) -static $(OFILES) $(LDFLAGS) -o $@

%.o:	%.cxx
	@echo Compiling $(notdir $<)...
	$(CXX)  -MMD $(CXXFLAGS) $(INCLUDE) -c $<

%.o:	%.s
	@echo Compiling $(notdir $<)...
	$(CXX) -MMD $(ASFLAGS) -c $< -o $@

%.o:	%.rc
	@echo Compiling resource $(notdir $<)...
	@windres -i $< -o $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
