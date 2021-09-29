# format helper

This tool encrypts/decrypts/inflates/deflates content using the same methods and properties as implemented in godoh. This should help you when developing clients in other languages to check that your content will be understood by the server.

## usage

Run with `go run main.go`.

The tool has two flags; `-encrypt` & `-decrypt`; hopefully self explanitory. Input strings to decrypt may be too long for your TTY to accept, in which case you should `echo "content" | go run main.go -decrypt`.

## cyberchef

A cyberchef recipe to decrypt payloads can be found [here](https://gchq.github.io/CyberChef/#recipe=From_Hex('Auto')AES_Decrypt(%7B'option':'Hex','string':'2589213f0c51583dcbaacbe0005e5908'%7D,%7B'option':'Hex','string':'ad70f4d5a04c7e7924ca62fb2ebf70af'%7D,'CBC','Raw','Raw',%7B'option':'Hex','string':''%7D,%7B'option':'Hex','string':''%7D)Take_bytes(16,100,false)&input=YWQ3MGY0ZDVhMDRjN2U3OTI0Y2E2MmZiMmViZjcwYWZjNWE2YjI4NGQxNmYyZjExNGYzYmEzOTZjOGVhMWEzMA). Note that the IV value is the first 16 bytes of the hex encoded, encrypted payload. 
