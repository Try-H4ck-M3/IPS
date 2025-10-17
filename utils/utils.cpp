#include "utils.h"

// Global variable definitions
struct nfq_handle *h;
struct nfq_q_handle *qh;
int fd;

void setup_iptables() {
    system("iptables -I INPUT -j NFQUEUE --queue-num 0");
    system("iptables -I OUTPUT -j NFQUEUE --queue-num 0");
    system("iptables -I FORWARD -j NFQUEUE --queue-num 0");
}

void cleanup_iptables()
{
    // Flush all NFQUEUE rules from all chains
    system("iptables -F INPUT");
    system("iptables -F OUTPUT");
    system("iptables -F FORWARD");
    
    // Restore default policies
    system("iptables -P INPUT ACCEPT");
    system("iptables -P OUTPUT ACCEPT");
    system("iptables -P FORWARD ACCEPT");
}

void cleanup_and_exit(int sig)
{
    cleanup_iptables();

    if (qh) nfq_destroy_queue(qh);
    if (h) nfq_close(h);

    exit(sig);
}

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    unsigned char *packetData;
    int len = nfq_get_payload(nfa, &packetData);

    uint32_t id = 0;
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
    if (ph)
    {
        id = ntohl(ph->packet_id);
    }

    // Default values
    string src_ip = "0.0.0.0";
    string dst_ip = "0.0.0.0";
    uint16_t src_port = 0;
    uint16_t dst_port = 0;
    uint8_t proto = 0;

    if (len >= sizeof(struct iphdr))
    {
        struct iphdr *ip_header = (struct iphdr *)packetData;

        // Validate IPv4
        if (ip_header->version != 4)
        {
            goto process_packet;  // Not IPv4, just accept
        }

        // Check total length
        uint16_t ip_len = ntohs(ip_header->tot_len);
        if (len < ip_len)
        {
            goto process_packet;  // Incomplete packet
        }

        // Convert IPs to string
        char src[INET_ADDRSTRLEN];
        char dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_header->saddr, src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ip_header->daddr, dst, INET_ADDRSTRLEN);
        src_ip = string(src);
        dst_ip = string(dst);

        proto = ip_header->protocol;

        // Calculate IP header size (in bytes, not words)
        uint8_t ip_header_len = ip_header->ihl * 4;

        // Only parse ports if TCP or UDP
        if (proto == 6 || proto == 17) // 6 = TCP, 17 = UDP 
        {
            if (len >= ip_header_len + sizeof(struct tcphdr))\
            {
                void *transport_ptr = packetData + ip_header_len;

                if (proto == 6) // TCP
                {
                    struct tcphdr *tcp = (struct tcphdr *)transport_ptr;
                    src_port = ntohs(tcp->source);
                    dst_port = ntohs(tcp->dest);
                }
                else if (proto == 17) // UDP
                {
                    struct udphdr *udp = (struct udphdr *)transport_ptr;
                    src_port = ntohs(udp->source);
                    dst_port = ntohs(udp->dest);
                }
            }
        }
    }

process_packet:
    bool verdict = packet_processor(packetData, len, src_ip, dst_ip, src_port, dst_port, proto);

    return nfq_set_verdict(qh, id, verdict ? NF_ACCEPT : NF_DROP, 0, nullptr);
}

int prepare(Logger logger)
{
    uid_t uid = geteuid();
    if (uid != 0)
    {
        logger.error("Must be run as root");
        exit(1);
    }

    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);
    signal(SIGQUIT, cleanup_and_exit);

    logger.info("Setting up iptables");
    setup_iptables();

    h = nfq_open();
    if (!h)
    {
        logger.error("Can't open nfqueue");
        cleanup_and_exit(1);
    }

    if (nfq_unbind_pf(h, AF_INET) < 0)
    {
        logger.error("nfq_unbind_pf");
    }

    if (nfq_bind_pf(h, AF_INET) < 0)
    {
        logger.error("nfq_bind_pf");
        cleanup_and_exit(1);
    }

    qh = nfq_create_queue(h, 0, &cb, nullptr);
    if (!qh)
    {
        logger.error("nfq_create_queue");
        cleanup_and_exit(1);
    }

    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
    {
        logger.error("nfq_set_mode");
        cleanup_and_exit(1);
    }
    
    return 0;
}