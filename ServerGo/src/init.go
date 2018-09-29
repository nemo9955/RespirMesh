package main

import (
	"fmt"
	"html/template"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"./RespirMeshServer"
	"github.com/gorilla/websocket"
)

var server *remserver.RmServer = nil

// var server = remserver.New("0.0.0.0:9995", 60*time.Second)

func main() {
	var quit = make(chan bool)

	server = remserver.New("0.0.0.0:9995", 60*time.Second)
	log.Println("Respir Mesh server created ", server)
	server.StartRemServer()
	log.Println("Respir Mesh server started ", server)

	cwd, _ := os.Getwd()
	fmt.Println("In PWD, looking for ", filepath.Join(cwd, "../res/web/tmpl/"))
	http.HandleFunc("/", hh_root)
	http.HandleFunc("/websockets", hh_websocket)
	// http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("/home/Casper/public/client/index/"))))
	go http.ListenAndServe(":9996", nil)

	fmt.Printf("%s", (server.RemTopo.AssembleJSON()))

	<-quit
}

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func hh_websocket(w http.ResponseWriter, r *http.Request) {
	conn, _ := upgrader.Upgrade(w, r, nil) // error ignored for sake of simplicity
	oldJSON := ""
	newJSON := ""
	for {
		newJSON = fmt.Sprintf("%s", (server.RemTopo.AssembleJSON()))

		if newJSON != oldJSON {
			oldJSON = newJSON
			if err := conn.WriteJSON(oldJSON); err != nil {
				return
			}
		}
		time.Sleep(2 * time.Second)
	}

	// for {
	// 	// Read message from browser
	// 	_, msg, err := conn.ReadMessage()
	// 	if err != nil {
	// 		return
	// 	}

	// 	// Print the message to the console
	// 	fmt.Printf("%s sent: %s\n", conn.RemoteAddr(), string(msg))

	// 	msg = append(msg, " and back"...)

	// 	// Write message back to browser
	// 	// if err = conn.WriteMessage(msgType, msg); err != nil {
	// 	// 	return
	// 	// }
	// 	if err = conn.WriteJSON(fmt.Sprintf("%s", (server.RemTopo.AssembleJSON()))); err != nil {
	// 		return
	// 	}
	// }
}

func hh_root(w http.ResponseWriter, r *http.Request) {
	cwd, _ := os.Getwd()
	t, _ := template.ParseFiles(filepath.Join(cwd, "../res/web/tmpl/index.html")) // Parse template file.
	// t.Execute(w, server.RemTopo.AssembleJSON())                                    // merge.
	t.Execute(w, struct {
		Json string
	}{
		Json: fmt.Sprintf("%s", (server.RemTopo.AssembleJSON())),
	}) // merge.

}
