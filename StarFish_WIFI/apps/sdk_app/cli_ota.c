/*
  * Copyright (c) 2013 Qualcomm Atheros, Inc..
  * All Rights Reserved.
  * Qualcomm Atheros Confidential and Proprietary.
  */

#include <qcom/qcom_common.h>
#include <qcom/socket_api.h>
#include <qcom/select_api.h>
#include <qcom/qcom_utils.h>
#include <qcom/qcom_nvram.h>
#include "libcli.h"
#include "main.h"

#define QCA_PARTITION_NUM 4

#define QCA_OTA_MAX_TFTP_RETRY_TIMES 2

#define QCA_OTA_PROTOCOL_ID 0xFF
#define QCA_OTA_MSG_BUF_SIZE 256
#define QCA_OTA_MAX_FILENAME_LEN 64

#define MD5_CHECKSUM_LEN 32

#define NVM_BLOCK_SZ 0x1000

extern A_UINT32 _inet_addr(A_CHAR *str);
extern SYS_CONFIG_t sys_config;

typedef enum {
  QCA_OTA_PROTOCOL_TFTP = 0,
  QCA_OTA_PROTOCOL_MAX = QCA_OTA_PROTOCOL_TFTP,
} QCA_OTA_PROTOCOL_t;

typedef enum {
  QCA_OTA_OK = 0,
  QCA_OTA_ERR_UNKOWN_MSG = 1000,
  QCA_OTA_ERR_IMAGE_NOT_FOUND = 1001,
  QCA_OTA_ERR_IMAGE_DOWNLOAD_FAIL = 1002,
  QCA_OTA_ERR_IMAGE_CHECKSUM_INCORRENT = 1003,
  QCA_OTA_ERR_SERVER_RSP_TIMEOUT = 1004,
  QCA_OTA_ERR_INVALID_FILENAME = 1005,
  QCA_OTA_ERR_UNSUPPORT_PROTOCOL = 1006,
  QCA_OTA_ERR_INVALID_PARTITION_INDEX = 1007,
  QCA_OTA_ERR_IMAGE_HDR_INCORRENT = 1008,
} QCA_OTA_ERROR_CODE_t;

typedef enum {
  QCA_OTA_REQUEST = 0,
  QCA_OTA_RESPONSE = 1,
} QCA_OTA_MSG_TYPE_t;

typedef struct {
    A_INT32 magic;
    A_INT32 length;
    A_CHAR checksum[MD5_CHECKSUM_LEN];
} __attribute__ ((packed)) QCA_OTA_IMAGE_HDR_t;

typedef struct {
    A_CHAR qca_code;
    A_CHAR opcode;
    A_UINT8 length;
    A_CHAR payload[0];
} __attribute__ ((packed)) QCA_OTA_MSG_HDR_t;

typedef struct {
    A_INT32 server_ip;
    A_CHAR file_name[QCA_OTA_MAX_FILENAME_LEN];
    A_UINT8 protocol;
    A_CHAR partition_index;
    A_CHAR ota_msg_checksum[MD5_CHECKSUM_LEN];
} __attribute__ ((packed)) QCA_OTA_MSG_REQUEST_t;

typedef struct {
    A_INT32 response_code;
    A_CHAR ota_image_checksum[MD5_CHECKSUM_LEN];
    A_CHAR ota_msg_checksum[MD5_CHECKSUM_LEN];
} __attribute__ ((packed)) QCA_OTA_MSG_RESPONSE_t;

/* TFTP misc */
#define TFTP_TIMEOUT		5000UL
#define TFTP_RX_SIZE        1024
#define TFTP_TX_SIZE        128
#define TFTP_SERVER_PORT    69

/* TFTP operations. */
#define TFTP_OP_RRQ		1
#define TFTP_OP_WRQ	 	2
#define TFTP_OP_DATA	3
#define TFTP_OP_ACK		4
#define TFTP_OP_ERROR	5
#define TFTP_OP_OACK	6

/* TFTP states */
#define TFTP_ST_RRQ			1
#define TFTP_ST_DATA		2
#define TFTP_ST_TOO_LARGE	3
#define TFTP_ST_BAD_MAGIC	4
#define TFTP_ST_OACK		5
#define TFTP_ST_ERROR 		6
#define TFTP_BLOCK_SIZE		512 /* default TFTP block size  */

