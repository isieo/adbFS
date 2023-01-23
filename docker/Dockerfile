# FROM ubuntu:22.04 as buildenv
# RUN apt install -y build-essential git pkg-config
# RUN apt install -y libfuse-dev fuse
# ADD ./* /src/
# WORKDIR /src
# RUN make


FROM budtmo/docker-android-x86-10.0
RUN apt update

RUN apt install -y build-essential git pkg-config

RUN apt install -y fuse libfuse-dev

ADD ./* /src/
ADD ./docker/* /src/docker/

WORKDIR /src
RUN make

RUN chmod +x /src/docker/run-docker-test.sh

WORKDIR /root
CMD /src/docker/run-docker-test.sh


# COPY --from=buildenv /src/adbfs /usr/bin/adbfs
# RUN chmod +x /usr/bin/adbfs






