package main

import (
	"bufio"
	"bytes"
	"compress/flate"
	"compress/zlib"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strings"
)

var cryptKey = "2589213f0c51583dcbaacbe0005e5908"

// encryption

// Encrypt will encrypt a byte stream
func Encrypt(plaintext []byte, clientIv string) ([]byte, error) {
	key, _ := hex.DecodeString(cryptKey)

	block, err := aes.NewCipher(key)
	if err != nil {
		panic(err)
	}

	plaintext, err = pkcs7pad(plaintext, aes.BlockSize) // BlockSize = 16
	if err != nil {
		return nil, err
	}

	// The IV needs to be unique, but not secure. Therefore it's common to
	// include it at the beginning of the ciphertext.
	ciphertext := make([]byte, aes.BlockSize+len(plaintext))

	// check if we have an iv to use
	var iv []byte
	if clientIv != "" {
		iv, err = hex.DecodeString(clientIv)
		if err != nil {
			return nil, err
		}
		for i := 0; i < aes.BlockSize; i++ {
			ciphertext[i] = iv[i]
		}
	} else {
		iv = ciphertext[:aes.BlockSize]
		if _, err := io.ReadFull(rand.Reader, iv); err != nil {
			return nil, err
		}
	}

	hx := hex.EncodeToString([]byte(key))
	fmt.Printf(" i (encryption key):	%s\n", hx)
	hx = hex.EncodeToString([]byte(iv))
	fmt.Printf(" i (encryption iv):	%s\n", hx)
	hx = hex.EncodeToString(plaintext)
	fmt.Printf(" i (padded plaintext):	%s\n", hx)

	stream := cipher.NewCBCEncrypter(block, iv)
	stream.CryptBlocks(ciphertext[aes.BlockSize:], plaintext)

	return ciphertext, nil
}

// pkcs7pad add pkcs7 padding
// https://gist.github.com/nanmu42/b838acc10d393bc51cb861128ce7f89c
func pkcs7pad(data []byte, blockSize int) ([]byte, error) {

	if blockSize < 0 || blockSize > 256 {
		return nil, fmt.Errorf("pkcs7: Invalid block size %d", blockSize)
	}

	padLen := blockSize - len(data)%blockSize
	padding := bytes.Repeat([]byte{byte(padLen)}, padLen)

	return append(data, padding...), nil
}

// ZlibWrite data to a Writer
func ZlibWrite(w io.Writer, data []byte) error {
	wr, err := zlib.NewWriterLevel(w, flate.BestCompression)
	defer wr.Close()
	wr.Write(data)

	return err
}

func doencrypt(iv string) {
	fmt.Println(" i were going to encrypt -> compress input you provide")

	data, err := inputReader()
	if err != nil {
		fmt.Printf("! error reading input: %s", err)
		return
	}

	// encode source data as hex repr
	hx := hex.EncodeToString([]byte(data))
	fmt.Printf(" i (input as hex):	%s\n", hx)

	// encrypt
	// Encrypt the data
	enc, err := Encrypt([]byte(data), iv)
	if err != nil {
		fmt.Printf("! error encrypting input: %s\n", err)
		return
	}
	hx = hex.EncodeToString(enc)
	fmt.Printf(" i (input encrypted):	%s\n", hx)

	// deflate
	var deflated bytes.Buffer
	ZlibWrite(&deflated, enc)
	hx = hex.EncodeToString(deflated.Bytes())
	fmt.Printf(" i (input defalted): 	%s\n", hx)
}

// decryption

// UnzlibWrite data to a Writer
func UnzlibWrite(w io.Writer, data []byte) error {
	zr, err := zlib.NewReader(bytes.NewBuffer(data))
	if err != nil {
		return err
	}

	data, err = ioutil.ReadAll(zr)
	if err != nil {
		return err
	}
	w.Write(data)

	return nil
}

