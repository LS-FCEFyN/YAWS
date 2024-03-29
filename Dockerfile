# Stage  1: Build the application
FROM alpine:latest AS builder

WORKDIR /app

# Install build dependencies
RUN apk add --no-cache g++ make && \
    # Clean up to reduce image size
    rm -rf /var/cache/apk/*

# Copy source code and headers
COPY Makefile /app/
COPY src /app/src
COPY headers /app/headers

# Copy the bin folder containing everything not code related
COPY bin /app/bin

# Build the application
RUN make -C /app

# Stage  2: Setup the runtime environment
FROM alpine:latest

# Install runtime dependencies
RUN apk add --no-cache libstdc++

# Set working directory
WORKDIR /app/bin

# Copy the bin folder from the builder stage
COPY --from=builder /app/bin/ .

# Expose port
EXPOSE   80

# Command to run the application
CMD ["./server"]
