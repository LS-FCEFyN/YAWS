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

# Copy the bin folder containing everything not code related
COPY bin /app/bin

# Build the application
RUN make -C /app

# Stage  2: Setup the runtime environment
FROM archlinux:latest

# Create a non-root user
RUN useradd -ms /bin/bash appuser

# Set working directory
WORKDIR /app/bin

# Copy the bin folder from the builder stage
COPY --from=builder /app/bin/ /app/

# Expose port
EXPOSE  80

# Switch to the non-root user
USER appuser

# Command to run the application
CMD ["./server"]