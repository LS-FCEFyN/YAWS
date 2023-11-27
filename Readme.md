# Yet Another Web Server (YAWS)
YAWS is a simple static web server written in C++ from scratch, created as a means to gain knowledge on how to use the sockets API on *nix systems and also gain some knowledge about the new features available in C++20.

# Compiling
To compile the YAWS project, simply run the __make__ command in your terminal.

# Usage
After compiling, you can start the server by running the executable file named "server". Once running, the server will listen on port __80__ by default, this can be changed modifying __main.cpp__, and you can access it using your web browser or a tool like __cURL__. Only HTTP currently supported, although HTTPS is straight forward to implement.

# Routing
Routing is implemented using an unordered map, routes are to be added to __routes.conf__ located at the __bin__
directory. The format of the config file is quite simple, it's just a key-value pair; where the key
is the route and the value is the file path.
For example:
> / /index.html

Points to:
> /public/index.html

# Mime types
More mime types can be added modifying the file __mime__ any file requested to the server that does not have a known mime type will be returned with the following type __application/octet-stream__

# License
YAWS is released under the [GPL-2.0 License](https://opensource.org/license/gpl-2-0/). Feel free to use, modify, and distribute this project as you see fit.

# Contributing
Contributions to the YAWS project are welcome! If you find a bug, have a suggestion, or would like to contribute to the project, please feel free to submit a pull request or open an issue on [GitHub](https://github.com/LS-FCEFYN/YAWS).

Thank you for your interest in YAWS!