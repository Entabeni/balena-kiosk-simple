#!/bin/bash
# ultra/pro360.ppd Magicard Rio Pro 360
uuu=$(lpinfo -v | grep usb://Magicard/600)
URLFIN=${uuu#"direct "}
echo $URLFIN

# Check to see if the printer is already installed
if [ `lpstat -p 2>&1 | grep -E 'Magicard' -c || true` = "0" ]
then
    # Make sure the printer is available before trying to install it (check the printer web page)
    if [ $URLFIN ]
    then
        echo "Installing MAGICARD !!!"
        lpadmin \
            -p 'Magicard_600' \
            -v 'usb://Magicard/600?serial=72925919' \
            -m 'Magicard 600' \
            -P '/usr/share/cups/model/ultra/pro360.ppd' \
            -o 'CFColourFormat/Colour format=2MonoK' \
            -o 'CFBlackResin/Black resin options=0BlackResinK' \
            -o 'CFOverCoat/Overcoat=0PrintNoOvercoat' \
            -o 'CFHoloKote/HoloKote(R)=0PrintNoHoloKote' \
            -o 'CFHoloPatch/HoloPatch(R)=0PrintNoHoloPatch' \
            -o 'CFRotation/Rotation=1Rotation' \
            -o 'CFHolokoteRotation/HoloKote(R) Rotation=2HolokoteRotation' \
            -o 'CFUsewithLaminate/Use with Laminate=0UsewithLaminate' \
            -E
        # Set it as the default printer
        lpadmin -d 'Magicard_600'
        echo "Setting up CUPS default print options"
        sudo sed -ir 's/CFColourFormat.*/Colour format: 2MonoK/g;  
         s/CFBlackResin.*/Black resin options: 0BlackResinK/g;  
         s/CFOverCoat.*/Overcoat: 0PrintNoOvercoat/g;  
         s/CFHoloKote.*/HoloKote(R): 0PrintNoHoloKote/g;  
         s/CFHoloPatch.*/HoloPatch(R): 0PrintNoHoloPatch/g;  
         s/CFRotation.*/Rotation: 1Rotation/g;  
         s/CFHolokoteRotation.*/HoloKote(R) Rotation: 2HolokoteRotation/g;  
         s/CFUsewithLaminate.*/Use with Laminate: 0UsewithLaminate/g;'
         /usr/share/cups/model/ultra/pro360.ppd
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
