#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <time.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <linux/tcp.h> // TCP


// SIGNATURES
void my_packet_handler(u_char *args,const struct pcap_pkthdr *header,const u_char *packet);
u_int16_t handle_ethernet(const struct pcap_pkthdr* pkthdr, const u_char* packet);

// MAIN
int main(int argc, char **argv)
{

  char errbuf[PCAP_ERRBUF_SIZE];// tell us if there is an error

  if(argc<2) {
    printf("[ERROR] Missing the pcap file as argument!\nUsage: ./a.out my_file.pcap\n");
    return 1;
  }

  pcap_t *handle = pcap_open_offline(argv[1], errbuf);// to retrieve a pcap file pass in argument

  if(handle == NULL){
    printf("[ERROR] %s\n", errbuf);
  }

  // allow to parse a pcap file, 0 show that unlimited loop, callback function, we don't have argument for the callbal function
  pcap_loop(handle, 0, my_packet_handler, NULL);

  return 0;
}

/*
* Name : my_packet_handler
* function :  function is the callback to be run on every packet captured (called by <pcap_loop>)
*             then display packet info
*
*/
void my_packet_handler(u_char *args,const struct pcap_pkthdr *packet_header,const u_char *packet_body)
{
  //printf("Packet capture length: %d\n", packet_header->caplen);
  printf("Packet:\nTotal length: %d\n", packet_header->len);

  u_int16_t type = handle_ethernet(packet_header, packet_body);
  /*
  if(ntohs(type) == ETHERTYPE_IP) {
    // handle IP packet
  }
  */
}

u_int16_t handle_ethernet(const struct pcap_pkthdr* pkthdr, const u_char* packet) {
    // http://yuba.stanford.edu/~casado/pcap/section4.html
    struct ether_header *eptr;  /* net/ethernet.h */
    const struct ip* ipHeader;
    const struct tcphdr* tcpHeader;
    char sourceIp[INET_ADDRSTRLEN];
    char destIp[INET_ADDRSTRLEN];
    u_int sourcePort, destPort;

    /* lets start with the ether header... */
    eptr = (struct ether_header *) packet;

    fprintf(stdout,"[Ethernet] %s -> %s\n"
            ,ether_ntoa((const struct ether_addr *)&eptr->ether_shost)
            ,ether_ntoa((const struct ether_addr *)&eptr->ether_dhost));

    /* check to see if we have an ip packet */
    fprintf(stdout, "[Protocol] ");
    if (ntohs(eptr->ether_type) == ETHERTYPE_IP) {
        // IPv4
        fprintf(stdout,"IPv4\n");
        ipHeader = (struct ip*)(packet + sizeof(struct ether_header));
        inet_ntop(AF_INET, &(ipHeader->ip_src), sourceIp, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipHeader->ip_dst), destIp, INET_ADDRSTRLEN);
        if (ipHeader->ip_p == IPPROTO_TCP) {
          //tcpHeader = (tcphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip));
          //sourcePort = ntohs(tcpHeader->source);
          //destPort = ntohs(tcpHeader->dest);
          printf("[TCP] %s -> %s\n", sourceIp, destIp);
        }
    } else if (ntohs(eptr->ether_type) == ETHERTYPE_ARP) {
        fprintf(stdout,"ARP");
    } else if (ntohs(eptr->ether_type) == ETHERTYPE_REVARP) {
        fprintf(stdout,"RARP");
    } else if (ntohs(eptr->ether_type)==34525){
        // IPv6
        fprintf(stdout,"IPv6\n");
        ipHeader = (struct ip*)(packet + sizeof(struct ether_header));
        if (ipHeader->ip_p == IPPROTO_TCP) {
          printf("TCP\n");
        }
    } else {
        fprintf(stdout,"?");
        // exit(1);
    }
    fprintf(stdout,"\n\n");
    return eptr->ether_type;
}

/* ask pcap to find a valid device for use to sniff on */
/*  dev = pcap_lookupdev(errbuf);

 error checking
if(dev == NULL)
{
 printf("%s\n",errbuf);
 exit(1);
}

/* print out device name
printf("DEV: %s\n",dev);

/* ask pcap for the network address and mask of the device *
ret = pcap_lookupnet(dev,&netp,&maskp,errbuf);

if(ret == -1)
{
 printf("%s\n",errbuf);
 exit(1);
}

/* get the network address in a human readable form
addr.s_addr = netp;
net = inet_ntoa(addr);

if(net == NULL)/* thanks Scott :-P
{
  perror("inet_ntoa");
  exit(1);
}

printf("NET: %s\n",net);

/* do the same as above for the device's mask
addr.s_addr = maskp;
mask = inet_ntoa(addr);

if(mask == NULL)
{
  perror("inet_ntoa");
  exit(1);
}

printf("MASK: %s\n",mask);
*/