// Decrypt will decrypt a byte stream
func Decrypt(ciphertext []byte) ([]byte, error) {
	key, _ := hex.DecodeString(cryptKey)

	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	// The IV needs to be unique, but not secure. Therefore it's common to
	// include it at the beginning of the ciphertext.
	if len(ciphertext) < aes.BlockSize {
		return nil, errors.New("Cipher text too short")
	}

	// Ensure we have the correct blocksize
	if (len(ciphertext) % aes.BlockSize) != 0 {
		return nil, errors.New("Cipher text is not the expected length")
	}

	iv := ciphertext[:aes.BlockSize]
	ciphertext = ciphertext[aes.BlockSize:]

	stream := cipher.NewCBCDecrypter(block, iv)
	stream.CryptBlocks(ciphertext, ciphertext)

	hx := hex.EncodeToString([]byte(key))
	fmt.Printf(" i (encryption key):	%s\n", hx)
	hx = hex.EncodeToString([]byte(iv))
	fmt.Printf(" i (encryption iv):	%s\n", hx)
	hx = hex.EncodeToString(ciphertext)
	fmt.Printf(" i (padded plaintext):	%s\n", hx)

	ciphertext, err = pkcs7strip(ciphertext, aes.BlockSize)
	if err != nil {
		return nil, err
	}

	return ciphertext, nil
}

// pkcs7strip remove pkcs7 padding
// https://gist.github.com/nanmu42/b838acc10d393bc51cb861128ce7f89c
func pkcs7strip(data []byte, blockSize int) ([]byte, error) {
	length := len(data)

	if length == 0 {
		return nil, errors.New("pkcs7: Data is empty")
	}

	if length%blockSize != 0 {
		return nil, errors.New("pkcs7: Data is not block-aligned")
	}

	padLen := int(data[length-1])
	ref := bytes.Repeat([]byte{byte(padLen)}, padLen)

	if padLen > blockSize || padLen == 0 || !bytes.HasSuffix(data, ref) {
		return nil, errors.New("pkcs7: Invalid padding")
	}

	return data[:length-padLen], nil
}

func dodecrypt() {

	fmt.Println(" i were going to decompress -> decrypt input you provide")
	fmt.Println(" i provide hex-encoded input")

	data, err := inputReader()
	if err != nil {
		fmt.Printf("! error reading input: %s", err)
		return
	}

	// decode
	dataBytes, err := hex.DecodeString(data)
	fmt.Printf(" i input len as str: %d\n", len(data))
	fmt.Printf(" i input len as hex: %d\n", len(dataBytes))

	if len(dataBytes) == 0 {
		fmt.Println(" ! decoding input hex string to bytes failed")
		return
	}

	if len(data)/len(dataBytes) == 2 {
		fmt.Println(" i inut length looks ok")
	} else {
		fmt.Println(" ! raw data is not half the bytes of data?")
		return
	}

	// decompress
	inflated := bytes.Buffer{}
	UnzlibWrite(&inflated, dataBytes)
	hx := hex.EncodeToString(inflated.Bytes())
	fmt.Printf(" i (input inflated):	%s\n", hx)

	// decrypt
	decryptData, err := Decrypt(inflated.Bytes())
	if err != nil {
		fmt.Printf(" ! error decrypting input: %s\n", err)
		return
	}

	hx = hex.EncodeToString(decryptData)
	fmt.Printf(" i (input decrypted):	%s\n", hx)
	fmt.Printf(" i (input ascii):	%s\n", string(decryptData))

}

func inputReader() (string, error) {

	scn := bufio.NewScanner(os.Stdin)
	var lines []string

	fmt.Printf(" ? (stop input reading with a ^] (ctrl-]) on the last line)\n")
	fmt.Printf(" ? input: ")
	for scn.Scan() {
		line := scn.Text()
		if len(line) == 1 {
			// Group Separator (GS ^]): ctrl-]
			if line[0] == '\x1D' {
				break
			}
		}
		lines = append(lines, line)
	}

	if err := scn.Err(); err != nil {
		return "", err
	}

	return strings.Join(lines, ""), nil
}

func main() {
	fmt.Printf(" i godoh client format helper\n")

	encPtr := flag.Bool("encrypt", false, "perform encryption on an input")
	decPtr := flag.Bool("decrypt", false, "perform decryption on an input")
	ivPtr := flag.String("iv", "", "a hex encoded iv to use instead of a random one")
	flag.Parse()

	if *encPtr && *decPtr {
		fmt.Println(" ! choose only one of -encrypt or -decrypt")
		return
	}

	if !*encPtr && !*decPtr {
		fmt.Println(" ! no option provided")
		return
	}

	if *encPtr {
		doencrypt(*ivPtr)
	}

	if *decPtr {
		dodecrypt()
	}

	fmt.Println(" i done")

}
