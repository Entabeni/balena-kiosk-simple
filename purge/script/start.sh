# all services restart at this point
#!/bin/bash
(crontab -l; echo "${PURGE_TIME:-0 4 * * *} /usr/src/purge.sh") | crontab -

crond -f
