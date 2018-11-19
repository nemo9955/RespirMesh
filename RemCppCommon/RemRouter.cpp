#include "RemRouter.hpp"
#include <RemChannel.hpp>

#include "mesh-packet.pb.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

void RemRouter::stop(){};

void RemRouter::set_orchestrator(RemOrchestrator *remOrch_)
{
    remOrch = remOrch_;
};

void RemRouter::begin()
{
    mesh_topo_looper.begin(remOrch->basicHardware);
    mesh_topo_looper.set(8 * 1000);
}

void RemRouter::send_packet(uint8_t *data, uint16_t size)
{

    // funcf("send_packet :                \t");
    // for (uint8_t i = 0; i < size; i++)
    //     funcf("%d ", data[i]);
    // funcf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;
    switch (header->ForwardingType)
    {

    case ForwardingType_PARENT_AND_BACK:
    case ForwardingType_TO_PARENT:
    case ForwardingType_TO_PARENT_TO_ROOT:
    case ForwardingType_TO_ROOT:
        for (auto it = remOrch->channels.begin(); it != remOrch->channels.end(); ++it)
        {
            if ((*it)->connected_to_root)
            {
                (*it)->send(data, size);
                remOrch->basicHardware->sleep_milis(1);
            }
        }
        break;

    case ForwardingType_TO_NEIGHBORS:
    case ForwardingType_NEIGHBOR_TO_ROOT:
        for (auto it = remOrch->channels.begin(); it != remOrch->channels.end(); ++it)
        {
            (*it)->send(data, size);
            remOrch->basicHardware->sleep_milis(1);
        }
        break;
    }
    return;
}

