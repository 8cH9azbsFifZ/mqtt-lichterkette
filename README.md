# mqtt-lichterkette
IR remote control for "Lichterkette" on Raspi with MQTT support for integration in homeautomation systems (i.e. OpenHAB).

# Preparation of the Rasperry Pi
- Prepare a Raspi W Zero 
- Prepare and wire an IR transmitter module (i.e. [IR Transceiver](https://www.amazon.de/HALJIA-Digital-Infrarot-IR-Empf%C3%A4nger-Sensor-Modul-Transmitter/dp/B07BFNGF53))

| PIN GPIO          | PIN IR Transmitter  |
| ----------------- |:-------------------:| 
| 18 (GPIO24)       | PIN DAT             |
| PIN 2 (5V)        | PIN VCC             |
| 20 (GND)          | PIN GND             |

![Raspi GPIO Pins][gpio]


- Download [raspian](https://www.raspberrypi.org/downloads/raspbian/) buster lite and flash it (i.e. [Belena Etcher](https://www.balena.io/etcher/)).
- Prepare with ssh and WiFi
```
touch /Volumes/boot/ssh
cp lib/wpa_supplicant.conf /Volumes/boot/
vim /Volumes/boot/wpa_supplicant.conf
```

- Boot it, and adjust hostname (and fixed IP on your router?) password raspberry (default)
```
ssh -lpi <your_ip>

sudo su
echo klima-raspi > /etc/hostname
```
- Enable IR Mode for PINS by uncommenting the following lines:
```
apt-get install -y vim
vim /boot/config.txt 

dtoverlay=gpio-ir,gpio_pin=17
dtoverlay=gpio-ir-tx,gpio_pin=18
```
- Reboot the raspi
- Run install script for preparation  `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/8cH9azbsFifZ/mqtt-lichterkette/master/lib/install.sh)"` or clone this repo and run `./lib/install.sh`.


# Configuration of the software
- Configure MQTT variables in the corresponding source file:
```
vim src/lichterkette.c

#define MQTT_ADDRESS              "t20:1883"     
#define MQTT_TOPIC_ROOT           "lichterkette/"          
#define PAC_NAME                  "Lichterkette 1"        
```

- Build and install the software
```
make lichterkette
make install
sudo systemctl start  lichterkette
sudo systemctl enable lichterkette
```

# Reverse engineering a new remote
- use the `gcc bits.c`, `showbinaries.py` and `showpulses.py` in lib
- extract the bit patterns using stuff like: `showcodes.py` in lib

# Testing the installation
```
mosquitto_pub -h t20 -t lichterkette/toggle/power -m off
mosquitto_sub -h t20 -t lichterkette/power
```

# Example configurations
- Relevant fragments for configuration of OpenHAB are given in `examples`.

## References
- [PAC N90 Remote](https://github.com/8cH9azbsFifZ/pac-n90-raspi)
- [Excellent description how to build this repo](https://blog.bschwind.com/2016/05/29/sending-infrared-commands-from-a-raspberry-pi-without-lirc/)
- [Bit masks for N90](https://github.com/zeroflow/ESPAircon)
- [IR Slinger library](https://github.com/bschwind/ir-slinger)
- [Bit masks for NK76](https://github.com/QuadrifoglioVerde/DL_PAC_NK76)
- [Reverse Engineering of an air conditioning control](https://www.instructables.com/id/Reverse-engineering-of-an-Air-Conditioning-control/)