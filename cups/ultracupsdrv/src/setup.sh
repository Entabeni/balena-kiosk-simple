#!/bin/sh

echo "Magicard Ltd"
echo "ultracupsdrv-1.0.16 installer"
echo "---------------------------------------"
echo ""
echo "Models supported:"
echo ""
echo "Magicard Enduro"
echo "Magicard Rio Pro"
echo "Magicard Rio Pro Xtended"
echo "Magicard Pronto"
echo "Magicard Enduro+"
echo "Magicard MC200"
echo "Magicard Helix"
echo "Magicard Enduro3E"
echo "Magicard Enduro3ELE"
echo "Magicard Rio Pro 360"
echo "IDville ID Maker"
echo "Intersider Authentys"
echo "Intersider Authentys Pro"
echo "Intersider Authentys Plus"
echo "Authentys Retrax"
echo "Polaroid P2500S"
echo "Polaroid P4500S"
echo "Polaroid P4500S+"
echo "PPC ID 2000"
echo "PPC ID 2100"
echo "PPC ID 2300"
echo "PPC ID 2350e"
echo "PPC ID 3100"
echo "PPC ID 3300 "
echo "PPC RTP 7500w"
echo "ID Maker Value"
echo "ID Maker Advantage"
echo "ID Maker Secure"
echo "ID Maker Advantage+"
echo "Aisino JKE700C"
echo "Aisino JKE160C"
echo "Fagoo P310e"
echo "Secuprinter S200"
echo "Identity Pro"
echo "DoH"
echo "Ying ING171"
echo "Pridento"
echo "Pridento+"
echo "Pridento Pro"
echo "Pridento Pro Xtended"
echo "Pridento Open Data"
echo "Pridento+ Open Data"
echo "IDentilam DOPPIE 100"
echo "IDentilam DOPPIE 200"
echo "IDentilam DOPPIE 300"
echo "IDville ID Maker"
echo "ilinkcard iT-2600"
echo "Aisino JKE701C"
echo "Orphicard Duplet"
echo "PT Smart P-Card"
echo "Matica M300"
echo "Matica M500"
echo "Doppie 100"
echo "Doppie 200"
echo "Doppie 300"
echo "Authentys Plus"
echo "Authentys Pronto"
echo "Authentys Pro"
echo "Authentys Retrax"
echo "ScreenCheck SC2500"
echo "ScreenCheck SC4500"
echo "ScreenCheck SC6500"
echo "ScreenCheck SC7500"
echo "PriceCardPro LITE"
echo "PriceCardPro DUO"
echo "PriceCardPro FLEX"
echo "AlphaCard PRO 700"
echo "AlphaCard Compass"
echo "AlphaCard PRO 500"
echo "AlphaCard Pilot"
echo "AlphaCard PRO 100"
echo "AlphaCard PRO 100"
echo "Magicard Rio Pro 360"
echo "PriceCardPro LITE"
echo "PriceCardPro DUO"
echo "PriceCardPro FLEX"
echo "Magicard Helix RT"
echo "Magicard Rio Pro 360 Xtd"
echo "DTP MK I"
echo "DTP MK II"
echo "DTP MK III"
echo "Titan T3"
echo "Titan T5"
echo "Elliaden Caroline"
echo "ID Maker Secure 360"
echo "Authentys Pro 360"
echo "AlphaCard PRO 700"
echo "DTP MK III"
echo "Magicard Rio Pro 360 LE"
echo "ScreenCheck SC6500"
echo "Titan T5 360"
echo "PPC ID 3350e"
echo "Pridento Pro 360"
echo "Magicard Rio Pro Select"
echo "Pridento Pro 360 Xtended"
echo "PriceCardPro FLEX"
echo ""

ROOT_UID=0

if [ -z $RPMBUILD ] && [ "$UID" -ne "$ROOT_UID" ]
then
    echo "This script requires root user access."
    echo "Re-run as root user."
    exit 1
fi

if [ ! -z $DESTDIR ]
then
    echo "DESTDIR set to $DESTDIR"
    echo ""
fi

