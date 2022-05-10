#!/bin/sh
make CFLAGS=-DDEBUG >/dev/null
make qemu-run
