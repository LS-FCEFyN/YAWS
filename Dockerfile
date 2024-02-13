# Stage 1: Build the application
FROM alpine:latest AS builder

WORKDIR /app

RUN apk update && apk add g++ make musl-dev

COPY Makefile /app/
COPY src /app/src
COPY headers /app/headers

RUN make

# Stage 2: Create the production image
FROM alpine:latest

WORKDIR /app

COPY --from=builder /app/bin /app/bin

EXPOSE 80

CMD ["cd ./bin/"]
CMD ["./bin/server"]
