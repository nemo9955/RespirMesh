#include "RespirMesh.hpp"

void RespirMesh::receive_fn(uint8_t *data, uint16_t size, void *arg)
{
    logf("[ REM ] Received DATA \n");
    RemBasicHeader *header = (RemBasicHeader *)data;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    // memcpy(&header, data, offsetHeader);

    // Serial.printf("  ForwardingType %d \n", header->ForwardingType);
    // Serial.printf("      HeaderType %d \n", header->HeaderType);
    // Serial.printf("    ProtobufType %d \n", header->ProtobufType);

    uint8_t *packetData = ((uint8_t *)(data)) + offsetHeader;
    uint16_t packetDataLen = size - offsetHeader;

    if (header->ForwardingType == ForwardingType_TO_ROOT)
    {
        logf("Print shit");
         ((RespirMesh *)arg)->recv(data, size);
    }
    else if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
    {
        switch (header->ProtobufType)
        {
        case ProtobufType_MESH_TOPOLOGY:
           // Serial.printf("putting data in  TOPO to root : \n");
            ((RespirMesh *)arg)->handleMeshTopology( data, size);
            break;
        }
    }
}

void RespirMesh::add_channel(RemChannel *channel)
{
        channel->set_recv_cb(receive_fn, this);
        channels.push_back(channel);
}

void RespirMesh::send(uint8_t *data, uint16_t size)
{
    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
            (*it)->send(data, size);
    }
}

void RespirMesh::recv(uint8_t *data, uint16_t size)
{
    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
            (*it)->send(data, size);
    }
}

void RespirMesh::update()
{
        //logf(" Time: %d  \n", hardware_->time_milis());

        action_counter++;

        if (action_counter % 4 == 1)
        {
            send_mesh_topo_to_server();
        }
        // else if (action_counter % 3 == 0)
        // {
        //     send_ping_to_server();
        // }
        // else
        // {
        //     send_ping_to_server();
        // }
}

void RespirMesh::handleMeshTopology(uint8_t *data, size_t len)
    {

        for (uint8_t i = 0; i < len + 10; i++)
            printf("%d ", ((char *)data)[i]);
        printf("\n");
        for (uint8_t i = 0; i < len + 10; i++)
            printf("%d ", ((char *)pb_buffer)[i]);
        printf("\n");

        bool pb_status = false;
        uint16_t offsetHeader = sizeof(RemBasicHeader);
        uint8_t *packetData = (uint8_t *)(data) + offsetHeader;
        // uint8_t *packetData = ((uint8_t *)(data)) + offsetHeader;
        uint16_t packetDataLen = len - offsetHeader;
        memcpy(pb_buffer, data, offsetHeader);
        RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
        // header = (RemBasicHeader *)pb_buffer;

        RespirMeshInfo meshTopo;

        pb_istream_t istream = pb_istream_from_buffer(packetData, packetDataLen);
        pb_status = pb_decode(&istream, RespirMeshInfo_fields, &meshTopo);
        if (!pb_status)
        {
            printf("Decoding meshTopo failed %s with type %d\n", PB_GET_ERROR(&istream), header->ProtobufType);
            return;
        }

        for (uint8_t i = 0; i < len + 10; i++)
            printf("%d ", ((char *)pb_buffer)[i]);
        printf("\n");

        if (header->ForwardingType == ForwardingType_PARENT_TO_ROOT)
        {

            header->ForwardingType = ForwardingType_TO_ROOT;
            meshTopo.type = ProtobufType_MESH_TOPOLOGY;
            meshTopo.target_id = hardware_->device_id();

            pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
            pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

            if (!pb_status)
            {
                printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
                return;
            }

            // printf(" target_id %x \n", pingpong.target_id);
            printf("Sending completed TOPO to root %lu %d \n", ostream.bytes_written, offsetHeader);
            printf("         src %x  tar %x \n", meshTopo.source_id, meshTopo.target_id);
            printf("         Forward %d \n", header->ForwardingType);
            printf("             Hea %d \n", header->HeaderType);
            printf("           Proto %d \n", header->ProtobufType);

            for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader + 3; i++)
                printf("%d ", ((char *)pb_buffer)[i]);
            printf("\n");

            // parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
            send(pb_buffer, ostream.bytes_written + offsetHeader);
        }
    }

