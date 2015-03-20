fseventw: fseventw.c
	clang -o $@ -Wall -Werror -framework CoreServices -framework CoreFoundation $<
