export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket
sleep 10
echo 'Starting PM2'
pm2 start /usr/src/app/build/terminal.js
echo 'Started'

while : ; do echo "${MESSAGE=Idling...}"; sleep ${INTERVAL=600}; done

