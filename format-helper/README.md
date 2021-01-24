# format helper

This tool encrypts/decrypts/inflates/deflates content using the same methods and properties as implemented in godoh. This should help you when developing clients in other languages to check that your content will be understood by the server.

## usage

Run with `go run main.go`.

The tool has two flags; `-encrypt` & `-decrypt`; hopefully self explanitory. Input strings to decrypt may be too long for your TTY to accept, in which case you should `echo "content" | go run main.go -decrypt`.

