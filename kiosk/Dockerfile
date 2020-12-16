FROM balenablocks/browser:fincm3

WORKDIR /usr/src/app

COPY test_policy.json /etc/opt/chrome/policies/managed/
COPY start.sh .
ENV CLIENT_FACING '0'
ENV SHOW_CURSOR '1'
ENV DEBUG '1'
ENV KIOSK '1'
ENV PRINT_TERMINAL_ID 'f8ea744a-d959-4ae4-b3f8-4711cced2020'
ENV LAUNCH_URL 'pos-demo.entabeni.tech'
ENV DISPLAY :1
ENV HOST '97293c3'
ENV TERMINAL_PASSWORD '973595bf280d548eb8455d4f2d131561'
ENTRYPOINT ["bash", "start.sh"]
CMD ["export DISPLAY=:0"]
