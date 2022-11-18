FROM debian:latest

RUN apt-get update && apt-get install -y build-essential \
	# https://stackoverflow.com/questions/39901311/docker-ubuntu-bash-ping-command-not-found
	iputils-ping

COPY ./srcs /ft_ping

WORKDIR /ft_ping

RUN make re