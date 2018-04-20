package main

import (
	"log"
	"net"
	"strconv"
	"strings"
	"time"
)

const (
	message       = "Ping"
	StopCharacter = "\r\n\r\n"
)

func SocketClient(ip string, port int) {
	addr := strings.Join([]string{ip, strconv.Itoa(port)}, ":")
	conn, err := net.Dial("tcp", addr)

	defer conn.Close()

	if err != nil {
		log.Fatalln(err)
	}

	conn.Write([]byte(message))
	conn.Write([]byte(StopCharacter))
	log.Printf("Send: %s", message)

	buff := make([]byte, 1024)
	n, _ := conn.Read(buff)
	log.Printf("Receive: %s", buff[:n])

}

func main() {

	var (
		ip   = "127.0.0.1"
		port = 9995
	)
	spamNr := 100
	for i := 0; i < spamNr; i++ {
		go SocketClient(ip, port)
		time.Sleep(10 * time.Millisecond)
	}
	time.Sleep(500 * time.Millisecond)

}
