# Demonstrative Dockerfile for C development.

FROM alpine:3.17 as dev

ARG UID=1000
ARG GID=1000

# UID+GID to map the host user onto the container user, so as to avoid
#    file permissions issues
# bash for comfort
# musl-dev musl-dbg gcc gdb make for c development
# sudo for experimenting with new packages in the image, without having
#     to wait for image rebuild
# linenoise-dev, for compiled sqlite3 shell (although, could apk add it)

RUN apk update \
	&& apk add bash musl-dev musl-dbg gcc gdb make sudo \
	&& addgroup -g ${GID} myproject-dev \
	&& adduser -D -u ${UID} -G myproject-dev myproject-dev \
	&& install -d -m 0755 -o myproject-dev -g myproject-dev /root/myproject \
	&& echo 'myproject-dev ALL=NOPASSWD:ALL' > /etc/sudoers.d/wheel
USER myproject-dev

ADD ./ /home/myproject-dev/myproject

# gdb quit confirmation prompt is a little annoying
ADD tools/docker/.gdbinit /home/myproject-dev/.gdbinit

WORKDIR /home/myproject-dev/myproject

FROM dev as build
USER root
RUN make install

FROM alpine:3.17 as release
COPY --from=build /usr/share/myproject/ /usr/share/myproject/
COPY --from=build /usr/local/bin/myproject /usr/local/bin/myproject
COPY --from=build /var/lib/myproject/ /var/lib/myproject/
ENV LISTENADDR=0.0.0.0:80
ENV DATABASE=/var/lib/myproject/db
ENV LOGLEVEL=info
ENV LOGCOLOR=false
CMD [ "/usr/local/bin/myproject" ]

