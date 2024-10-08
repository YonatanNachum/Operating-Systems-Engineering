#include <kern/e1000.h>
#include <kern/trap.h>
#include <kern/env.h>

#include <inc/env.h>

#define INDEX2OFFSET(index)     (index >> 2)
#define E1000_MAP_STATUS        0x80080783 

volatile uint32_t *e1000_bar0;

struct e1000_tx_desc tx_desc_pool[TX_DESC_POOL_SIZE];
//char tx_buf_array[TX_DESC_POOL_SIZE][TX_PACKET_SIZE];

struct e1000_tx_desc rx_desc_pool[RX_DESC_POOL_SIZE];
//char rx_buf_array[RX_DESC_POOL_SIZE][RX_PACKET_SIZE];
struct rx_buf *rx_buf_array = NULL;

uint8_t e1000_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};

uint32_t rec_head_index = 0;

// LAB 6: Your driver code here

static void
e1000_tx_init()
{
        uint32_t i;

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

        /* Initializing flags:
         * Transmit Enable
         * Pad Short Packets: Padding makes the packet 64 bytes long.
         * Collision Distance - Full-Duplex
         * Collision Threshold - Number of re-transmission prior to giving up on the packet.
         */
        e1000_bar0[INDEX2OFFSET(E1000_TCTL)] = (E1000_TCTL_EN | E1000_TCTL_PSP | (E1000_TCTL_COLD & (0x40 << 12)) |
                                                (E1000_TCTL_CT & (0x10 << 4)));

        /* Transmit IPG:
         *  The value that should be programmed into IPGT is 10
         *  According to the IEEE802.3 standard, IPGR1 should be 2/3 of IPGR2 value.
         *  For the IEEE 802.3 standard IPG, the value that should be programmed into IPGR2 is 6. 
         */
        e1000_bar0[INDEX2OFFSET(E1000_TIPG)] = 10 | (4 << 10) | (6 << 20);

        for (i = 0; i < TX_DESC_POOL_SIZE; ++i) {
		//tx_desc_pool[i].addr = PADDR(tx_buf_array[i]);
		tx_desc_pool[i].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
		tx_desc_pool[i].status |= E1000_TXD_STAT_DD;
	}
}

static void
e1000_config_mac()
{
       uint32_t low = 0, high = 0;
       int i;

        for (i = 0; i < 4; i++) {
                low |= e1000_mac[i] << (8 * i);
        }

        for (i = 4; i < 6; i++) {
                high |= e1000_mac[i] << (8 * i);
        }

        e1000_bar0[INDEX2OFFSET(E1000_RAL)] = low;

        /* Address Valid(E1000_RAH_AV):
         * Determines whether this address is compared against the
         * incoming packet. When set, the address is valid and is
         * compared against the incoming packet. When cleared, the
         * address is invalid and is not compared against the received packet.
         */
        e1000_bar0[INDEX2OFFSET(E1000_RAH)] = high | E1000_RAH_AV;
}

static inline uint16_t
e1000_eeprom_read(uint8_t addr)
{
        uint16_t word;

        /* Software can use the EEPROM Read register (EERD) to cause the Ethernet controller to read a
         * word from the EEPROM that the software can then use. To do this, software writes the address to
         * read the Read Address (EERD.ADDR) field and then simultaneously writes a 1b to the Start Read
         * bit (EERD.START). The Ethernet controller then reads the word from the EEPROM, sets the Read
         * Done bit (EERD.DONE), and puts the data in the Read Data field (EERD.DATA). Software can
         * poll the EEPROM Read register until it sees the EERD.DONE bit set, then use the data from the
         * EERD.DATA field. Any words read this way are not written to hardware’s internal registers.
         */
        e1000_bar0[INDEX2OFFSET(E1000_EERD)] = E1000_EEPROM_RW_REG_START | (addr << E1000_EEPROM_RW_ADDR_SHIFT);
        while ((e1000_bar0[INDEX2OFFSET(E1000_EERD)] & E1000_EEPROM_RW_REG_DONE) == 0);
        word = e1000_bar0[INDEX2OFFSET(E1000_EERD)] >> E1000_EEPROM_RW_REG_DATA;
        e1000_bar0[INDEX2OFFSET(E1000_EERD)] &= ~(E1000_EEPROM_RW_REG_START);
        return word;
}

