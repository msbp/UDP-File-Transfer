# UDP File Transfer

A server side application that listens for requests from clients and transfers requested files via the UDP protocol. A client side application that sends a request for a file to a server and receives the file.

## Getting Started
Sample text files are provided in the ```tests``` folder in the Server application.
Start by compiling the files.

### In the Server machine
```
gcc ufs.c -o ufs
```
### In the Client machine
```
gcc ufc.c -o ufc
```