static void    OTACLI_TftpPktRrq(const A_CHAR *file_name, A_UINT8 *buf, A_INT32 *len);
static void    OTACLI_TftpPktAck(A_INT32 block_seq, A_UINT8 *buf, A_INT32 *len);
static void    OTACLI_TftpPktParse(A_UINT8 *pkt_ptr, A_INT32 pkt_len, A_INT32 *pkt_seq, A_INT32 *pkt_state);
static void    CLIOTA_NvramReadData(A_INT32 partition, A_INT32 offset, A_UINT8 *buf, A_INT32 size);
static void    CLIOTA_NvramWriteData(A_INT32 partition, A_INT32 offset, A_UINT8 *buf, A_INT32 size);
static void    CLIOTA_NvramErasePartition(A_INT32 partition);
static void    CLIOTA_NvramWritePartitionMagicword(A_INT32 partition);
static A_INT32 CLIOTA_NvramGetPartitionBase(A_INT32 partition);
static void    CLIOTA_NvramSetPartitionBase(A_INT32 partition, A_UINT32 base_addr);
static void    CLIOTA_NvramGetActiveImagePartition(A_INT32 *partition);
static A_INT32 CLIOTA_ParseImageHdr(A_UINT8 *buffer);
static void    CLIOTA_CalculateImageChecksum(A_INT32 partition, A_CHAR *checksum);
static void    CLIOTA_NvramSwapPartitionTblEntry(A_INT32 partition1, A_INT32 partition2);
static A_INT32 CLIOTA_Tftp(struct cli_def *cli, A_INT32 ip_addr, A_CHAR *file_name, A_INT32 partition_index,A_INT32 test_flag);

/* TFTP buffer */
static A_UINT8 *p_tftp_tx_buf = NULL;
static A_UINT8 *p_tftp_rx_buf = NULL;

static A_INT32 qca_partition_entries[QCA_PARTITION_NUM] = { 0 };
static A_CHAR  ota_image_checksum[MD5_CHECKSUM_LEN]     = { 0 };
static A_INT32 ota_image_len                            = 0;

static void OTACLI_TftpPktRrq(const A_CHAR *file_name, A_UINT8 *buf, A_INT32 *len)
{
    A_UINT8 *pkt;
    unsigned short *s;
    pkt = buf;
    s = (unsigned short *) pkt;
    *s++ = htons(TFTP_OP_RRQ);
    pkt = (A_UINT8 *) s;
    A_STRCPY((A_CHAR *) pkt, file_name);
    pkt += A_STRLEN(file_name) + 1;
    A_STRCPY((A_CHAR *) pkt, "octet");
    pkt += 5;
    *len = pkt - buf;
}

static void OTACLI_TftpPktAck(A_INT32 block_seq, A_UINT8 *buf, A_INT32 *len)
{
    A_UINT8 *pkt;
    unsigned short *s;
    pkt = buf;
    s = (unsigned short *) pkt;
    *s++ = htons(TFTP_OP_ACK);
    *s++ = htons(block_seq);
    pkt = (A_UINT8 *) s;
    *len = pkt - buf;
}

static void OTACLI_TftpPktParse(A_UINT8 *pkt_ptr, A_INT32 pkt_len, A_INT32 *pkt_seq, A_INT32 *pkt_state)
{
    A_INT32 len = pkt_len;
    A_UINT8 *pkt = pkt_ptr;
    unsigned short opcode, *s;
    if (len < 2) {
        return;
    }
    len -= 2;
    s = (unsigned short *) pkt;
    opcode = *s++;
    pkt = (A_UINT8 *) s;

    switch (ntohs(opcode)) {

    case TFTP_OP_RRQ:
    case TFTP_OP_WRQ:
    case TFTP_OP_ACK:
        break;

    case TFTP_OP_OACK:
        *pkt_state = TFTP_ST_OACK;
        break;

    case TFTP_OP_DATA:
        if (len < 2)
            return;
        *pkt_seq = ntohs(*(unsigned short *) pkt);
        if (*pkt_state == TFTP_ST_RRQ || *pkt_state == TFTP_ST_OACK) {
            *pkt_state = TFTP_ST_DATA;
        }
        break;

    case TFTP_OP_ERROR:
        *pkt_state = TFTP_ST_ERROR;
        break;

    default:
        break;
    }
}

