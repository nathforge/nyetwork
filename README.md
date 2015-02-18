Simple method to log network connections and DNS lookups for a given app. Requires Linux.

Have used this to track down network connections made within a test suite.

Example usage:

```
$ NY_LOG=access.log LD_PRELOAD=`pwd`/nyetwork.so firefox
```

Using Firefox to access google.co.uk might result in the following `access.log`:

```
Conn: 127.0.1.1:53
DNS: 74.125.136.94 is www.google.co.uk
Conn: 74.125.136.94:80
```
