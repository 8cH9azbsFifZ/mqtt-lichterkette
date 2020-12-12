lichterkette:
	gcc  src/lichterkette.c -lm -lpigpio -pthread -lrt -lpaho-mqtt3c -olichterkette

install:
	sudo install lichterkette /usr/local/bin
	sudo install src/lichterkette.service /etc/systemd/system/


uninstall:
	sudo systemctl stop lichterkette
	sudo systemctl disable lichterkette
	sudo rm /usr/local/bin/lichterkette
	sudo rm /etc/systemd/system/lichterkette.service


run:
	sudo ./lichterkette