/* write data to one partition */
static void CLIOTA_NvramReadData(A_INT32 partition, A_INT32 offset, A_UINT8 *buf, A_INT32 size)
{
    A_INT32 base_addr = 0;
    base_addr = CLIOTA_NvramGetPartitionBase(partition);
    qcom_nvram_read(base_addr + offset, buf, size);
    return;
}

/* write data to one partition */
static void CLIOTA_NvramWriteData(A_INT32 partition, A_INT32 offset, A_UINT8 *buf, A_INT32 size)
{
    A_INT32 base_addr = 0;
    base_addr = CLIOTA_NvramGetPartitionBase(partition);
    qcom_nvram_write(base_addr + offset, buf, size);
    return;
}

static void CLIOTA_NvramErasePartition(A_INT32 partition)
{
    A_INT32 base_addr = 0;
    A_INT32 size = 0;
    base_addr = CLIOTA_NvramGetPartitionBase(partition);
    if (partition == 0) {

        /* partition #0 is special, with some bytes used for other purpose,
           16 bytes at the header, and 12 bytes at the end. These bytes
           shoule not been erased */
        size = 0x40000 - 16 - 12;

        /* align with FLASH sector size */
        size = size & 0xFFFFF000;
    }
    else {
        size = 0x40000;
    }
	qcom_nvram_erase(base_addr, size);
    return;
}

/* write MAGIC word for a partition */
static void CLIOTA_NvramWritePartitionMagicword(A_INT32 partition)
{
    A_INT32 base_addr = 0;
    A_UINT8 magic[4] = { 'N', 'V', 'R', 'M' };
    base_addr = CLIOTA_NvramGetPartitionBase(partition);
    qcom_nvram_write(base_addr, magic, 4);
    return;
}

static void CLIOTA_NvramLoadPartitionTable(void)
{
    A_INT32 qca_partition_tbl_base = 0;

    qca_partition_entries[0] = 0x10;

    /* load partition table */
    qcom_nvram_read(0, (A_UINT8 *) & qca_partition_tbl_base, 4);
    qcom_nvram_read(qca_partition_tbl_base, (A_UINT8 *) & qca_partition_entries[1], 12);
 A_PRINTF("qca_partition_tbl_base= %d\n", qca_partition_tbl_base); 

    return;
}

/* set base address of one partition */
static A_INT32 CLIOTA_NvramGetPartitionBase(A_INT32 partition)
{
    return qca_partition_entries[partition];
}

/* get base address of one partition */
static void CLIOTA_NvramSetPartitionBase(A_INT32 partition, A_UINT32 base_addr)
{
    qca_partition_entries[partition] = base_addr;
    CLIOTA_NvramLoadPartitionTable();
    return;
}

/* get partition num of current active image*/
static void CLIOTA_NvramGetActiveImagePartition(A_INT32 *partition)
{
    A_INT32 active_partition = 0;
    active_partition = qcom_nvram_select(-1);
    *partition = active_partition;
    return;
}

/* get partition num of current active image*/
static A_INT32 CLIOTA_ParseImageHdr(A_UINT8 *buffer)
{
    A_INT32 ret = 0;
    QCA_OTA_IMAGE_HDR_t *ota_image_hdr;
    A_INT32 magic = 0;
    ota_image_hdr = (QCA_OTA_IMAGE_HDR_t *) buffer;
    magic = ntohl(ota_image_hdr->magic);

    if (magic != 0x1234) {
        return QCA_OTA_ERR_IMAGE_HDR_INCORRENT;
    }

    ota_image_len = ntohl(ota_image_hdr->length);
    memcpy(ota_image_checksum, ota_image_hdr->checksum, MD5_CHECKSUM_LEN);

    return ret;
}

static void CLIOTA_NvramSwapPartitionTblEntry(A_INT32 partition1, A_INT32 partition2)
{
    A_INT32 tmp_base_1;
    A_INT32 tmp_base_2;

    tmp_base_1 = CLIOTA_NvramGetPartitionBase(partition1);
    tmp_base_2 = CLIOTA_NvramGetPartitionBase(partition1);

    CLIOTA_NvramSetPartitionBase(partition1, tmp_base_2);
    CLIOTA_NvramSetPartitionBase(partition2, tmp_base_1);
} 

