FROM balenablocks/browser:fincm3

WORKDIR /usr/src/app

COPY start.sh .
ENV LAUNCH_URL http://pos-demo.entabeni.tech/
ENTRYPOINT ["bash", "start.sh"]
CMD ["export DISPLAY=:0"]
