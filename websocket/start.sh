export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket
# redis-server
sleep 10
echo 'Starting PM2'
pm2 start /usr/src/app/build/terminal.js
# echo 'Seems wittyPi is installed already, skip this step.4'

# pm2 logs
# #!/usr/bin/env bash
# [ -z $BASH ] && { exec bash "$0" "$@" || exit; }
# #!/bin/bash


# # target directory
# DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/wittyPi"

# # error counter
# ERR=0

# echo '================================================================================'
# echo '|                                                                              |'
# echo '|                   Witty Pi Software Installing Script                        |'
# echo '|                                                                              |'
# echo '================================================================================'



# # install wittyPi
# if [ $ERR -eq 0 ]; then
#   echo '>>> Install wittyPi'
#   if [ -f wittyPi ]; then
#     echo 'Seems wittyPi is installed already, skip this step.'
#   else
#     wget http://www.uugear.com/repo/WittyPi/LATEST -O wittyPi.zip || ((ERR++))
#     unzip wittyPi.zip -d wittyPi || ((ERR++))
#     cd wittyPi
#     chmod +x wittyPi.sh
#     chmod +x daemon.sh
#     chmod +x syncTime.sh
#     chmod +x runScript.sh
#     sed -e "s#/home/pi/wittyPi#$DIR#g" init.sh >/etc/init.d/wittypi
#     chmod +x /etc/init.d/wittypi
#     update-rc.d wittypi defaults
#     cd ..
#     sleep 2
#     rm wittyPi.zip
#   fi
# fi

# echo
# if [ $ERR -eq 0 ]; then
#   echo '>>> All done. Please reboot your Pi :-)'
# else
#   echo '>>> Something went wrong. Please check the messages above :-('
# fi

while : ; do echo "${MESSAGE=Idling...}"; sleep ${INTERVAL=600}; done
