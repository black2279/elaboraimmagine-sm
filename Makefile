
SOURCES=elaboraimmagine.cpp

ADDITIONAL_FLAGS ?= -g
RELEASE_WIN32 = win32
RELEASE_WIN64 = win64
RELEASE_UNIX = unix

all: $(RELEASE_WIN32) $(RELEASE_WIN64) $(RELEASE_UNIX)

win32: elaboraimmagine-$(RELEASE_WIN32).exe
win64: elaboraimmagine-$(RELEASE_WIN64).exe
unix: elaboraimmagine

elaboraimmagine-x86.exe: $(SOURCES)
	$(CXX) $(ADDITIONAL_FLAGS) -Wall "$(<)" -o "$(RELEASE_WIN32)/$(@)"
	
elaboraimmagine-x64.exe: $(SOURCES)
	$(CXX) $(ADDITIONAL_FLAGS) -Wall "$(<)" -o "$(RELEASE_WIN64)/$(@)"

elaboraimmagine: $(SOURCES)
	$(CXX) $(ADDITIONAL_FLAGS) -Wall "$(<)" -o "$(RELEASE_UNIX)/$(@)"

clean: 
	$(RM) -rf $(RELEASE_WIN32)
	$(RM) -rf $(RELEASE_WIN64)
	$(RM) -rf $(RELEASE_UNIX)