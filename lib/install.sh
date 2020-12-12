#!/bin/bash

# Enable IR Mode for PINS
echo "Enable IR Mode for PINS by uncommenting the following lines in config.txt"
sudo sed -i 's/\#dtoverlay=gpio-ir,gpio_pin=17/dtoverlay=gpio-ir,gpio_pin=17/g' /boot/config.txt
sudo sed -i 's/\#dtoverlay=gpio-ir-tx,gpio_pin=18/dtoverlay=gpio-ir-tx,gpio_pin=18/g' /boot/config.txt

# Upgrade Raspian
sudo apt-get update
sudo apt-get -y upgrade

# Install essential stuff
sudo apt-get -y install vim git build-essential
sudo apt-get -y install libpigpio-dev libpigpio1 

# Install MQTT stuff
sudo apt-get -y install build-essential gcc make cmake cmake-gui cmake-curses-gui
sudo apt-get -y install fakeroot fakeroot devscripts dh-make lsb-release
sudo apt-get -y install libssl-dev
sudo apt-get -y install doxygen graphviz
cd
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make
sudo make install


# Install remote control stuff
cd
git clone https://github.com/8cH9azbsFifZ/mqtt-lichterkette.git
cd mqtt-lichterkette
