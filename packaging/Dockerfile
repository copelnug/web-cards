FROM ubuntu:20.04

# Because otherwise installing tzdata ask a question
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -q \
	&& apt-get install -yq libboost-all-dev

RUN apt-get update -q \
	&& apt-get install -yq g++ meson ninja-build

# Build
COPY . /docker-sources/
RUN cd /docker-sources \
	&& mkdir /docker-build \
	&& meson /docker-build \
	&& cd /docker-build \
	&& ninja \
	&& meson install \
	&& cd .. \
	&& rm -rf /docker-sources /docker-build

# Create user
RUN useradd --create-home --shell /bin/bash webenfer
COPY src/web /home/webenfer

EXPOSE 8080
USER webenfer
WORKDIR /home/webenfer
ENV LD_LIBRARY_PATH=/usr/local/lib64

CMD ["server"]