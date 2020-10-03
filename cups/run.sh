#!/bin/bash
# ultra/pro360.ppd Magicard Rio Pro 360
sleep 10
echo 'Starting1'
sudo systemctl start org.cups.cupsd.service
echo 'Starting2'
sleep 10

echo 'Starting'
ABCD=$(lpinfo -v | grep usb://Magicard/600)
echo $ABCD
URLFIN1=${ABCD#"direct "}
echo $URLFIN1
sudo sed -ir 's/*DefaultCFBlackResin: 1BlackResinYMC/*DefaultCFBlackResin: 0BlackResinK/g;
            s/*DefaultCFOverCoat: 1PrintOvercoat/*DefaultCFOverCoat: 0PrintNoOvercoat/g;
            s/*DefaultCFColourFormat: 0ColourYMC/*DefaultCFColourFormat: 2MonoK/g;
            s/*DefaultCFHoloKote: 1PrintHoloKote/*DefaultCFHoloKote: 0PrintNoHoloKote/g;
            s/*DefaultCFRotation: 0Rotation/*DefaultCFRotation: 1Rotation/g;
            s/*DefaultCFUsewithLaminate: 0UsewithLaminate/*DefaultCFUsewithLaminate: 1UsewithLaminate/g;' /usr/share/cups/model/ultra/pro360.ppd
echo 'Updated ppd'

# Check to see if the printer is already installed
if [ `lpstat -p 2>&1 | grep -E 'Magicard' -c || true` = "0" ]
then
    # Make sure the printer is available before trying to install it (check the printer web page)
    if [ $URLFIN1 ]
    then
        echo "Installing MAGICARD !!!"
        lpadmin \
            -p 'Magicard_600' \
            -v $URLFIN1 \
            -m 'Magicard 600' \
            -P '/usr/share/cups/model/ultra/pro360.ppd' \
            -E
        # Set it as the default printer
        lpadmin -d 'Magicard_600'
        echo "Setting up CUPS default print options"
              echo "Setting retry policy"
        lpadmin -p Magicard_600 -o printer-error-policy=abort-job

    fi
fi
# Tell the container that DBUS should report to Host OS
export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket

# Set rules for connection only through the public device URL
iptables -A INPUT -i resin-vpn -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -p tcp --dport 80 -i resin-vpn -j ACCEPT
iptables -A INPUT -p tcp --dport 80 -j REJECT

echo "Starting CUPS2"
cupsd -f &



wait
