FROM archlinux:latest

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

RUN useradd -ms /bin/bash appuser

# Set working directory
WORKDIR /app/bin

# Expose port
EXPOSE  80

# Command to run the application
CMD ["./server"]
