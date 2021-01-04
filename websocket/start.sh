export DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket
sleep 10
rm -rf /usr/src/app/build/downloads/balena-cups-2.0.0
echo 'Setting PM2 logsize'
pm2 set pm2-logrotate:max_size 1K
echo 'Starting PM2'
pm2 start /usr/src/app/build/terminal.js
echo 'Started'
while : ; do echo "${MESSAGE=Idling...}"; sleep ${INTERVAL=600}; done