SERVERROOT=$(grep '^ServerRoot' /etc/cups/cupsd.conf | awk '{print $2}')

if [ -z $FILTERDIR ] || [ -z $PPDDIR ]
then
    echo "Searching for ServerRoot, ServerBin, and DataDir tags in /etc/cups/cupsd.conf"
    echo ""

    if [ -z $FILTERDIR ]
    then
        SERVERBIN=$(grep '^ServerBin' /etc/cups/cupsd.conf | awk '{print $2}')

        if [ -z $SERVERBIN ]
        then
            echo "ServerBin tag not present in cupsd.conf - using default"
            FILTERDIR=usr/lib/cups/filter
        elif [ ${SERVERBIN:0:1} = "/" ]
        then
            echo "ServerBin tag is present as an absolute path"
            FILTERDIR=$SERVERBIN/filter
        else
            echo "ServerBin tag is present as a relative path - appending to ServerRoot"
            FILTERDIR=$SERVERROOT/$SERVERBIN/filter
        fi
    fi

    echo ""

    if [ -z $PPDDIR ]
    then
        DATADIR=$(grep '^DataDir' /etc/cups/cupsd.conf | awk '{print $2}')

        if [ -z $DATADIR ]
        then
            echo "DataDir tag not present in cupsd.conf - using default"
            PPDDIR=/usr/share/cups/model/ultra
        elif [ ${DATADIR:0:1} = "/" ]
        then
            echo "DataDir tag is present as an absolute path"
            PPDDIR=$DATADIR/model/ultra
        else
            echo "DataDir tag is present as a relative path - appending to ServerRoot"
            PPDDIR=$SERVERROOT/$DATADIR/model/ultra
        fi
    fi

    echo ""

    echo "ServerRoot = $SERVERROOT"
    echo "ServerBin  = $SERVERBIN"
    echo "DataDir    = $DATADIR"
    echo ""
fi

echo "Copying rastertoultra filter to $DESTDIR/$FILTERDIR"
mkdir -p $DESTDIR/$FILTERDIR
chmod +x rastertoultra
cp rastertoultra $DESTDIR/$FILTERDIR
echo ""

echo "Copying cmdtoultra to $DESTDIR/$FILTERDIR"
mkdir -p $DESTDIR/$FILTERDIR
chmod +x cmdtoultra
cp cmdtoultra $DESTDIR/$FILTERDIR
echo ""

echo "Copying model ppd files to $DESTDIR/$PPDDIR"
mkdir -p $DESTDIR/$PPDDIR
cp *.ppd $DESTDIR/$PPDDIR
echo ""

if [ -z $RPMBUILD ]
then
    echo "Restarting CUPS"
    if [ -x /etc/software/init.d/cups ]
    then
        /etc/software/init.d/cups stop
        /etc/software/init.d/cups start
    elif [ -x /etc/rc.d/init.d/cups ]
    then
        /etc/rc.d/init.d/cups stop
        /etc/rc.d/init.d/cups start
    elif [ -x /etc/init.d/cups ]
    then
        /etc/init.d/cups stop
        /etc/init.d/cups start
    elif [ -x /sbin/init.d/cups ]
    then
        /sbin/init.d/cups stop
        /sbin/init.d/cups start
    elif [ -x /etc/software/init.d/cupsys ]
    then
        /etc/software/init.d/cupsys stop
        /etc/software/init.d/cupsys start
    elif [ -x /etc/rc.d/init.d/cupsys ]
    then
        /etc/rc.d/init.d/cupsys stop
        /etc/rc.d/init.d/cupsys start
    elif [ -x /etc/init.d/cupsys ]
    then
        /etc/init.d/cupsys stop
        /etc/init.d/cupsys start
    elif [ -x /sbin/init.d/cupsys ]
    then
        /sbin/init.d/cupsys stop
        /sbin/init.d/cupsys start
    else
        echo "Could not restart CUPS"
    fi
    echo ""
fi

echo "Install Complete"
echo "Plug in the printer and connect the printer via USB and visit"
echo "http://localhost:631/Printers. If the printer is not listed"
echo "add the printer queue manually at http://localhost:631"
echo ""