static void
e1000_eeprom_read_mac()
{
        uint16_t word1, word2, word3;

        if ((e1000_bar0[INDEX2OFFSET(E1000_EECD)] & E1000_EECD_PRES) == 0) {
                panic("rx_init: EEPROM is not present\n");
        }

        word1 = e1000_eeprom_read(0x0);
        word2 = e1000_eeprom_read(0x1);
        word3 = e1000_eeprom_read(0x2);

        e1000_mac[0] = word1 & 0xff;
        e1000_mac[1] = word1 >> 8;
        e1000_mac[2] = word2 & 0xff;
        e1000_mac[3] = word2 >> 8;
        e1000_mac[4] = word3 & 0xff;
        e1000_mac[5] = word3 >> 8;
        cprintf("MAC: %x %x %x %x %x %x\n", e1000_mac[0], e1000_mac[1], e1000_mac[2],
                e1000_mac[3], e1000_mac[4], e1000_mac[5]);
}

static void
e1000_rx_init()
{
        uint32_t i;

        memset(rx_desc_pool, 0, sizeof(rx_desc_pool));

        e1000_eeprom_read_mac();
        e1000_config_mac();

        // Initialize the MTA
        e1000_bar0[INDEX2OFFSET(E1000_MTA)] = 0;

        // Intializing pool adresses
        e1000_bar0[INDEX2OFFSET(E1000_RDBAL)] = PADDR(rx_desc_pool);
        e1000_bar0[INDEX2OFFSET(E1000_RDBAH)] = 0;
        if (sizeof(rx_desc_pool) % 128 != 0 ) {
                panic("e1000_rx_init: sizeof tx_desc_pool not aligned to 128 bytes size: [0x%8x]\n",
                      sizeof(rx_desc_pool));
        }

        // Intializing pool ring
        e1000_bar0[INDEX2OFFSET(E1000_RDLEN)] = sizeof(rx_desc_pool);
        e1000_bar0[INDEX2OFFSET(E1000_RDH)] = 0;
        e1000_bar0[INDEX2OFFSET(E1000_RDT)] = RX_DESC_POOL_SIZE - 1;

        /* Initializing Receive Control fields:
         * Receiver Enable = 1
         * Store Bad Packets = 0
         * Long Packet Reception Enable = 0
         * Loopback mode = No loopback
         * Receive Descriptor Minimum Threshold Size = 0
         * Broadcast Accept Mode = 1
         * Receive Buffer Size = 2048
         * Strip Ethernet CRC from incoming packet = Strip CRC field
         */
        e1000_bar0[INDEX2OFFSET(E1000_RCTL)] = E1000_RCTL_EN | E1000_RCTL_SZ_2048 | E1000_RCTL_BAM | E1000_RCTL_SECRC;

        for (i = 0; i < RX_DESC_POOL_SIZE; ++i) {
		rx_desc_pool[i].addr = PADDR(rx_buf_array[i].buf);
	}
}

int
e1000_attach(struct pci_func *pcif)
{
        uint32_t status_reg, clean_icr;

        pci_func_enable(pcif);
        e1000_bar0 = (uint32_t *)mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
        status_reg = e1000_bar0[INDEX2OFFSET(E1000_STATUS)];
        if ((status_reg & E1000_MAP_STATUS)!= E1000_MAP_STATUS){
                panic("e1000_attach: memory map error, E1000 status[0x%8x]\n", status_reg);
        }
        e1000_tx_init();
        e1000_rx_init();

        /* ICR register bits are cleared upon read */
        clean_icr = e1000_bar0[INDEX2OFFSET(E1000_ICR)];

        /* Program the Interrupt Mask Set/Read:
         * Receiver Timer Interrupt (ICR.RXT0)
         * Descriptor done [Transmit Descriptor Write-back (TXDW)] 
         */
        e1000_bar0[INDEX2OFFSET(E1000_IMS)] = E1000_ICR_TXDW | E1000_ICR_RXT0;
        assert(IRQ_E1000 == pcif->irq_line);
        irq_setmask_8259A(irq_mask_8259A & ~(1<<pcif->irq_line));
        return 0;
}

