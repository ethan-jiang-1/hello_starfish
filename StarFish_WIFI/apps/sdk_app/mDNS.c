#include "main.h"
#include "ctype.h"

#define READ_BUFFER_SIZE (20)
#define HEADER_SIZE      (12)
#define QDCOUNT_OFFSET   (4)
#define A_RECORD_SIZE    (14)
#define NSEC_RECORD_SIZE (20)
#define TTL_OFFSET       (4)
#define IP_OFFSET        (10)

extern A_UINT32 _inet_addr(A_CHAR *str);

extern SYS_CONFIG_t sys_config;

void MDNS_StartDaemon()
{
    // Expected query values
    A_UINT8 *expected = NULL;
    A_INT32  expectedLen = 0;

    // Current parsing state
    A_INT32 index = 0;

    // Response data
    A_UINT8 *response = NULL;

    A_INT32 responseLen = 0;

    // Socket for MDNS communication
    A_INT32 mdnsSocket = -1; 

    // Construct DNS query response
    // TODO: Move these to flash or just construct in code.
    const A_UINT8 respHeader[] = { 0x00, 0x00,   // ID = 0
        0x84, 0x00,   // Flags = response + authoritative answer
        0x00, 0x00,   // Question count = 0
        0x00, 0x01,   // Answer count = 1
        0x00, 0x00,   // Name server records = 0
        0x00, 0x01    // Additional records = 1
    };

    // Generate positive response for IPV4 address
    const A_UINT8 aRecord[] = { 0x00, 0x01,                // Type = 1, A record/IPV4 address
        0x80, 0x01,                // Class = Internet, with cache flush bit
        0x00, 0x00, 0x00, 0x00,    // TTL in seconds, to be filled in later
        0x00, 0x04,                // Length of record
        0x00, 0x00, 0x00, 0x00     // IP address, to be filled in later
    };

    // Generate negative response for IPV6 address (CC3000 doesn't support IPV6)
    const A_UINT8 nsecRecord[] = {  0xC0, 0x0C,                // Name offset
        0x00, 0x2F,                // Type = 47, NSEC (overloaded by MDNS)
        0x80, 0x01,                // Class = Internet, with cache flush bit
        0x00, 0x00, 0x00, 0x00,    // TTL in seconds, to be filled in later
        0x00, 0x08,                // Length of record
        0xC0, 0x0C,                // Next domain = offset to FQDN
        0x00,                      // Block number = 0
        0x04,                      // Length of bitmap = 4 bytes
        0x40, 0x00, 0x00, 0x00     // Bitmap value = Only first bit (A record/IPV4) is set
    };

    // Values for:
    //  - 5 (length)
    //  - "local"
    //  - 0x00 (end of domain)
    //  - 0x00 0x01 (A record query)
    //  - 0x00 0x01 (Class IN)
    const A_UINT8 local[] = { 0x05, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x00, 0x00, 0x01, 0x00, 0x01 };

    int queryFQDNLen;
    int i;

    A_CHAR hostname[LEN_SSID] = { 0 };
    A_UINT8 macAddr[6]        = { 0 };

    A_STATUS status = qcom_mac_get((A_UINT8 *)&macAddr);
    if (A_OK != status) {
        A_PRINTF("Fail to get the MAC\r\n");
        return;
    }

    sprintf(hostname, "%s-%02X%02X%02X", sys_config.apSSID, macAddr[3], macAddr[4], macAddr[5]);

    // Construct DNS request/response fully qualified domain name of form:
    // <domain length>, <domain characters>, 5, "local"
    size_t n = strlen(hostname);
    if (n > 255) {
        // Can only handle domains that are 255 chars in length.
        return;
    }

    expectedLen = 12 + n;

    expected = (A_UINT8 *)qcom_mem_alloc(expectedLen);
    if (expected == NULL) {
        return;
    }

    expected[0] = (uint8_t)n;

    // Copy in domain characters as lowercase
    for (i = 0; i < n; ++i) {
        expected[1 + i] = tolower(hostname[i]);
    }

    memcpy(&expected[1 + n], local, 11); 

    // Allocate memory for response.
    queryFQDNLen = expectedLen - 4;
    responseLen = HEADER_SIZE + queryFQDNLen + A_RECORD_SIZE + NSEC_RECORD_SIZE;

    response = (A_UINT8 *)qcom_mem_alloc(responseLen);
    if (response == NULL) {
        return;
    }

    // Copy data into response.
    memcpy(response, respHeader, HEADER_SIZE);
    memcpy(response + HEADER_SIZE, expected, queryFQDNLen); 

    while (1) {

        // Add IP address to response
        A_UINT32 ipAddress, netmask, gateway;
        A_UINT8 buffer[READ_BUFFER_SIZE];
        A_UINT32 ttlSeconds = 3600;

        struct timeval timeout;
        q_fd_set reads;

        int n;

        A_UINT8 *records = response + HEADER_SIZE + queryFQDNLen;
        memcpy(records, aRecord, A_RECORD_SIZE);
        memcpy(records + A_RECORD_SIZE, nsecRecord, NSEC_RECORD_SIZE);

        // Add TTL to records.
        A_UINT8 ttl[4] = { (A_UINT8)(ttlSeconds >> 24), (A_UINT8)(ttlSeconds >> 16), (A_UINT8)(ttlSeconds >> 8), (A_UINT8)ttlSeconds };
        memcpy(records + TTL_OFFSET, ttl, 4);
        memcpy(records + A_RECORD_SIZE + 2 + TTL_OFFSET, ttl, 4);

        qcom_ip_address_get( &ipAddress, &netmask, &gateway);

        records[IP_OFFSET]     = (A_UINT8)(ipAddress >> 24);
        records[IP_OFFSET + 1] = (A_UINT8)(ipAddress >> 16);
        records[IP_OFFSET + 2] = (A_UINT8)(ipAddress >> 8);
        records[IP_OFFSET + 3] = (A_UINT8)ipAddress;

        // Open the MDNS socket if it isn't already open.
        if (mdnsSocket == -1) {

            struct sockaddr_in address;

            // Create the UDP socket
            mdnsSocket = qcom_socket(AF_INET, SOCK_DGRAM, 0);

            if (mdnsSocket < 0) {
                return;
            }

            // Use port 5353 and listen/send to the multicast IP 224.0.0.251
            memset(&address, 0, sizeof(address));
            address.sin_family      = AF_INET;
            address.sin_port        = htons(5353);
            address.sin_addr.s_addr = _inet_addr("224, 0, 0, 251");

            if (qcom_bind(mdnsSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
                return;
            }
        }

        // Check if data is available to read using select
        timeout.tv_sec  = 0;
        timeout.tv_usec = 5000;

        while (1) {

            FD_ZERO(&reads); 
            FD_SET(mdnsSocket, &reads);

            qcom_select(mdnsSocket + 1, &reads, NULL, NULL, &timeout);

            if (!FD_ISSET(mdnsSocket, &reads)) {
                // No data to read.
                continue;
            }

            // Read available data.
            n = qcom_recv(mdnsSocket, (char *)&buffer, sizeof(buffer), 0);

            if (n < 1) {
                // Error getting data.
                break;
            }

            // Look for domain name in request and respond with canned response if found.
            for (i = 0; i < n; ++i) {

                A_UINT8 ch = tolower(buffer[i]);

                // Check character matches expected.
                if (ch == expected[index]) {

                    index++;

                    // Check if domain name was found and send a response.
                    if (index >= expectedLen) {
                        // Send response to multicast address.
                        qcom_send(mdnsSocket, (char *)response, responseLen, 0);
                        index = 0;
                    }
                }
                else if (ch == expected[0]) {
                    // Found a character that doesn't match, but does match the start of the domain.
                    index = 1;
                }
                else {
                    // Found a character that doesn't match the expected character or start of domain.
                    index = 0;
                }
            }
        }
    }

    return;
}
