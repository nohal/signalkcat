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
Build signalkcat executable by invoking ```make```
If you want to install the tool, invoke ```sudo make install```

### Mac OS X
You must have libwebsockets installed, the easiest is to get it from Homebrew using ```brew install libwebsockets```
Build signalkcat executable by invoking ```make```
If you want to install the tool, invoke ```make install```

### MS Windows
TBD
