#include <kern/e1000.h>

#define INDEX2OFFSET(index)     (index >> 2)
#define E1000_MAP_STATUS        0x80080783 

volatile uint32_t *e1000_bar0;

struct e1000_tx_desc tx_desc_pool[TX_DESC_POOL_SIZE];
char tx_buf_array[TX_DESC_POOL_SIZE][TX_PACKET_SIZE];

// LAB 6: Your driver code here

static void
e1000_tx_init()
{
        uint32_t TCTL_COLD_flag_value, TCTL_CT_flag_value, i;

        memset(tx_desc_pool, 0, sizeof(tx_desc_pool));

        // Intializing pool adresses
        e1000_bar0[INDEX2OFFSET(E1000_TDBAL)] = PADDR(tx_desc_pool);
        e1000_bar0[INDEX2OFFSET(E1000_TDBAH)] = 0;
        if (sizeof(tx_desc_pool) % 128 != 0 ) {
                panic("e1000_tx_init: sizeof tx_desc_pool not aligned to 128 bytes size: [0x%8x]\n",
                      sizeof(tx_desc_pool));
        }
        // Intializing pool ring
        e1000_bar0[INDEX2OFFSET(E1000_TDLEN)] = sizeof(tx_desc_pool);
        e1000_bar0[INDEX2OFFSET(E1000_TDH)] = 0;
        e1000_bar0[INDEX2OFFSET(E1000_TDT)] = 0;
        // Initializing flags
        TCTL_COLD_flag_value = E1000_TCTL_COLD & (0x40 << 12);
        TCTL_CT_flag_value = E1000_TCTL_CT & (0x10 << 4);
        e1000_bar0[INDEX2OFFSET(E1000_TCTL)] |= (E1000_TCTL_EN | E1000_TCTL_PSP | TCTL_COLD_flag_value |
                                                 TCTL_CT_flag_value);
        /* Transmit IPG:
         *  The value that should be programmed into IPGT is 10
         *  According to the IEEE802.3 standard, IPGR1 should be 2/3 of IPGR2 value.
         *  For the IEEE 802.3 standard IPG, the value that should be programmed into IPGR2 is 6. 
         */
        e1000_bar0[INDEX2OFFSET(E1000_TIPG)] = 10 | (4 << 10) | (6 << 20);

        for (i = 0; i < TX_DESC_POOL_SIZE; ++i) {
		tx_desc_pool[i].addr = PADDR(tx_buf_array[i]);
		tx_desc_pool[i].cmd = 0;
		tx_desc_pool[i].status |= E1000_TXD_STAT_DD;
	}
}

int
e1000_attach(struct pci_func *pcif)
{
        uint32_t status_reg;

        pci_func_enable(pcif);
        e1000_bar0 = (uint32_t *)mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
        status_reg = e1000_bar0[INDEX2OFFSET(E1000_STATUS)];
        if ((status_reg & E1000_MAP_STATUS)!= E1000_MAP_STATUS){
                panic("e1000_attach: memory map error, E1000 status[0x%8x]\n", status_reg);
        }
        e1000_tx_init();
        return 0;
}