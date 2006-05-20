#include <monapi.h>
#include <sys/types.h>
#include <monapi/io.h>

namespace mones{

inline word  bswap(word w){ return (w>>8)|(w<<8); }

#pragma pack(push,2)

struct ICMP {
    byte  type;
    byte  code;
    word  chksum;
    byte  data[0];
};

struct UDP {
    word  srcport;
    word  dstport;
    word  len;
    word  chksum;
    byte  data[0];
};

struct TCP {
    word  srcport;
    word  dstport;
    dword seqnumber;
    dword acknumber;
    byte  reserved;
    byte  flag;
    word  window;
    word  chksum;
    word  purgent;
    byte  data[0];
};

struct IP {
    byte  verhead;
    byte  tos;
    word  len;
    word  id;
    word  frag;
    byte  ttl;
    byte  prot;
    word  chksum;
    dword srcip;
    dword dstip;
    union{
        ICMP ICMPHeader[0];
        TCP  TCPHeader[0];
        UDP  UDPHeader[0];
    };
    enum {
        TYPEICMP = 0x01,
        TYPEIGMP = 0x02,
        TYPETCP  = 0x06,
        TYPEUDP  = 0x11,
    };
};

class IPStack
{
protected:
    void FillIPHeader(IP*);
    void dumpPacket(IP*); 
    void FillICMPHeader(ICMP*,ICMP*);
};

};
#pragma pack(pop)