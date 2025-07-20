# Makefile for Nugget game
#
# Team 7, 2025

L = libcs50
.PHONY: all clean

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
        make -C libcs50 
        make -C support 
        make -C client 
        make -C server

############### TAGS for emacs users ##########
TAGS:  Makefile */Makefile */*.c */*.h */*.md */*.sh 
        etags $^

############## clean  ##########
clean: 
        rm -f *~ 
        rm -f TAGS 
        make -C support clean 
        make -C client clean 
        make -C server clean