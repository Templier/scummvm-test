MODULE := engines/hugo

MODULE_OBJS := \
	detection.o \
	display.o \
	display_v1d.o \
	display_v1w.o \
	engine.o \
	file.o \
	file_v1d.o \
	file_v2d.o \
	file_v3d.o \
	file_v1w.o \
	hugo.o \
	intro.o \
	intro_v1d.o \
	intro_v2d.o \
	intro_v3d.o \
	intro_v1w.o \
	intro_v2w.o \
	intro_v3w.o \
	inventory.o \
	mouse.o \
	parser.o \
	parser_v1w.o \
	parser_v1d.o \
	parser_v2d.o \
	route.o \
	schedule.o \
	schedule_v1d.o \
	schedule_v3d.o \
	sound.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_HUGO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
