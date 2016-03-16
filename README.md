# SignalKCat

Tool to dump data served by a SignalK websocket server.

## Usage:
```
signalkcat <server host> [--port=<p>] [--ssl] [-k] [-v <ver>] [-d <log bitfield>] [-l]
```

### Minimal usage example:
```
signalkdump localhost
```

### TLS support:
```
signalkdump localhost --ssl
```

To get additional diagnosting information, invoke with a high value for the ```-d``` parameter, like 1023.

## Building

### Linux
You must have libwebsockets installed, on Debian based distributions, use ```sudo apt-get install libwebsockets```.
The build was tested with libwebsockets version 1.6 and 1.7, version 1.2.2 (as in Jessie and all but recent Ubuntus) is known not to work at this moment. You may try the packages from https://launchpad.net/~acooks/+archive/ubuntu/libwebsockets6/+packages
Build signalkcat executable by invoking ```make```
If you want to install the tool, invoke ```sudo make install```

### Mac OS X
You must have libwebsockets installed, the easiest is to get it from Homebrew using ```brew install libwebsockets```
Build signalkcat executable by invoking ```make```
If you want to install the tool, invoke ```make install```

### MS Windows
TBD
