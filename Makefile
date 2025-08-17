# Makefile for savecb.c
# This version uses pkg-config, the recommended way to get compiler flags
# and library paths for GTK, making it more portable.

CC = gcc

# Get the necessary compiler flags and library paths using pkg-config.
# This assumes pkg-config is installed and can find the gtk+-3.0 library.
# The `_CFLAGS` variable will contain flags like `-I/usr/include/gtk-3.0`.
# The `_LIBS` variable will contain flags like `-lgtk-3`.
PKG_CONFIG_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
PKG_CONFIG_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Rule to build the executable
savecb:	savecb.c
	$(CC) $(PKG_CONFIG_CFLAGS) savecb.c -o savecb $(PKG_CONFIG_LIBS)

# Rule to install the executable
# It checks if the user is root and installs to the appropriate directory.
install: savecb
	@if [ "$(shell whoami)" = "root" ]; then \
		echo "Installing savecb to /usr/local/bin..."; \
		install -D -m 755 savecb /usr/local/bin/savecb; \
		echo "Successfully installed for all users."; \
	else \
		echo "Installing savecb to ~/projects/bin..."; \
		mkdir -p ~/projects/bin; \
		install -D -m 755 savecb ~/projects/bin/savecb; \
		echo "Successfully installed to your local bin directory."; \
	fi

# Rule to clean up generated files
clean:
	rm -f savecb

