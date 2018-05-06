#ifndef RESPIRMESH_HPP_
#define RESPIRMESH_HPP_

#include <RemChannel.hpp>
#include <list>
#include <stdint.h>

#include "RemHeaderTypes.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "mesh-packet.pb.h"

template <class Hardware,
          int INTERNAL_BUFFER_ZISE = 64>
class RespirMesh
{
  public:
    RespirMesh(){};
    ~RespirMesh(){};

    static void receive_fn(uint8_t *data, uint16_t size, void *arg)
    {
        logf("[ REM ] Received DATA \n");
        // ((RespirMesh<Hardware> *)arg)->recv(data, size);
        ((RespirMesh*)arg)->recv(data,size);
    };

    void add_channel(AbstractChannel *channel)
    {
        channel->set_recv_cb(receive_fn, (void *)this);
        channels.push_back(channel);
    };

    void send(uint8_t *data, uint16_t size)
    {
        for (std::list<AbstractChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            (*it)->send(data, size);
        }
    };

    void recv(uint8_t *data, uint16_t size)
    {
        for (std::list<AbstractChannel *>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            (*it)->send(data, size);
        }
    };

    void update()
    {
        logf(" Time: %d  \n", hardware_.time_milis());

        action_counter++;

        if (action_counter % 2 == 1)
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
    };

    void handleMeshTopology(uint8_t *data, size_t len)
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
            meshTopo.target_id = hardware_.device_id();

            pb_ostream_t ostream = pb_ostream_from_buffer(pb_buffer + offsetHeader, sizeof(pb_buffer) - offsetHeader);
            pb_status = pb_encode(&ostream, RespirMeshInfo_fields, &meshTopo);

            if (!pb_status)
            {
                printf("Encoding meshTopo failed: %s\n", PB_GET_ERROR(&ostream));
                return;
            }

            // printf(" target_id %x \n", pingpong.target_id);
            printf("Sending completed TOPO to root %d %d \n", ostream.bytes_written, offsetHeader);
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

    void send_mesh_topo_to_server()
    {
        RemBasicHeader *header = (RemBasicHeader *)pb_buffer;
        uint16_t offsetHeader = sizeof(RemBasicHeader);
        header->ForwardingType = ForwardingType_TO_ROOT;
        header->HeaderType = HeaderType_BASIC;

        RespirMeshInfo remPingPong = RespirMeshInfo_init_default;
        remPingPong.source_id = hardware_.device_id();

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

  private:
    std::list<AbstractChannel *> channels;
    Hardware hardware_;
    uint8_t pb_buffer[INTERNAL_BUFFER_ZISE];
    int action_counter = 0;

  protected:
};

#endif /* !RESPIRMESH_HPP_ */
