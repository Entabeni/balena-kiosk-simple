#!/bin/bash
# ultra/pro360.ppd Magicard Rio Pro 360
PRINTER_BOOLEAN="0"
while [ "$PRINTER_BOOLEAN" = "0" ];
do
    echo "Starting CUPS2"
    cupsd -f &
    chown -R root:root /usr/share/cups/model/ultra/
    PRINTERURL=$(lpinfo -v | grep usb://Magicard)
    echo $PRINTERURL
    FINALURL=${PRINTERURL#"direct "}
    echo $FINALURL
    # Delete all printers
    if [ $FINALURL ]
    then
        for printer in `lpstat -p | awk '{print $2}'`; do
            echo Deleting "$printer"
            lpadmin -x $printer
            echo "Deleting Printers and Continuing Install"
            
        done
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
            PRINTER_BOOLEAN="1"
            echo $PRINTER_BOOLEAN
    else
        echo "Not deleting printer because no printers are plugged in"
    fi
    echo 'Starting'
    echo $FINALURL

    cupsctl --remote-admin --remote-any

    # Tell the container that DBUS should report to Host OS
    export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket

    # Set rules for connection only through the public device URL
    iptables -A INPUT -i resin-vpn -m state --state ESTABLISHED,RELATED -j ACCEPT
    iptables -A INPUT -p tcp --dport 80 -i resin-vpn -j ACCEPT
    iptables -A INPUT -p tcp --dport 80 -j REJECT


    sleep 60;
done

wait
