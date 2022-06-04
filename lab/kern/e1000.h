#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>
#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>
#include <kern/picirq.h>

#define E1000_VENODR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

#define TX_DESC_POOL_SIZE       64
#define TX_PACKET_SIZE          1518

#define RX_DESC_POOL_SIZE       128
#define RX_PACKET_SIZE          2048

/* Register Set:
 * Registers are defined to be 32 bits and should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 */
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_IMS      0x000D0  /* Interrupt Mask Set - RW */

#define E1000_RCTL     0x00100          /* RX Control - RW */
#define E1000_RAL      0x5400           /* Receive Address Low */
#define E1000_RAH      0x5404           /* Receive Address High */
#define E1000_RDBAL    0x02800          /* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH    0x02804          /* RX Descriptor Base Address High - RW */
#define E1000_RDLEN    0x02808          /* RX Descriptor Length - RW */
#define E1000_RDH      0x02810          /* RX Descriptor Head - RW */
#define E1000_RDT      0x02818          /* RX Descriptor Tail - RW */
#define E1000_RAH_AV   0x80000000       /* Receive descriptor valid */
#define E1000_MTA      0x05200          /* Multicast Table Array - RW Array */
#define E1000_ICR      0x000C0          /* Interrupt Cause Read - R/clr */

/* Receive Control */
#define E1000_RCTL_EN             0x00000002    /* enable */
#define E1000_RCTL_SZ_2048        0x00000000    /* rx buffer size 2048 */
#define E1000_RCTL_BAM            0x00008000    /* broadcast enable */
#define E1000_RCTL_SECRC          0x04000000    /* Strip Ethernet CRC */

/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_BCE    0x00000004    /* busy check enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define E1000_TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */

/* Interrupt Cause Read */
#define E1000_ICR_TXDW    0x00000001    /* Transmit desc written back */
#define E1000_ICR_RXT0    0x00000080    /* rx timer intr (ring 0) */

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x1        /* End of Packet */
#define E1000_TXD_CMD_RS     0x8        /* Report Status */
#define E1000_TXD_CMD_DEXT   0x20       /* Descriptor extension (0 = legacy) */

#define E1000_TXD_STAT_DD    0x1        /* Descriptor Done */
#define E1000_TXD_STAT_EC    0x2        /* Excess Collisions */
#define E1000_TXD_STAT_LC    0x4        /* Late Collisions */
#define E1000_TXD_STAT_TU    0x8        /* Transmit underrun */

/* Receive Descriptor bit definitions */
#define E1000_RXD_STAT_DD       0x01    /* Descriptor Done */
#define E1000_RXD_STAT_EOP      0x02    /* End of Packet */

struct e1000_tx_desc
{
        uint64_t addr;
        uint16_t length;
        uint8_t cso;
        uint8_t cmd;
        uint8_t status;
        uint8_t css;
        uint16_t special;
}__attribute__((packed));

struct e1000_rx_desc
{
        uint64_t addr;
        uint16_t length;
        uint16_t cs;
        uint8_t status;
        uint8_t errors;
        uint16_t special;
}__attribute__((packed));


int e1000_attach(struct pci_func *pcif);
int e1000_transmit(void *data, uint16_t len);
int e1000_receive(void *data);
void e1000_intr();

#endif	// JOS_KERN_E1000_H
