MODULE := common

MODULE_OBJS := \
	archive.o \
	config-file.o \
	config-manager.o \
	debug.o \
	error.o \
	EventDispatcher.o \
	EventRecorder.o \
	file.o \
	fs.o \
	hashmap.o \
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	random.o \
	rational.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	translation.o \
	unarj.o \
	unzip.o \
	util.o \
	xmlparser.o \
	zlib.o

ifdef ENABLE_TRANSLATION
common/translation.cpp: common/messages.cpp

common/messages.cpp: $(wildcard po/*.po)
	tools/po2c $^ > common/messages.cpp
endif

# Include common rules
include $(srcdir)/rules.mk
