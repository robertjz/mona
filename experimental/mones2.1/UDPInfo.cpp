//$Id: Connection.cpp 3287 2006-06-18 07:17:51Z eds1275 $
#include "UDPInfo.h"
#include "Dispatch.h"
using namespace mones;
using namespace MonAPI;


void UDPCoInfo::CreateHeader(Ether* frame,uint8_t* data, uint16_t size)
{
    IP* ip=frame->IPHeader; //for psedo header
    ip->ttl =0x00;
    ip->prot=TYPEUDP;
    ip->chksum=bswap(size+sizeof(UDP));
    /////////
    UDP* udp=frame->IPHeader->UDPHeader;
    udp->srcport=bswap(localport);
    udp->dstport=bswap(remoteport);
    udp->len=bswap(size+sizeof(UDP));
    udp->chksum=0x0000;
    memcpy(udp->data,data,size);
    udp->chksum=bswap(checksum((uint8_t*)&(ip->ttl),size+sizeof(UDP)+12));
    CreateIPHeader(frame,size+sizeof(UDP)+sizeof(IP),TYPEUDP);
}

int UDPCoInfo::Strip(Ether* frame, uint8_t** data)
{   
    *data=frame->IPHeader->UDPHeader->data;
    return bswap(frame->IPHeader->len)-sizeof(IP)-sizeof(UDP);
}

bool UDPCoInfo::IsProcessed(Ether* frame)
{
    return Read_bottom_half(frame);
}

bool UDPCoInfo::IsMyPacket(Ether* frame)
{
    if( TYPEUDP    == frame->IPHeader->prot ){
        if( remoteip   == frame->IPHeader->srcip &&
            localport  == bswap(frame->IPHeader->UDPHeader->dstport) &&
            remoteport == bswap(frame->IPHeader->UDPHeader->srcport) )
        {
            return true;
        }
    }
    return false;
}

bool UDPCoInfo::Reply(Ether* frame)
{   
    if( frame->IPHeader->UDPHeader->dstport==bswap(DAYTIME)){
        remoteip=frame->IPHeader->srcip;
        localport=DAYTIME;
        remoteport=bswap(frame->IPHeader->UDPHeader->srcport);
        char* data="I can't see a clock.";    
        dispatcher->Send((uint8_t*)data,20,this);
        return true;
    }
    return false;
}
