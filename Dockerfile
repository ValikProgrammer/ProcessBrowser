FROM gcc:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    make \
    libncurses-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the project
RUN make clean && make

# Build tests
RUN gcc -o tests/test_sort tests/test_sort.c src/sort.c -Isrc/include -Wall -Wextra
RUN gcc -o tests/test_kill tests/test_kill.c -Wall -Wextra

# Run tests
CMD echo "Running unit tests..." && \
    ./tests/test_sort && \
    echo "" && \
    echo "Running integration tests..." && \
    ./tests/test_kill && \
    echo "" && \
    echo "All tests passed successfully!"

