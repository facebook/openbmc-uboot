/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include <common.h>
#include <pci.h>

#ifdef CONFIG_PCI

#define PCI_CSR_BASE		0x60000000
#define ASPEED_PCI_IO_BASE	0x00000000
#define ASPEED_PCI_IO_SIZE	0x00010000
#define ASPEED_PCI_MEM_BASE	0x68000000
#define ASPEED_PCI_MEM_SIZE	0x18000000

#define CSR_CRP_CMD_OFFSET	0x00
#define CSR_CRP_WRITE_OFFSET    0x04
#define CSR_CRP_READ_OFFSET	0x08
#define CSR_PCI_ADDR_OFFSET	0x0C
#define CSR_PCI_CMD_OFFSET	0x10
#define CSR_PCI_WRITE_OFFSET	0x14
#define CSR_PCI_READ_OFFSET	0x18
#define CSR_PCI_STATUS_OFFSET	0x1C

#define CRP_ADDR_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_CRP_CMD_OFFSET)
#define CRP_WRITE_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_CRP_WRITE_OFFSET)
#define CRP_READ_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_CRP_READ_OFFSET)
#define PCI_ADDR_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_ADDR_OFFSET)
#define PCI_CMD_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_CMD_OFFSET)
#define PCI_WRITE_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_WRITE_OFFSET)
#define PCI_READ_REG		(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_READ_OFFSET)

#define PCI_CMD_READ		0x0A
#define PCI_CMD_WRITE		0x0B

#define RESET_PCI_STATUS        *(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_STATUS_OFFSET) = 0x01
#define CHK_PCI_STATUS          (*(volatile ulong*) (PCI_CSR_BASE + CSR_PCI_STATUS_OFFSET) & 0x03)

static int pci_config_access (u8 access_type, u32 dev, u32 reg, u32 * data)
{
	u32 bus;
	u32 device;
	u32 function;

	bus = ((dev & 0xff0000) >> 16);
	device = ((dev & 0xf800) >> 11);
	function = (dev & 0x0700);

	if (bus == 0) {
		// Type 0 Configuration 		
		*PCI_ADDR_REG = (u32) (1UL << device | function | (reg & 0xfc));
	} else {
		// Type 1 Configuration 
		*PCI_ADDR_REG = (u32) (dev | ((reg / 4) << 2) | 1);
	}

        RESET_PCI_STATUS;
        
	if (access_type == PCI_CMD_WRITE) {
		*PCI_CMD_REG = (ulong) PCI_CMD_WRITE;
		*PCI_WRITE_REG = *data;
	} else {
		*PCI_CMD_REG = (ulong) PCI_CMD_READ;		
		*data = *PCI_READ_REG;
	}
            
	return (CHK_PCI_STATUS);
}

static int aspeed_pci_read_config_byte (u32 hose, u32 dev, u32 reg, u8 * val)
{
	u32 data;

	if (pci_config_access (PCI_CMD_READ, dev, reg, &data)) {
		*val = 0;
		return -1;
	}	

	*val = (data >> ((reg & 3) << 3)) & 0xff;

	return 0;
}


static int aspeed_pci_read_config_word (u32 hose, u32 dev, u32 reg, u16 * val)
{
	u32 data;

	if (reg & 1)
		return -1;

	if (pci_config_access (PCI_CMD_READ, dev, reg, &data)) {
		*val = 0;
		return -1;
	} 		

	*val = (data >> ((reg & 3) << 3)) & 0xffff;

	return 0;
}


static int aspeed_pci_read_config_dword (u32 hose, u32 dev, u32 reg,
					 u32 * val)
{
	u32 data = 0;

	if (reg & 3)
		return -1;

	if (pci_config_access (PCI_CMD_READ, dev, reg, &data))	{
		*val = 0;
		return -1;
	}		

	*val = data;

	return (0);
}

static int aspeed_pci_write_config_byte (u32 hose, u32 dev, u32 reg, u8 val)
{
	u32 data = 0;

	if (pci_config_access (PCI_CMD_READ, dev, reg, &data))
		return -1;

	data = (data & ~(0xff << ((reg & 3) << 3))) | (val <<
						       ((reg & 3) << 3));

	if (pci_config_access (PCI_CMD_WRITE, dev, reg, &data))
		return -1;

	return 0;
}


static int aspeed_pci_write_config_word (u32 hose, u32 dev, u32 reg, u16 val)
{
	u32 data = 0;

	if (reg & 1)
		return -1;

	if (pci_config_access (PCI_CMD_READ, dev, reg, &data))
		return -1;

	data = (data & ~(0xffff << ((reg & 3) << 3))) | (val <<
							 ((reg & 3) << 3));

	if (pci_config_access (PCI_CMD_WRITE, dev, reg, &data))
		return -1;

	return 0;
}

static int aspeed_pci_write_config_dword (u32 hose, u32 dev, u32 reg, u32 val)
{
	u32 data;

	if (reg & 3) {
		return -1;
	}

	data = val;

	if (pci_config_access (PCI_CMD_WRITE, dev, reg, &data))
		return -1;

	return (0);
}

/*
 *	Initialize PCIU
 */
aspeed_pciu_init ()
{

    unsigned long reg;

    /* Reset PCI Host */
    reg = *((volatile ulong*) 0x1e6e2004);
    *((volatile ulong*) 0x1e6e2004) = reg | 0x00280000;

    reg = *((volatile ulong*) 0x1e6e2074);		/* REQ2 */    
    *((volatile ulong*) 0x1e6e2074) = reg | 0x00000010;

    *((volatile ulong*) 0x1e6e2008) |= 0x00080000;        
    reg = *((volatile ulong*) 0x1e6e200c); 
    *((volatile ulong*) 0x1e6e200c) = reg & 0xfff7ffff;
    udelay(1); 
    *((volatile ulong*) 0x1e6e2004) &= 0xfff7ffff;
                 
    /* Initial PCI Host */
    RESET_PCI_STATUS;
	
    *CRP_ADDR_REG = ((ulong)(PCI_CMD_READ) << 16) | 0x04;
    reg = *CRP_READ_REG;
    
    *CRP_ADDR_REG = ((ulong)(PCI_CMD_WRITE) << 16) | 0x04;
    *CRP_WRITE_REG = reg | 0x07;
   
}

/*
 *	Initialize Module
 */

void aspeed_init_pci (struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	aspeed_pciu_init ();			/* Initialize PCIU */

	/* PCI memory space #1 */
	pci_set_region (hose->regions + 0,
			ASPEED_PCI_MEM_BASE, ASPEED_PCI_MEM_BASE, ASPEED_PCI_MEM_SIZE, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (hose->regions + 1,
			ASPEED_PCI_IO_BASE, ASPEED_PCI_IO_BASE, ASPEED_PCI_IO_SIZE, PCI_REGION_IO);

	hose->region_count = 2;

	hose->read_byte   = aspeed_pci_read_config_byte;
	hose->read_word   = aspeed_pci_read_config_word;
	hose->read_dword  = aspeed_pci_read_config_dword;
	hose->write_byte  = aspeed_pci_write_config_byte;
	hose->write_word  = aspeed_pci_write_config_word;
	hose->write_dword = aspeed_pci_write_config_dword;

	pci_register_hose (hose);

	hose->last_busno = pci_hose_scan (hose);

	return;
}
#endif /* CONFIG_PCI */

