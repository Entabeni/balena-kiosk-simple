FROM balenablocks/browser:fincm3

WORKDIR /usr/src/app

COPY start.sh .
ENV CLIENT_FACING '0'
ENV PRINT_TERMINAL_ID 'f8ea744a-d959-4ae4-b3f8-4711cced2020'
ENV LAUNCH_URL 192.168.0.45:3000
ENV TERMINAL_PASSWORD '2ad58f3bda6a71357c5ca4c80bcac37b'
ENTRYPOINT ["bash", "start.sh"]
CMD ["export DISPLAY=:0"]
