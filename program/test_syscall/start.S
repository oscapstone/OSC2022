.section ".text.start"
.global _start
.extern init
_start:
    bl init