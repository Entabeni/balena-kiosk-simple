#!/bin/sh
echo "Purging data"
curl -X POST --header "Content-Type:application/json" "$BALENA_SUPERVISOR_ADDRESS/v2/applications/$BALENA_APP_ID/purge?apikey=$BALENA_SUPERVISOR_API_KEY"