int
e1000_transmit(void *data, uint16_t len)
{
        uint32_t tail_index;
	pte_t *pte;

        tail_index = e1000_bar0[INDEX2OFFSET(E1000_TDT)];
        if ((tx_desc_pool[tail_index].status & E1000_TXD_STAT_DD) == 0) {
                return -E_TX_POOL_FULL;
        }
        if (len > TX_PACKET_SIZE) {
                return -E_INVAL;
        }
        //memmove(tx_buf_array[tail_index], data, len);
        pte = pgdir_walk(curenv->env_pgdir, data, 0);
        tx_desc_pool[tail_index].addr = PTE_ADDR(*pte) + PGOFF(data);
        tx_desc_pool[tail_index].length = len;
        tx_desc_pool[tail_index].status &= (~E1000_TXD_STAT_DD);
        tail_index = (tail_index + 1) % TX_DESC_POOL_SIZE;
        e1000_bar0[INDEX2OFFSET(E1000_TDT)] = tail_index;
        return tail_index | ((uint32_t)(tx_desc_pool[tail_index].status & E1000_TXD_STAT_DD) << 30);
}

int
e1000_receive(uint32_t *buf_idx)
{
        //uint32_t tail_index;
        int pkt_length;

        //tail_index = e1000_bar0[INDEX2OFFSET(E1000_RDT)];
        if ((rx_desc_pool[rec_head_index].status & E1000_RXD_STAT_DD) == 0) {
                return -E_RX_POOL_EMPTY;
        }
        *buf_idx = rec_head_index;
        // memmove(data, rx_buf_array[rec_head_index].buf, rx_desc_pool[rec_head_index].length);
        pkt_length = rx_desc_pool[rec_head_index].length;
        // rx_desc_pool[rec_head_index].status &= (~(E1000_RXD_STAT_DD | E1000_RXD_STAT_EOP));
        // e1000_bar0[INDEX2OFFSET(E1000_RDT)] = (tail_index + 1) % RX_DESC_POOL_SIZE;
        // rec_head_index = (rec_head_index + 1) % RX_DESC_POOL_SIZE;
        return pkt_length;
}

void
e1000_intr()
{
        uint32_t intr_status;
        static envid_t out_ns, in_ns;

	if (out_ns <= 0 || in_ns <= 0) {
		out_ns = env_find_by_type(ENV_TYPE_OUT_NS);
                in_ns = env_find_by_type(ENV_TYPE_IN_NS);
        }
        intr_status = e1000_bar0[INDEX2OFFSET(E1000_ICR)];
        if (out_ns > 0 && intr_status & E1000_ICR_TXDW) {
                envs[out_ns].env_status = ENV_RUNNABLE;
        }
        if (in_ns > 0 && intr_status & E1000_ICR_RXT0) {
                envs[in_ns].env_status = ENV_RUNNABLE;
        }
}

void
e1000_free_rx_buf()
{
        uint32_t tail_index;

        tail_index = e1000_bar0[INDEX2OFFSET(E1000_RDT)];
        rx_desc_pool[rec_head_index].status &= (~(E1000_RXD_STAT_DD | E1000_RXD_STAT_EOP));
        e1000_bar0[INDEX2OFFSET(E1000_RDT)] = (tail_index + 1) % RX_DESC_POOL_SIZE;
        rec_head_index = (rec_head_index + 1) % RX_DESC_POOL_SIZE;
}
