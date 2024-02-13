# Stage  1: Build the application
FROM archlinux:latest AS builder

WORKDIR /app

# Install build dependencies
RUN pacman -Syu --noconfirm gcc make && \
    # Clean up to reduce image size
    pacman -Scc --noconfirm && \
    rm -rf /var/cache/pacman/pkg

# Copy source code and headers
COPY Makefile /app/
COPY src /app/src
COPY headers /app/headers

# Build the application
RUN make -C /app

# Stage  2: Setup the runtime environment
FROM archlinux:latest

# Create a non-root user
RUN useradd -ms /bin/bash appuser

# Set working directory
WORKDIR /app/bin

# Copy the compiled binary from the builder stage
COPY --from=builder /app/bin/server /app/bin/server

# Expose port
EXPOSE  80

# Command to run the application
CMD ["./server"]
