# godoh c client

This is a C client for godoh.

## build info

Build using cmake from this directory.

A `build.sh` script exists to help building on macOS & Linux. Usage is:

```text
Usage: ./build.sh [build type] <domain> <jitter seconds>
 eg: ./build.sh release c2.local

 Build types can be debug; release
```

To build a **debug** release, which has verbose output and symbols to debug using domin.com as c2 with a 2 second jitter:

```text
./build.sh debug domain.com 2
```

To build a **release** build, with optimizations & symbols stripped using domain.com and the default 30 second jitter:

```text
./build.sh release domain.com
```

In both cases, the `build/` directory should have the resultant `godoh` binary.

