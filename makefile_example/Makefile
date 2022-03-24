# set /include as include path
GCC = gcc
COPS = -Iinclude 
OUT_DIR = compiled_files
SRC_DIR = src
C_FILES = $(wildcard $(SRC_DIR)/*.c)
COMPIELD = $($(OUT_DIR)/%.o)

run: one two COMPIELD
	# Outputs "hey", since this is the first target
	echo $@

	# Outputs all prerequisites newer than the target
	# if you change file two, this line will be "echo two"
	echo $?

	# Outputs all prerequisites
	echo $^

	#first prerequisites
	echo $<

	touch hey

one:
	touch one

two:
	touch two

COMPIELD: $(C_FILES)
	echo $^
	echo $@
	$(GCC) $(COPS) -o $@ $^

clean:
	rm -f hey one two 