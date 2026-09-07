FROM alpine:3.22 AS builder

WORKDIR /app

RUN apk add --no-cache build-base upx

COPY Makefile ./
COPY headers ./headers
COPY src ./src

RUN make && upx --best --lzma ./yaws

FROM alpine:3.22

RUN apk add --no-cache libstdc++ \
	&& addgroup -S yaws \
	&& adduser -S -G yaws yaws

WORKDIR /app
COPY --from=builder /app/yaws ./yaws
COPY public ./public
COPY routes.conf ./routes.conf

USER yaws
EXPOSE 8080

ENTRYPOINT ["/app/yaws"]
CMD ["--port", "8080"]