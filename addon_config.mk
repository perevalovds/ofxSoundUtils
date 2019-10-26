meta:
	ADDON_NAME = ofxSoundUtils
	ADDON_DESCRIPTION = Low-level sound utils, such as mu-law conversion and reading/writing RAW files 
	ADDON_AUTHOR = Denis Perevalov
	ADDON_TAGS = "Sound"
	ADDON_URL = https://github.com/perevalovds/ofxSoundUtils

common:

	#ADDON_INCLUDES +=  libs/...

osx:
vs:	
	# x64
	#ADDON_LIBS += libs/realsense2/lib/x64/realsense2.lib
	#ADDON_DLLS_TO_COPY += libs/realsense2/lib/x64/realsense2.dll;libs/realsense2/lib/x64/Intel.Realsense.dll
	# Win32
	#ADDON_LIBS += libs/realsense2/lib/vs/Win32/realsense2.lib
	#ADDON_DLLS_TO_COPY += libs/realsense2/lib/Win32/realsense2.dll;libs/realsense2/lib/Win32/Intel.Realsense.dll
	
	
linux64:
linuxarmv6l:
linuxarmv7l:
msys2:
