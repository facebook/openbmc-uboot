//#include "io.h"
#include <asm/io.h>

#define MAC1_BASE	0x1e660000
#define MAC2_BASE	0x1e680000
#define MDIO0_BASE	(MAC1_BASE + 0x60)
#define MDIO1_BASE	(MAC2_BASE + 0x60)
#define SCU_BASE        0x1e6e2000


#ifdef CONFIG_ASPEED_AST2600
#define MAC3_BASE	0x1e670000
#define MAC4_BASE	0x1e690000

#define PMI_BASE	0x1e650000
#undef MDIO0_BASE
#undef MDIO1_BASE
#define MDIO0_BASE	(PMI_BASE + 0x00)
#define MDIO1_BASE	(PMI_BASE + 0x08)
#define MDIO2_BASE	(PMI_BASE + 0x10)
#define MDIO3_BASE	(PMI_BASE + 0x18)
#endif

#define GPIO_BASE	0x1e780000

/* macros for register access */
#define SCU_RD(offset)          readl(SCU_BASE + offset)
#define SCU_WR(value, offset)   writel(value, SCU_BASE + offset)

#define MAC1_RD(offset)		readl(MAC1_BASE + offset)
#define MAC1_WR(value, offset)	writel(value, MAC1_BASE + offset)
#define MAC2_RD(offset)		readl(MAC2_BASE + offset)
#define MAC2_WR(value, offset)	writel(value, MAC2_BASE + offset)
#ifdef CONFIG_ASPEED_AST2600
#define MAC3_RD(offset)		readl(MAC3_BASE + offset)
#define MAC3_WR(value, offset)	writel(value, MAC3_BASE + offset)
#define MAC4_RD(offset)		readl(MAC4_BASE + offset)
#define MAC4_WR(value, offset)	writel(value, MAC4_BASE + offset)
#endif

#define GPIO_RD(offset)		readl(GPIO_BASE + offset)
#define GPIO_WR(value, offset)	writel(value, GPIO_BASE + offset)
/* typedef for register access */
typedef union {
	uint32_t w;
	struct {
		uint32_t reserved_0 : 6;	/* bit[5:0] */
		uint32_t mac1_interface : 1;	/* bit[6] */
		uint32_t mac2_interface : 1;	/* bit[7] */
		uint32_t reserved_1 : 24;	/* bit[31:8] */
	}b;
} hw_strap1_t;

typedef union {
	uint32_t w;
	struct {
		uint32_t mac3_interface : 1;	/* bit[0] */
		uint32_t mac4_interface : 1;	/* bit[1] */		
		uint32_t reserved_0 : 30;	/* bit[31:2] */
	}b;
} hw_strap2_t;

uint32_t SRAM_RD(uint32_t addr);