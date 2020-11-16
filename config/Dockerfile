FROM python:3.8

RUN apt-get update
RUN apt-get install -y python3 python3-pip python3-dev git libssl-dev libffi-dev build-essential
RUN python -m pip install --upgrade pip
RUN python -m pip install --upgrade pwntools
ADD ./ /root/

ENTRYPOINT [ "/bin/bash" ]