void RespirMesh::send_mesh_topo_to_server()
{
        RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
        uint16_t offsetHeader = sizeof(RemBasicHeader);
        header->HeaderType = HeaderType_BASIC;

        RespirMeshInfo remPingPong = RespirMeshInfo_init_default;
        remPingPong.source_id = hardware_->device_id();

        header->ForwardingType = ForwardingType_PARENT_TO_ROOT;
        header->ProtobufType = ProtobufType_MESH_TOPOLOGY;
        remPingPong.type = ProtobufType_MESH_TOPOLOGY;

        pb_ostream_t ostream = pb_ostream_from_buffer((uint8_t *)pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &remPingPong);

        if (!pb_status)
        {
            PRINTF("Encoding PingPong failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        PRINTF(" HEADER size %d \n", offsetHeader);
        PRINTF(" Protobuf size %d \n", (int)ostream.bytes_written);

        // pb_buffer[ostream.bytes_written + offsetHeader] = '\n';
        // pb_buffer[ostream.bytes_written + offsetHeader + 1] = '\r';
        // pb_buffer[ostream.bytes_written + offsetHeader + 2] = '\0';

        for (uint8_t i = 0; i < ostream.bytes_written + offsetHeader; i++)
            PRINTF("%d ", pb_buffer[i]);
        PRINTF("\n");

        send(pb_buffer, ostream.bytes_written + offsetHeader);
        // tcpParent.Send(string(((char *)pb_buffer), ostream.bytes_written + offsetHeader + 1));
        // parClient.write((const char *)(pb_buffer), ostream.bytes_written + offsetHeader);
}

void RespirMesh::sendPing()
{
        for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
             sendPingToNode((*it));
        }
}

void RespirMesh::sendPingToNode(RemChannel* c)
{
        RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
        uint16_t offsetHeader = sizeof(RemBasicHeader);
        RespirMeshTimeSync timesync=RespirMeshTimeSync_init_default;
        
	    timesync.info.type = ProtobufType_PING;
	    timesync.info.target_id = 0;
        timesync.info.source_id = hardware_->device_id();

	    header->ForwardingType = uint8_t(ForwardingType_TO_NODE);
	    header->ProtobufType = uint8_t(ProtobufType_PING);


        timesync.request_sent_time=hardware_->time_milis();
        pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        bool pb_status = pb_encode(&stream, RespirMeshTimeSync_fields, &timesync);
        if (!pb_status)
        {
            printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
            return;
        }
        c->send(( uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
}

void RespirMesh::sendPongToNode(RemChannel* c, uint8_t *data, size_t len,RemBasicHeader *header)
{
        RespirMeshTimeSync timesync;

        uint16_t offsetHeader = sizeof(RemBasicHeader);
        
        // uint32_t us_start = micros();
        pb_istream_t istream = pb_istream_from_buffer(data, len);
        bool pb_status = pb_decode(&istream, RespirMeshTimeSync_fields, &timesync);
        if (!pb_status)
        {
            printf("Decoding TimeSync failed %s with type %d\n", PB_GET_ERROR(&istream), header->ProtobufType);
            return;
        }
        timesync.request_arrive_time =hardware_->time_milis();
        header->ProtobufType = uint8_t(ProtobufType_PONG);
        timesync.info.type = ProtobufType_PONG;
        timesync.response_sent_time =hardware_->time_milis();
        pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
         pb_status = pb_encode(&stream, RespirMeshTimeSync_fields, &timesync);

        if (!pb_status)
        {
            printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
            return;
        }
        c->send(( uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
}

void RespirMesh::HandlePong(RemChannel *c, uint8_t *data, size_t len, RemBasicHeader *header)
{
        RespirMeshTimeSync timesync;
        pb_istream_t stream = pb_istream_from_buffer(data, len);
        bool pb_status = pb_decode(&stream, RespirMeshTimeSync_fields, &timesync);

        if (!pb_status)
        {
            printf("Decoding PingPong failed %s with type %d\n", PB_GET_ERROR(&stream), header->ProtobufType);
            return ;
        }
        timesync.request_arrive_time =hardware_->time_milis();
        timesync.info.type=ProtobufType_TIMESYNC;
        header->ForwardingType = uint8_t(ForwardingType_PARENT_TO_ROOT);
}