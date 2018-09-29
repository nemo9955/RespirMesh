#include "RespirMesh.hpp"

uint32_t RespirMesh::time_milis()
{
    return hardware_->time_milis();
}

uint32_t RespirMesh::device_id()
{
    return hardware_->device_id();
}

void RespirMesh::set_hardware(Hardware *_hard)
{
    hardware_ = _hard;
}

void RespirMesh::receive_fn(uint8_t *data, uint16_t size, void *arg)
{

    logf("data  receive_fn :  ");
    for (uint8_t i = 0; i < size; i++)
        logf("%d ", data[i]);
    logf("\n");

    RespirMesh *rm = (RespirMesh *)arg;
    (rm)->route_packet(data, size);
}

void RespirMesh::add_channel(RemChannel *channel)
{
    channel->set_receiver(receive_fn, this);
    logf("ADDED ch %d , CHs size  %d  \n", channel->ch_info(), channels.size() + 1);
    channels.push_back(std::move(channel));
    clean_channels();
}

void RespirMesh::clean_channels()
{

    for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->is_ok() == false)
        {
            logf("DELETING CHANNEL  %d  \n", (*it)->ch_info());
            (*it)->stop();
            channels.erase(it++);
        }
    }
}

void RespirMesh::send_packet(uint8_t *data, uint16_t size)
{

    logf("data  send_packet :  ");
    for (uint8_t i = 0; i < size; i++)
        logf("%d ", data[i]);
    logf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;
    switch (header->ForwardingType)
    {

    case ForwardingType_PARENT_AND_BACK:
    case ForwardingType_TO_PARENT:
    case ForwardingType_TO_PARENT_TO_ROOT:
    case ForwardingType_TO_ROOT:
        for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
            if ((*it)->connected_to_root)
            {
                // logf(" P/TO_ROOT  \n");
                (*it)->send(data, size);
                //logf(" sendd ch %d   \n", (*it)->ch_info());
            }
        break;

    case ForwardingType_TO_NEIGHBORS:
    case ForwardingType_NEIGHBOR_TO_ROOT:
        // logf(" NEIGHBOR   \n");
        for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
            (*it)->send(data, size);
        break;
    }
}

void RespirMesh::route_packet(uint8_t *data, uint16_t size)
{
    // logf(" RECV   \n");

    logf("data  route_packet :  ");
    for (uint8_t i = 0; i < size; i++)
        logf("%d ", data[i]);
    logf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;

    switch (header->ForwardingType)
    {

    case ForwardingType_TO_ROOT:
    {
        for (std::list<RemChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            if ((*it)->connected_to_root)
                (*it)->send(data, size);
        }
        break;
    }
    case ForwardingType_PARENT_AND_BACK:
    case ForwardingType_TO_PARENT:
    case ForwardingType_TO_PARENT_TO_ROOT:
    {
        process_packet(data, size);
        break;
    }
    }
}

void RespirMesh::process_packet(uint8_t *data, uint16_t size)
{
    logf("data  process_packet :  ");
    for (uint8_t i = 0; i < size; i++)
        logf("%d ", data[i]);
    logf("\n");


    logf(" 00000000000000000000   RespirMesh::process_packet(uint8_t      \n");
    printf(" dev_id %x    \n", hardware_->device_id());
    printf(" dev_id  %d    \n", hardware_->device_id());
    printf(" dev_id %x  %d    \n", hardware_->device_id(), hardware_->device_id());


    RemBasicHeader *header = (RemBasicHeader *)data;

    switch (header->ProtobufType)
    {
    case ProtobufType_MESH_TOPOLOGY:
        // Serial.printf("putting data in  TOPO to root : \n");
        handle_mesh_topo(data, size);
        break;
    }
}

void RespirMesh::update()
{
    // logf(" Time: %d  \n", hardware_->time_milis());

    action_counter++;

    if (action_counter % 7 == 3)
    {
        send_mesh_topo();
    }
    else if (action_counter % 5 == 4)
    {
        // clean_channels();
    }
    else if (action_counter % 4 == 2)
    {
        // send_ping(ForwardingType_TO_ROOT);
        // send_ping(ForwardingType_TO_NEIGHBORS);
    }
    // else
    // {
    //     send_ping_to_server();
    // }
}

