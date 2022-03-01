# 0 "main.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/aarch64-linux-gnu/include/stdc-predef.h" 1 3
# 0 "<command-line>" 2
# 1 "main.S"
.section ".text"
_start:
    wfe
    b _start
