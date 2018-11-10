package logswriteinfluxdb

import (
	"fmt"
	"os"
	"strings"
	"time"

	"../../../../protobuf/rem_go_pb"

	client "github.com/influxdata/influxdb/client/v2"
)

// CreateDatabase with a query
func CreateDatabase() {
	// Make client
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: os.Getenv("INFLUX_URL"),
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
	}
	defer c.Close()

	q := client.NewQuery("CREATE DATABASE respirmesh", "", "")
	if response, err := c.Query(q); err == nil && response.Error() == nil {
		fmt.Println(response.Results)
	}
}

// ClientPing Ping the cluster using the HTTP client
func ClientPing() bool {
	// Make client
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: os.Getenv("INFLUX_URL"),
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
		return false
	}
	defer c.Close()

	_, _, err = c.Ping(0)
	if err != nil {
		fmt.Println("!!!!!!!!!!!!!! Error pinging InfluxDB Cluster: ", err.Error())
		return false
	}

	// fmt.Println("!!!!!!!!!!!!!! SUCCESS pinging InfluxDB Cluster: ")
	return true

}

// WriteLog hold comment
func WriteLog(remLog *rem.RespirMeshLog) {
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: os.Getenv("INFLUX_URL"),
	})
	if err != nil {
		fmt.Println("Error creating InfluxDB Client: ", err.Error())
		return
	}
	defer c.Close()

	// ssssss := reflect.ValueOf(remLog).Elem()
	// typeOfT := ssssss.Type()
	// for iiiii := 0; iiiii < ssssss.NumField(); iiiii++ {
	// 	fffff := ssssss.Field(iiiii)
	// 	fmt.Printf("%d: %s %s = %v\n", iiiii,
	// 		strings.Split(typeOfT.Field(iiiii).Tag.Get("json"), ",")[0], fffff.Type(), fffff.Interface())
	// 	// typeOfT.Field(iiiii).Name, fffff.Type(), fffff.Interface())
	// }

	// Fields:

	// procid (string)
	// severity_code (integer)
	// timestamp (integer)
	// version (integer)

	// Create a point and add to batch
	tags := map[string]string{
		// "appname":   "respirmesh",
		// "facility":  "console",
		"source_id": fmt.Sprint(remLog.GetSourceId()),
		"host":      fmt.Sprint(remLog.GetSourceId()),
		// "hostname": fmt.Sprint(remLog.GetSourceId()),
		"severity": remLog.GetSeverity().String(),
	}

	fields := map[string]interface{}{
		// "facility_code":    14,
		// "version":          1,
		// "procid":           "123",
		"source_timestamp": remLog.GetSourceTimestamp(),
		"severity_code":    rem.Severity_value[remLog.GetSeverity().String()],
		"message":          strings.TrimSpace(remLog.GetMessage()),
	}

	pt, err := client.NewPoint("syslog", tags, fields, time.Now())
	if err != nil {
		fmt.Println("Error: ", err.Error())
	}

	// Create a new point batch
	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{})
	bp.SetDatabase("respirmesh")
	bp.SetPrecision("ms")

	bp.AddPoint(pt)
	// Write the batch
	fmt.Printf("|||||| LOG  : %s \n", pt)
	c.Write(bp)

}