void RespirMesh::send_ping(ForwardingType forward_type)
{
    logf("Send PING \n");
    RemDataHeaderByte *header = (RemDataHeaderByte *)pb_buffer;
    // uint8_t ActionSize = sizeof(action_counter);
    header->ForwardingType = forward_type;
    header->HeaderType = HeaderType_DATA_BYTE;
    header->ProtobufType = ProtobufType_PING;
    header->Data = (uint8_t)hardware_->device_id();

    RespirMeshInfo ping_data = RespirMeshInfo_init_default;
    ping_data.source_id = hardware_->device_id();
    ping_data.type = ProtobufType_MESH_TOPOLOGY;

    // logf(" *******  %d  %d  %d  \n", sizeof(*header), sizeof(RemDataHeaderByte), sizeof(RemBasicHeader));
    uint16_t offsetHeader = sizeof(*header);
    pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &ping_data);
    if (!pb_status)
    {
        errorf("Encoding Mesh Topology failed: %s\n", PB_GET_ERROR(&ostream));
        return;
    }
    uint16_t packet_size = ostream.bytes_written + offsetHeader;

    tmili = hardware_->time_milis();
    send_packet(pb_buffer, packet_size);
}

void RespirMesh::send_mesh_topo()
{
    logf(" 00000000000000000000   RespirMesh::send_mesh_topo()      \n");
    printf(" dev_id %x  %d    \n", device_id(), device_id());

    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    header->ForwardingType = ForwardingType_TO_PARENT_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;
    header->ProtobufType = ProtobufType_MESH_TOPOLOGY;

    RespirMeshInfo mesh_topo_data = RespirMeshInfo_init_default;
    mesh_topo_data.source_id = hardware_->device_id();
    mesh_topo_data.type = ProtobufType_MESH_TOPOLOGY;

    // logf(" *******  %d  %d  %d  \n", sizeof(*header), sizeof(RemDataHeaderByte), sizeof(RemBasicHeader));
    uint16_t offsetHeader = sizeof(*header);
    pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
    bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &mesh_topo_data);

    if (!pb_status)
    {
        errorf("Encoding Mesh Topology failed: %s\n", PB_GET_ERROR(&ostream));
        return;
    }
    uint16_t packet_size = ostream.bytes_written + offsetHeader;

    debugf("RemBasicHeader size %d \n", offsetHeader);
    debugf("Protobuf size %d \n", (int)ostream.bytes_written);

    for (uint8_t i = 0; i < packet_size; i++)
        debugf("%d ", pb_buffer[i]);
    debugf("\n");

    logf("Send TOPO \n");
    send_packet(pb_buffer, packet_size);
    // handle_mesh_topo(pb_buffer, packet_size);
}

