# Yet Another Web Server (YAWS)
YAWS is a simple static web server written from scratch in C++. It was created as a way to gain a better understanding of the sockets API on *nix systems, as well as to explore some of the newer features introduced in C++20.

# Compiling
To compile the YAWS project, simply run the `make` command in your terminal.

# Usage
After compiling, you can start the server by running the executable named `yaws`. By default, the server listens on port `8080`. This can be changed by passing the `-p | --port` command-line argument followed by the desired port number.

Once the server is running, it can be accessed using a web browser or a tool such as cURL.

Currently, only HTTP is supported, although adding HTTPS support should be relatively straightforward.

For a complete list of available command-line arguments, run the binary with `-h | --help`.

# Routing
YAWS reads a small route table from `routes.conf`, which must be present in the same directory the server is run from (see the Dockerfile for how this is laid out in the container image). **The server will fail to start if this file is missing.**

Each line maps a request path to a target path:

    /path = /target

Blank lines and lines starting with `#` are ignored. A route for `/` is required if you want requests to the site root to resolve to a specific file, e.g.:

    / = /index.html

# MIME Types
MIME types are determined by file extension via a lookup table in `headers/http/mime.hpp`. Additional types can be added by editing that table directly.

Any file requested from the server for which no known MIME type is found will be returned with the `application/octet-stream` MIME type.

# License
YAWS is released under the [GPL-2.0 License](https://opensource.org/license/gpl-2-0/). Feel free to use, modify, and distribute this project as you see fit.

# Contributing
Contributions to the YAWS project are welcome! If you find a bug, have a suggestion, or would like to contribute to the project, feel free to submit a pull request or open an issue on [GitHub](https://github.com/LS-FCEFYN/YAWS).

Thank you for your interest in YAWS!