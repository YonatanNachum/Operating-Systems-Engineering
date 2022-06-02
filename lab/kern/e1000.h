#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>
#include <kern/pci.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

#define E1000_VENODR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

#define TX_DESC_POOL_SIZE       64
#define TX_PACKET_SIZE          1518

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

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x1        /* End of Packet */
#define E1000_TXD_CMD_RS     0x8        /* Report Status */
#define E1000_TXD_CMD_DEXT   0x20       /* Descriptor extension (0 = legacy) */

#define E1000_TXD_STAT_DD    0x1        /* Descriptor Done */
#define E1000_TXD_STAT_EC    0x2        /* Excess Collisions */
#define E1000_TXD_STAT_LC    0x4        /* Late Collisions */
#define E1000_TXD_STAT_TU    0x8        /* Transmit underrun */

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


int e1000_attach(struct pci_func *pcif);
int e1000_transmit(void *data, uint16_t len);
#endif	// JOS_KERN_E1000_H
