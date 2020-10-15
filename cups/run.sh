#!/bin/bash
# ultra/pro360.ppd Magicard Rio Pro 360

echo "Starting CUPS2"
cupsd -f &
chown -R root:root /usr/share/cups/model/ultra/

# Delete all printers
for printer in `lpstat -p | awk '{print $2}'`; do
    echo Deleting "$printer"
    lpadmin -x $printer
    echo """"
    echo "Deleting Printers and Continuing Install"
    echo ""
  done
echo 'Starting'
PRINTERURL=$(lpinfo -v | grep usb://Magicard/600)
echo $PRINTERURL
FINALURL=${PRINTERURL#"direct "}
echo $FINALURL

# Check to see if the printer is already installed
if [ `lpstat -p 2>&1 | grep -E 'Magicard' -c || true` = "0" ]
then
    # Make sure the printer is available before trying to install it (check the printer web page)
    if [ $FINALURL ]
    then
        echo "Installing MAGICARD !!!"
        lpadmin \
            -p 'Magicard_600' \
            -v $FINALURL \
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

cupsctl --remote-admin --remote-any

# Tell the container that DBUS should report to Host OS
export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket

# Set rules for connection only through the public device URL
iptables -A INPUT -i resin-vpn -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -p tcp --dport 80 -i resin-vpn -j ACCEPT
iptables -A INPUT -p tcp --dport 80 -j REJECT


wait
