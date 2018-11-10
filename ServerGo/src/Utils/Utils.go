package utils

import (
	"encoding/json"
	"fmt"
)

func PrettyPrint(msg string, v interface{}) (err error) {
	b, err := json.MarshalIndent(v, "", "  ")
	if err == nil {
		fmt.Println(msg, string(b))
	}
	return
}