void RemRouter::route_packet(uint8_t *data, uint16_t size)
{
    // funcf("route_packet :               \t");
    // for (uint8_t i = 0; i < size; i++)
    //     funcf("%d ", data[i]);
    // funcf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;

    switch (header->ForwardingType)
    {

    case ForwardingType_TO_ROOT:
    {
        for (auto it = remOrch->channels.begin(); it != remOrch->channels.end(); ++it)
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

void RemRouter::process_packet(uint8_t *data, uint16_t size)
{
    // funcf("process_packet :             \t");
    // for (uint8_t i = 0; i < size; i++)
    //     funcf("%d ", data[i]);
    // funcf("\n");

    RemBasicHeader *header = (RemBasicHeader *)data;

    switch (header->ProtobufType)
    {
    case ProtobufType_MESH_TOPOLOGY:
        // Serial.printf("putting data in  TOPO to root : \n");
        handle_mesh_topo(data, size);
        break;
    }
}

void RemRouter::update()
{
    // logf(" Time: %u  \n", remOrch->basicHardware->time_milis());

    if (mesh_topo_looper.check())
    {
        send_mesh_topo();
    }

    // action_counter++;

    // if (action_counter % 7 == 3)
    // {
    //     // remOrch->logs->trace(" 7 == 3");
    //     send_mesh_topo();
    // }
    // else if (action_counter % 5 == 4)
    // {
    //     // remOrch->logs->trace(" 5 == 4");
    // }
    // else if (action_counter % 4 == 2)
    // {
    //     // remOrch->logs->trace(" 4 == 2");
    //     // logf("\n");
    //     // send_ping(ForwardingType_TO_ROOT);
    //     // send_ping(ForwardingType_TO_NEIGHBORS);
    // }
    // else
    // {
    //     send_ping_to_server();
    // }
}

// void RemRouter::send_ping(ForwardingType forward_type)
// {
//     RemDataHeaderByte *header = (RemDataHeaderByte *)pb_buffer;
//     header->ForwardingType = forward_type;
//     header->HeaderType = HeaderType_DATA_BYTE;
//     header->ProtobufType = ProtobufType_PING;
//     header->Data = (uint8_t)remOrch->basicHardware->device_id();

//     RespirMeshInfo ping_data = RespirMeshInfo_init_default;
//     ping_data.source_id = remOrch->basicHardware->device_id();
//     ping_data.type = ProtobufType_MESH_TOPOLOGY;

//     // logf(" *******  %d  %d  %d  \n", sizeof(*header), sizeof(RemDataHeaderByte), sizeof(RemBasicHeader));
//     uint16_t offsetHeader = sizeof(*header);
//     pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
//     bool pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &ping_data);
//     if (!pb_status)
//     {
//         errorf("Encoding Mesh Topology failed: %s\n", PB_GET_ERROR(&ostream));
//         return;
//     }
//     uint16_t packet_size = ostream.bytes_written + offsetHeader;

// funcf("Send PING                     \t");
//     for (uint8_t i = 0; i < packet_size; i++)
//         funcf("%d ", pb_buffer[i]);
//     funcf("\n");

//     tmili = remOrch->basicHardware->time_milis();
//     send_packet(pb_buffer, packet_size);
// }

void RemRouter::send_mesh_topo()
{

    // remOrch->logs->debug(" void RemRouter::send_mesh_topo() ");

    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
    header->ForwardingType = ForwardingType_TO_PARENT_TO_ROOT;
    header->HeaderType = HeaderType_BASIC;
    header->ProtobufType = ProtobufType_MESH_TOPOLOGY;

    RespirMeshInfo mesh_topo_data = RespirMeshInfo_init_default;
    mesh_topo_data.source_id = remOrch->basicHardware->device_id();
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

    // debugf("RemBasicHeader size %d \n", offsetHeader);
    // debugf("Protobuf size %d \n", (int)ostream.bytes_written);

    // funcf("Send TOPO                    \t");
    // for (uint8_t i = 0; i < packet_size; i++)
    //     funcf("%d ", pb_buffer[i]);
    // funcf("\n");

    send_packet(pb_buffer, packet_size);
    // handle_mesh_topo(pb_buffer, packet_size);
}

void RemRouter::handle_mesh_topo(uint8_t *data, size_t size)
{
    //     funcf("handle_mesh_topo got :       \t");
    //     for (uint8_t i = 0; i < size; i++)
    //         funcf("%d ", data[i]);
    //     funcf("\n");

    uint16_t offsetHeader = sizeof(RemBasicHeader);
    memcpy(pb_buffer, data, offsetHeader);
    RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
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
        header->ForwardingType = ForwardingType_TO_ROOT;
        meshTopo.type = ProtobufType_MESH_TOPOLOGY;
        printf(" src %x  tar %x     \n", meshTopo.source_id, meshTopo.target_id);
        // printf(" src %x  tar %x  dev_id %x    \n", meshTopo.source_id, meshTopo.target_id ,remOrch->basicHardware->device_id() );
        meshTopo.target_id = remOrch->basicHardware->device_id();

        pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
        pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

        if (!pb_status)
        {
            printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
            return;
        }

        // printf("Send TOPO to root %lu %d \n", ostream.bytes_written, offsetHeader);
        printf("\t src %x -> tar %x \n", meshTopo.source_id, meshTopo.target_id);
        // printf(" Forward %d \n", header->ForwardingType);
        // printf(" Hea %d \n", header->HeaderType);
        // printf(" Proto %d \n", header->ProtobufType);

        uint16_t packet_size = ostream.bytes_written + offsetHeader;

        // funcf("handle_mesh_topo send:       \t");
        // for (uint8_t i = 0; i < packet_size; i++)
        //     funcf("%d ", pb_buffer[i]);
        // funcf("\n");
        send_packet(pb_buffer, packet_size);
    }
    // else if (header->ForwardingType == ForwardingType_TO_PARENT)
    // {
    //     MapedChannels.push_back(new RemChannelAndId{rc, meshTopo.source_id});
    // }
}

// void RemRouter::handle_ping(uint8_t *data, uint16_t size, RemChannel *rc)
// {
//     logf("Hand PING ");
//     RemDataHeaderByte *header = (RemDataHeaderByte *)data;
//     uint16_t offsetHeader = sizeof(RemDataHeaderByte);
//     header->ProtobufType = uint8_t(ProtobufType_PONG);

//     // memcpy(pb_buffer+offsetHeader+1,&ac,sizeof(ac));
//     logf("\tRecived PONG\n");
//     rc->send((uint8_t *)(pb_buffer), offsetHeader);
// }

// void RemRouter::handle_pong(uint8_t *data, uint16_t size, RemChannel *rc)
// {
//     logf("Hand PONG %d\n", tmili - remOrch->basicHardware->time_milis());
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
// void RemRouter::send_info(RemChannel *rc)
// {
//     RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
//     uint16_t offsetHeader = sizeof(RemBasicHeader);
//     header->ForwardingType = ForwardingType_TO_PARENT;
//     header->HeaderType = HeaderType_BASIC;
//     header->ProtobufType = ProtobufType_MESH_TOPOLOGY;

//     RespirMeshInfo info_data = RespirMeshInfo_init_default;
//     info_data.source_id = remOrch->basicHardware->device_id();
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

// void RemRouter::handlePingPong(/*AsyncClient *c,*/ uint8_t *data, size_t
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
// void RemRouter::sendPing()
// {
//     for (auto it = channels.begin(); it !=
//     channels.end(); ++it)
//     {
//         sendPingToNode((*it));
//     }
// }

// void RemRouter::sendPingToNode(RemChannel *c)
// {
//     RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
//     uint16_t offsetHeader = sizeof(RemBasicHeader);
//     RemRouterTimeSync timesync = RemRouterTimeSync_init_default;

//     timesync.info.type = ProtobufType_PING;
//     timesync.info.target_id = 0;
//     timesync.info.source_id = remOrch->basicHardware->device_id();

//     header->ForwardingType = uint8_t(ForwardingType_TO_NODE);
//     header->ProtobufType = uint8_t(ProtobufType_PING);

//     timesync.request_sent_time = remOrch->basicHardware->time_milis();
//     pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader,
//     sizeof(pb_buffer) - offsetHeader);
//     bool pb_status = pb_encode(&stream, RemRouterTimeSync_fields,
//     &timesync);
//     if (!pb_status)
//     {
//         printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
//         return;
//     }
//     c->send((uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
// }

// void RemRouter::sendPongToNode(RemChannel *c, uint8_t *data, size_t len,
// RemBasicHeader *header)
// {
//     RemRouterTimeSync timesync;

//     uint16_t offsetHeader = sizeof(RemBasicHeader);

//     // uint32_t us_start = micros();
//     pb_istream_t istream = pb_istream_from_buffer(data, len);
//     bool pb_status = pb_decode(&istream, RemRouterTimeSync_fields,
//     &timesync);
//     if (!pb_status)
//     {
//         printf("Decoding TimeSync failed %s with type %d\n",
//         PB_GET_ERROR(&istream), header->ProtobufType);
//         return;
//     }
//     timesync.request_arrive_time = remOrch->basicHardware->time_milis();
//     header->ProtobufType = uint8_t(ProtobufType_PONG);
//     timesync.info.type = ProtobufType_PONG;
//     timesync.response_sent_time = remOrch->basicHardware->time_milis();
//     pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer + offsetHeader,
//     sizeof(pb_buffer) - offsetHeader);
//     pb_status = pb_encode(&stream, RemRouterTimeSync_fields, &timesync);

//     if (!pb_status)
//     {
//         printf("Encoding PingPong failed: %s\n", PB_GET_ERROR(&stream));
//         return;
//     }
//     c->send((uint8_t *)(pb_buffer), stream.bytes_written + offsetHeader);
// }

// void RemRouter::handle_pong(RemChannel *c, uint8_t *data, size_t len,
// RemBasicHeader *header)
// {
//     RemRouterTimeSync timesync;
//     pb_istream_t stream = pb_istream_from_buffer(data, len);
//     bool pb_status = pb_decode(&stream, RemRouterTimeSync_fields,
//     &timesync);

//     if (!pb_status)
//     {
//         printf("Decoding PingPong failed %s with type %d\n",
//         PB_GET_ERROR(&stream), header->ProtobufType);
//         return;
//     }
//     timesync.request_arrive_time = remOrch->basicHardware->time_milis();
//     timesync.info.type = ProtobufType_TIMESYNC;
//     header->ForwardingType = uint8_t(ForwardingType_TO_PARENT_TO_ROOT);
// }