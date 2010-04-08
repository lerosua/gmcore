all:
	gcc -fPIC `pkg-config pidgin --cflags --libs` --shared -Wall -O2 gmchess-network.c -o gmchess-network.so

install:
	#cp gmchess-network.so /usr/lib/pidgin
	cp gmchess-network.so ${HOME}/.purple/plugins/
	#cp gmchess.png /usr/share/icons/hicolor/22x22/apps/

install-user:
	mkdir -p ${HOME}/.purple/plugins
	cp gmchess-network.so ${HOME}/.purple/plugins/

clean:
	rm *.o *.a *.so