static void CLIOTA_CalculateImageChecksum(A_INT32 partition, A_CHAR *checksum)
{
    A_INT32 len = ota_image_len;
    A_INT32 offset = 4;
    A_INT32 i = 0;
    qcom_sec_md5_init();

    while (len > TFTP_RX_SIZE) {
        memset(p_tftp_rx_buf, 0, TFTP_RX_SIZE);
        CLIOTA_NvramReadData(partition, offset, p_tftp_rx_buf, TFTP_RX_SIZE);
        qcom_sec_md5_update(p_tftp_rx_buf, TFTP_RX_SIZE);
        offset += TFTP_RX_SIZE;
        len -= TFTP_RX_SIZE;
        i++;
    }

    memset(p_tftp_rx_buf, 0, TFTP_RX_SIZE);
    CLIOTA_NvramReadData(partition, offset, p_tftp_rx_buf, len);
    qcom_sec_md5_update(p_tftp_rx_buf, len);
    memset(checksum, 0, 16);
    qcom_sec_md5_final(checksum);

    return;
}

/*  QCA OTA upgrade based on TFTP protocol */
static A_INT32 CLIOTA_Tftp(struct cli_def *cli, 
                       A_INT32 ip_addr, 
                       A_CHAR *file_name, 
                       A_INT32 partition_index,A_INT32 test_flag)
{
    A_INT32 ret = 0;
    A_INT32 fd, pkt_len, pkt_seq = 0, pkt_seq_last = 0;
    struct sockaddr_in serv_addr, from_addr;
    A_INT32 from_size = 0;
    struct timeval tmo;
    q_fd_set fd_sockSet;
    A_INT32 fd_act, nRecv, retry_times = 0;
    A_INT32 tftp_state = 0, total_rx_bytes = 0, blk_count = 0;
    A_UINT8 *data_blk_buf;
    A_INT32 data_blk_size = 0;

    A_INT32 currentPartition = 0;
    A_INT32 upgradePartition = 0;

    A_INT32 nSend = 0;
    A_INT32 retryCount = 0;

    A_INT32 otaWriteFlash = 1;
    A_INT32 otaNeedSwapPartition = 0;

    A_CHAR currentImageChecksum[MD5_CHECKSUM_LEN] = { 0 };
    A_CHAR lastImageChecksum[MD5_CHECKSUM_LEN]    = { 0 };
    A_INT32 ii=0;
    /* In current design, if FLASH is 512KB, then only 2 partitions are supported.
       Partition 0 (0x0000-0010 - 0x000-3FFFF): Gloden image
       Partition 1 (0x0004-0000 - 0x000-7FFFF): User image (upgradable)
       Partition 2 (0xFFFF-FFFF - 0FFFF-FFFF): Invalid
       Partition 3 (0xFFFF-FFFF - 0FFFF-FFFF): Invalid

       ------------------------------------------------------------------------
       |  Partition #1   | Current Active Part | Upgrade Partition | Need swap Partition Entry |
       -----------------------------------------------------------------------
       | Invalid           |  Partition #0           |  Partition#1         |  No                                |
       -----------------------------------------------------------------------
       |  Valid             |  Partition #1            | Partition#1          | No                                |
       -----------------------------------------------------------------------

       If FLASH is 1024 KB, the 4 partitions are supported

       Partition 0 (0x0000-0010 - 0x0003FFFF): Gloden image
       Partition 1 (0x0004-0000 - 0x0007FFFF): User image1(upgradable)
       Partition 2 (0x0008-0000 - 0x000BFFFF): User image2(upgradable)
       Partition 2 (0x000C-0000 - 0x0010FFFF): Reserved

       ------------------------------------------------------------------------------------
       |  Partition #1   |  Partition #2  | Current Active Part | Upgrade Partition | Need swap Partition Entry |
       ------------------------------------------------------------------------------------
       | Valid             |  Vaid            |   Partition #1           |  Partition#2         |  Yes                               |
       ------------------------------------------------------------------------------------
       |  Valid            |  Invalid         |  Partition #1            | Partition#2          | Yes                               |
       ------------------------------------------------------------------------------------
       |  Invalid         |  Invalid         |  Partition #0             |  Partition#1         | No                                |
       ------------------------------------------------------------------------------------
     */
if(test_flag==0)
{
    if (qca_partition_entries[2] == 0xFFFFFFFF) {

        /* Partition#2 invlaid, so it use 512KB FLASH */
        CLI_PRINTF("FLASH size is 512K");

        CLIOTA_NvramGetActiveImagePartition(&currentPartition);

        if (partition_index != -1 && partition_index < 2) {
            upgradePartition = partition_index;
        }
        else {
            upgradePartition = 1;
        }
    }

    else {

        /* Partition#2 invlaid, so it use 1024KB FLASH */
        CLI_PRINTF("FLASH size is  1024KB");

        CLIOTA_NvramGetActiveImagePartition(&currentPartition);

        if (partition_index != -1 && partition_index < 4) {
            upgradePartition = partition_index;
        }
        else if (currentPartition == 0) {
            upgradePartition = 1;
        }
        else {
            upgradePartition = 2;
            otaNeedSwapPartition = 1;
        }
    }
}
	A_PRINTF("my test1\r\n");

    /* create UDP socket */
    fd = qcom_socket(PF_INET, SOCK_DGRAM, 0);

    /* init tftp server address */
    serv_addr.sin_addr.s_addr = htonl(ip_addr);
    serv_addr.sin_port        = htons(TFTP_SERVER_PORT);
    serv_addr.sin_family      = AF_INET;

    p_tftp_tx_buf = (A_UINT8 *)qcom_mem_alloc(TFTP_TX_SIZE);
    p_tftp_rx_buf = (A_UINT8 *)qcom_mem_alloc(TFTP_RX_SIZE);

retry:
    total_rx_bytes = 0;

    /* send TFTP_RRQ */
    tftp_state = TFTP_ST_RRQ;
    memset(p_tftp_tx_buf, 0, TFTP_TX_SIZE);
    OTACLI_TftpPktRrq(file_name, p_tftp_tx_buf, &pkt_len);

    nSend = qcom_sendto(fd, 
                        (A_CHAR *) p_tftp_tx_buf, 
                        pkt_len, 
                        0, 
                        (struct sockaddr *) &serv_addr,
                        sizeof (serv_addr));

    while (1) {

        FD_ZERO(&fd_sockSet);
        FD_SET(fd, &fd_sockSet);

        tmo.tv_sec  = TFTP_TIMEOUT / 1000;
        tmo.tv_usec = TFTP_TIMEOUT % 1000;

        /* wait for response from TFTP server */
        fd_act = 0;
        fd_act = qcom_select(fd + 1, &fd_sockSet, NULL, NULL, &tmo);
	A_PRINTF("step 2.1\r\n");
        if (0 == fd_act) {

            /* timeout, retry */
            CLI_PRINTF("OTA server response timeout, retry");
            retry_times++;

            if (retry_times > 3) {
                ret = QCA_OTA_ERR_SERVER_RSP_TIMEOUT;
                break;
            }
            else {
                qcom_sendto(fd, (A_CHAR *) p_tftp_tx_buf, pkt_len, 0, (struct sockaddr *) &serv_addr,
                           sizeof (serv_addr));
            }
        }
        else {
		A_PRINTF("step 2.2\r\n");
            if (FD_ISSET(fd, &fd_sockSet)) {
		A_PRINTF("test_flag is:%d\r\n",test_flag);
                from_size = sizeof (from_addr);
                memset(p_tftp_rx_buf, 0, TFTP_RX_SIZE);
                nRecv = qcom_recvfrom(fd, (A_CHAR *) p_tftp_rx_buf, TFTP_RX_SIZE, 0,
                                     (struct sockaddr *) &from_addr, &from_size);
if(test_flag==1)
{
		A_PRINTF("yes test_flag\r\n");
   
	if (nRecv > 0) {
	retry_times = 0;
			A_PRINTF("step 1.1\r\n");
                    /* parse for TFTP packet */
                    OTACLI_TftpPktParse(p_tftp_rx_buf, nRecv, &pkt_seq, &tftp_state);
                    if ((TFTP_ST_OACK == tftp_state) || (TFTP_ST_DATA == tftp_state)) {
			A_PRINTF("step 1.2\r\n");
                        /* acknolowdge TFTP_ DATA and TFTP_OACK */
                        memset(p_tftp_tx_buf, 0, TFTP_TX_SIZE);
                        OTACLI_TftpPktAck(pkt_seq, p_tftp_tx_buf, &pkt_len);
                        serv_addr.sin_port = from_addr.sin_port;
                        qcom_sendto(fd, (A_CHAR *) p_tftp_tx_buf, pkt_len, 0,
                                   (struct sockaddr *) &serv_addr, sizeof (serv_addr));
                    }

                    else if (TFTP_ST_ERROR == tftp_state) {

                        /* response: 0x1002 (file download fail) */
                        ret = QCA_OTA_ERR_IMAGE_DOWNLOAD_FAIL;
                        break;
                    }
	   if ((TFTP_ST_DATA == tftp_state) && (pkt_seq != pkt_seq_last)) {
			A_PRINTF("step 1.3\r\n");

			 pkt_seq_last = pkt_seq;

                        /* skip TFTP header 4 bytes */
                        data_blk_buf = (p_tftp_rx_buf + 4);
                        data_blk_size = nRecv - 4;

                        /* save data block to upgrade partition */
                        if (data_blk_size >= 0) {

                            blk_count++;
                            total_rx_bytes += data_blk_size;
                        }
			for(ii=0;ii<data_blk_size;ii++)
{
			A_PRINTF("FTP DATA: 0x%x\r\n", (data_blk_buf+ii));
}
                        /* check if image reception complete */
                        if (total_rx_bytes == ota_image_len) {
			}
		}
	
	}
}
if(test_flag==0)
{
                if (nRecv > 0) {
                    retry_times = 0;

                    /* parse for TFTP packet */
                    OTACLI_TftpPktParse(p_tftp_rx_buf, nRecv, &pkt_seq, &tftp_state);
                    if ((TFTP_ST_OACK == tftp_state) || (TFTP_ST_DATA == tftp_state)) {

                        /* acknolowdge TFTP_ DATA and TFTP_OACK */
                        memset(p_tftp_tx_buf, 0, TFTP_TX_SIZE);
                        OTACLI_TftpPktAck(pkt_seq, p_tftp_tx_buf, &pkt_len);
                        serv_addr.sin_port = from_addr.sin_port;
                        qcom_sendto(fd, (A_CHAR *) p_tftp_tx_buf, pkt_len, 0,
                                   (struct sockaddr *) &serv_addr, sizeof (serv_addr));
                    }

                    else if (TFTP_ST_ERROR == tftp_state) {

                        /* response: 0x1002 (file download fail) */
                        ret = QCA_OTA_ERR_IMAGE_DOWNLOAD_FAIL;
                        break;
                    }

                    /* process TFTP_DATA */
                    if ((TFTP_ST_DATA == tftp_state) && (pkt_seq != pkt_seq_last)) {
                        pkt_seq_last = pkt_seq;

                        /* skip TFTP header 4 bytes */
                        data_blk_buf = (p_tftp_rx_buf + 4);
                        data_blk_size = nRecv - 4;

                        /* special work for first data block */
                        if (pkt_seq == 1) {

                            /* parse image header */
                            if ((ret = CLIOTA_ParseImageHdr(data_blk_buf)) != 0) {

                                /* invalid image header, stop and exit */
                                break;
                            }
                            data_blk_size -= sizeof (QCA_OTA_IMAGE_HDR_t);
                            data_blk_buf += sizeof (QCA_OTA_IMAGE_HDR_t);

                            /*  erase upgrade partition */
                            if (otaWriteFlash) {
                                CLIOTA_NvramErasePartition(upgradePartition);
                            }
                        }

                        /* save data block to upgrade partition */
                        if (data_blk_size >= 0) {
                            if (otaWriteFlash) {
                                CLIOTA_NvramWriteData(upgradePartition, total_rx_bytes + 4,
                                                      data_blk_buf, data_blk_size);
                            }
                            blk_count++;
                            total_rx_bytes += data_blk_size;
                        }

                        /* debug print */
                        if ((blk_count % 20) == 0) {
                            A_PRINTF("* ");
                            if ((blk_count % 400) == 0) {
                                A_PRINTF("\n");
                            }
                        }

                        /* check if image reception complete */
                        if (total_rx_bytes == ota_image_len) {

                            CLI_PRINTF("Image Receive completed");

                            /* calculate image checksum */
                            CLIOTA_CalculateImageChecksum(upgradePartition, currentImageChecksum);

                            /* checksum check pass */
                            if ((memcmp(currentImageChecksum, ota_image_checksum, MD5_CHECKSUM_LEN) == 0)) {

                                CLI_PRINTF("Checksum ok");

                                /* write MAGIC word for upgrade partition */
                                CLIOTA_NvramWritePartitionMagicword(upgradePartition);

                                /* do partition table entry swap if needed */
                                if (otaNeedSwapPartition) {
                                    CLIOTA_NvramSwapPartitionTblEntry(1, 2);
                                }

                                /* response code: 0 (success) */
                                ret = QCA_OTA_OK;
                                break;
                            }

                            else
                                /* checksum check fail */
                            {
                                CLI_PRINTF("Checksum error");

                                /* if retry < maxtime */
                                if (retryCount < QCA_OTA_MAX_TFTP_RETRY_TIMES) {

                                    /* save curernt checksum for later comparision */
                                    memcpy(lastImageChecksum, currentImageChecksum,
                                           MD5_CHECKSUM_LEN);

                                    /* reset TFTP client and retry */
                                    retryCount++;
                                    goto retry;
                                }

                                else {  /* retry exceeds maximum times */

                                    /* compare current checksum with last checksum */
                                    if (memcmp(currentImageChecksum, lastImageChecksum, MD5_CHECKSUM_LEN) == 0) {
                                        ret = QCA_OTA_ERR_IMAGE_CHECKSUM_INCORRENT;
                                    }
                                    else {
                                        ret = QCA_OTA_ERR_IMAGE_DOWNLOAD_FAIL;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
}
            }

            else {
                CLI_PRINTF("FD_ISSET fail");
            }
        }
    }

    /* close socket */
    qcom_close(fd);

    // Free rx buffer
    qcom_mem_free(p_tftp_tx_buf);
    qcom_mem_free(p_tftp_rx_buf);
    if(test_flag==0)
    ret = NVRAM_SaveSettings(&sys_config);

            if (ret != A_OK) {
                A_PRINTF("Save Settings Failed\n");
            }
	    else {
                A_PRINTF("Save Settings ok\n");
            }
    if (ret == QCA_OTA_OK)
        CLI_PRINTF("OTA upgrade finished!");
    else
        CLI_PRINTF("OTA upgrade fail: error code = %d!", ret);

    return ret;
}

/* QCA_OTA_upgrade external API */
static A_INT32 CLIOTA_Upgrade(struct cli_def *cli,
                   A_INT32 ip_addr, 
                   A_CHAR *file_name, 
                   A_INT32 protocol, 
                   A_INT32 partition_index,A_INT32 test_flag)
{
    A_INT32 ret;

    /* input parameter sanity check */
    if (!file_name) {
        return QCA_OTA_ERR_INVALID_FILENAME;
    }

    if (protocol > QCA_OTA_PROTOCOL_MAX) {
        return QCA_OTA_ERR_UNSUPPORT_PROTOCOL;
    }

    /* load partition table */
    CLIOTA_NvramLoadPartitionTable();

    /* do OTA upgrade according with different protocol */
    switch (protocol) {
    
    case QCA_OTA_PROTOCOL_TFTP:
        ret = CLIOTA_Tftp(cli, ip_addr, file_name, partition_index,test_flag);
        break;

    default:
        ret = QCA_OTA_ERR_UNSUPPORT_PROTOCOL;
        break;

    }
    return ret;
}

A_INT32 CLIOTA_FWUpgrade(struct cli_def *cli, const A_CHAR *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 ota_server_ip = 0;
    A_CHAR *file_name = NULL;
    A_INT32 test_flag=0;

    if (argc < 2) {
        cli_print(cli, "Usage: %s <OTA-server-ip> <image-name>", command);
        return -1;
    }

    ota_server_ip = _inet_addr(argv[0]);

    file_name = argv[1];
    if(argc>2)
    	test_flag=1;
    else 
	test_flag=0;	

    return CLIOTA_Upgrade(cli, ota_server_ip, file_name, 0, -1,test_flag);
}