void RespirMesh::handle_mesh_topo(uint8_t *data, size_t size)
{
    logf(" 00000000000000000000   RespirMesh::handle_mesh_topo      \n");
    printf(" dev_id %x  %d    \n", device_id(), device_id());

    logf("data  handle_mesh_topo :  ");
    for (uint8_t i = 0; i < size; i++)
        logf("%d ", data[i]);
    logf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;
    uint16_t offsetHeader = sizeof(RemBasicHeader);
    memcpy(pb_buffer, data, offsetHeader);
    RespirMeshInfo meshTopo;

    uint8_t *packetData = (uint8_t *)(data) + offsetHeader;
    uint16_t packetDataLen = size - offsetHeader;
    pb_istream_t istream = pb_istream_from_buffer(packetData, packetDataLen);
    bool pb_status = pb_decode(&istream, RespirMeshInfo_fields, &meshTopo);

    if (!pb_status)
    {
        errorf("Decoding meshTopo failed %s with type %d\n", PB_GET_ERROR(&istream), header->ProtobufType);
        return;
    }

    if (header->ForwardingType == ForwardingType_TO_PARENT_TO_ROOT)
    {
        logf(" 00000000000000000000         \n");
        header->ForwardingType = ForwardingType_TO_ROOT;
        logf(" 1111111111111111111111111111 \n");
        meshTopo.type = ProtobufType_MESH_TOPOLOGY;
        logf(" 2222222222222 \n");
        printf(" src %x  tar %x     \n", meshTopo.source_id, meshTopo.target_id);
        // printf(" src %x  tar %x  dev_id %x    \n", meshTopo.source_id, meshTopo.target_id ,device_id() );
        meshTopo.target_id = device_id();
        logf(" 333333333333333333333333333333333333333 \n");

        pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

        if (!pb_status)
        {
            printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        printf("Send TOPO to root %lu %d \n", ostream.bytes_written, offsetHeader);
        printf(" src %x  tar %x \n", meshTopo.source_id, meshTopo.target_id);
        printf(" Forward %d \n", header->ForwardingType);
        printf(" Hea %d \n", header->HeaderType);
        printf(" Proto %d \n", header->ProtobufType);

        logf("Hand TOPO send %d\n", tmili);
        send_packet(pb_buffer, ostream.bytes_written + offsetHeader);
        logf(" 4444444444444444444 \n");
    }
    // else if (header->ForwardingType == ForwardingType_TO_PARENT)
    // {
    //     MapedChannels.push_back(new RemChannelAndId{rc, meshTopo.source_id});
    // }
}

// void RespirMesh::handle_ping(uint8_t *data, uint16_t size, RemChannel *rc)
// {
//     logf("Hand PING ");
//     RemDataHeaderByte *header = (RemDataHeaderByte *)data;
//     uint16_t offsetHeader = sizeof(RemDataHeaderByte);
//     header->ProtobufType = uint8_t(ProtobufType_PONG);

//     // memcpy(pb_buffer+offsetHeader+1,&ac,sizeof(ac));
//     logf("\tRecived PONG\n");
//     rc->send((uint8_t *)(pb_buffer), offsetHeader);
// }

// void RespirMesh::handle_pong(uint8_t *data, uint16_t size, RemChannel *rc)
// {
//     logf("Hand PONG %d\n", tmili - hardware_->time_milis());
//     // RemDataHeaderByte *header = (RemDataHeaderByte *)data;
//     // uint16_t offsetHeader = sizeof(RemDataHeaderByte);
//     if (rc->connected_to_root)
//     {
//         nrping++;
//         if (nrping >= nrpingSendTopo)
//         {
//             nrping = 0;
//             nrpingSendTopo = 16;
//             // send_mesh_topo();
//         }
//     }
//     // header->ProtobufType = uint8_t(ProtobufType_PONG);

//     // memcpy(pb_buffer+offsetHeader+1,&ac,sizeof(ac));
//     // rc->send((uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
// }
// void RespirMesh::send_info(RemChannel *rc)
// {
//     RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
//     uint16_t offsetHeader = sizeof(RemBasicHeader);
//     header->ForwardingType = ForwardingType_TO_PARENT;
//     header->HeaderType = HeaderType_BASIC;
//     header->ProtobufType = ProtobufType_MESH_TOPOLOGY;

//     RespirMeshInfo info_data = RespirMeshInfo_init_default;
//     info_data.source_id = hardware_->device_id();
//     info_data.type = ProtobufType_MESH_TOPOLOGY;

//     pb_ostream_t ostream = pb_ostream_from_buffer(
//         (uint8_t *)pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
//     bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &info_data);
//     if (!pb_status)
//     {
//         debugf("Encoding Info failed: %s\n", PB_GET_ERROR(&ostream));
//         return;
//     }
//     uint16_t packet_size = ostream.bytes_written + offsetHeader;

//     debugf("RemBasicHeader size %d \n", offsetHeader);
//     debugf("Protobuf size %d \n", (int)ostream.bytes_written);

//     for (uint8_t i = 0; i < packet_size; i++)
//         debugf("%d ", pb_buffer[i]);
//     debugf("\n");
//     rc->send(pb_buffer, packet_size);
// }

// void RespirMesh::handlePingPong(/*AsyncClient *c,*/ uint8_t *data, size_t
// len, RemBasicHeader *header)
// {
//     // Serial.printf("\n PingPong size is %d \n", len);
//     RespirMeshInfo pingpong;
//     pb_istream_t stream = pb_istream_from_buffer(data, len);
//     bool pb_status = pb_decode(&stream, RespirMeshInfo_fields, &pingpong);

//     if (!pb_status)
//     {
//         printf("Decoding PingPong failed %s with type %d\n",
//         PB_GET_ERROR(&stream), header->ProtobufType);
//         return;
//     }

//     if (pingpong.type == ProtobufType_PONG)
//     {
//         succesfullPongFromServer++;

//         // Serial.printf("<<< Got PONG #%d size %d at %d \n",
//         succesfullPongFromServer, len, micros());
//         // Serial.printf(" target_id %x \n", pingpong.target_id);
//         // Serial.printf("      source_id %x \n", pingpong.source_id);
//         return;
//     }
// }
// void RespirMesh::sendPing()
// {
//     for (std::list<RemChannel *>::iterator it = channels.begin(); it !=
//     channels.end(); ++it)
//     {
//         sendPingToNode((*it));
//     }
// }

// void RespirMesh::sendPingToNode(RemChannel *c)
// {
//     RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
//     uint16_t offsetHeader = sizeof(RemBasicHeader);
//     RespirMeshTimeSync timesync = RespirMeshTimeSync_init_default;

//     timesync.info.type = ProtobufType_PING;
//     timesync.info.target_id = 0;
//     timesync.info.source_id = hardware_->device_id();

//     header->ForwardingType = uint8_t(ForwardingType_TO_NODE);
//     header->ProtobufType = uint8_t(ProtobufType_PING);

//     timesync.request_sent_time = hardware_->time_milis();
//     pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader,
//     sizeof(pb_buffer) - offsetHeader);
//     bool pb_status = pb_encode(&stream, RespirMeshTimeSync_fields,
//     &timesync);
//     if (!pb_status)
//     {
//         printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
//         return;
//     }
//     c->send((uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
// }

// void RespirMesh::sendPongToNode(RemChannel *c, uint8_t *data, size_t len,
// RemBasicHeader *header)
// {
//     RespirMeshTimeSync timesync;

//     uint16_t offsetHeader = sizeof(RemBasicHeader);

//     // uint32_t us_start = micros();
//     pb_istream_t istream = pb_istream_from_buffer(data, len);
//     bool pb_status = pb_decode(&istream, RespirMeshTimeSync_fields,
//     &timesync);
//     if (!pb_status)
//     {
//         printf("Decoding TimeSync failed %s with type %d\n",
//         PB_GET_ERROR(&istream), header->ProtobufType);
//         return;
//     }
//     timesync.request_arrive_time = hardware_->time_milis();
//     header->ProtobufType = uint8_t(ProtobufType_PONG);
//     timesync.info.type = ProtobufType_PONG;
//     timesync.response_sent_time = hardware_->time_milis();
//     pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader,
//     sizeof(pb_buffer) - offsetHeader);
//     pb_status = pb_encode(&stream, RespirMeshTimeSync_fields, &timesync);

//     if (!pb_status)
//     {
//         printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
//         return;
//     }
//     c->send((uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
// }

// void RespirMesh::handle_pong(RemChannel *c, uint8_t *data, size_t len,
// RemBasicHeader *header)
// {
//     RespirMeshTimeSync timesync;
//     pb_istream_t stream = pb_istream_from_buffer(data, len);
//     bool pb_status = pb_decode(&stream, RespirMeshTimeSync_fields,
//     &timesync);

//     if (!pb_status)
//     {
//         printf("Decoding PingPong failed %s with type %d\n",
//         PB_GET_ERROR(&stream), header->ProtobufType);
//         return;
//     }
//     timesync.request_arrive_time = hardware_->time_milis();
//     timesync.info.type = ProtobufType_TIMESYNC;
//     header->ForwardingType = uint8_t(ForwardingType_TO_PARENT_TO_ROOT);
// }