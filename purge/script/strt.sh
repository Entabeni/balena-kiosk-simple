# all services restart at this point
#!/bin/bash
if [ ! -z ${ENABLE_PURGE+x} ] && [ "$ENABLE_PURGE" -eq "1" ]
then
    echo "Setting purge chrontab"
    echo $PURGE_TIME
    (crontab -l; echo "${PURGE_TIME} * * * /usr/src/purge.sh") | crontab -
else
    echo "PURGE NOT ENABLED"
fi

crond -f
