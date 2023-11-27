FROM alpine:latest

WORKDIR /app

RUN apk update && apk add gcc make musl-dev

COPY Makefile /app/
COPY src /app/src
COPY headers /app/headers
COPY bin /app/bin

RUN make -C /app

EXPOSE 80

WORKDIR /app/bin

CMD ["./server"]