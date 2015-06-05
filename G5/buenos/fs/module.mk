# Makefile for the drivers module

# Set the module name
MODULE := fs

FILES := gfs.c vfs.c tfs.c filesystems.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
