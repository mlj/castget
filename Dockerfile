FROM alpine:3.14

VOLUME /castget

COPY . ./build

RUN apk add --no-cache --virtual build-dependencies \
      libxml2-dev glib-dev curl-dev taglib-dev alpine-sdk autoconf automake libtool ronn \
  && apk add --no-cache libxml2 glib curl taglib \
  && cd build \
  && autoreconf -fi \
  && ./configure \
  && make \
  && make install \
  && cd ../ \
  && rm -rf build \
  && apk del build-dependencies \
  && adduser -h /castget -D -H castget

USER castget

ENTRYPOINT ["castget"]
