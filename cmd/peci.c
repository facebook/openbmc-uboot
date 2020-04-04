#include <common.h>
#include <command.h>
#include <string.h>
#include <asm/io.h>

#define AST_SCU				(0x1e6e2000)
#if defined(CONFIG_ASPEED_AST2500)
#define AST_SCU_SYSRST_CTRL (AST_SCU + 0x04)
#elif defined(CONFIG_ASPEED_AST2600)
#define AST_SCU_SYSRST_CLR2	(AST_SCU + 0x54)
#endif

#define AST_PECI			(0x1e78b000)
#define AST_PECI_CTRL		(AST_PECI + 0x00)
#define AST_PECI_TIMING		(AST_PECI + 0x04)
#define AST_PECI_CMD		(AST_PECI + 0x08)
#define AST_PECI_RW_LEN		(AST_PECI + 0x0C)
#define AST_PECI_EXP_FCS	(AST_PECI + 0x10)
#define AST_PECI_CAP_FCS	(AST_PECI + 0x14)
#define AST_PECI_INT_CTRL	(AST_PECI + 0x18)
#define AST_PECI_INT_STAT	(AST_PECI + 0x1C)
#define AST_PECI_WR_DATA0	(AST_PECI + 0x20)
#define AST_PECI_WR_DATA1	(AST_PECI + 0x24)
#define AST_PECI_WR_DATA2	(AST_PECI + 0x28)
#define AST_PECI_WR_DATA3	(AST_PECI + 0x2C)
#define AST_PECI_RD_DATA0	(AST_PECI + 0x30)
#define AST_PECI_RD_DATA1	(AST_PECI + 0x34)
#define AST_PECI_RD_DATA2	(AST_PECI + 0x38)
#define AST_PECI_RD_DATA3	(AST_PECI + 0x3C)
#define AST_PECI_WR_DATA4	(AST_PECI + 0x40)
#define AST_PECI_WR_DATA5	(AST_PECI + 0x44)
#define AST_PECI_WR_DATA6	(AST_PECI + 0x48)
#define AST_PECI_WR_DATA7	(AST_PECI + 0x4C)
#define AST_PECI_RD_DATA4	(AST_PECI + 0x50)
#define AST_PECI_RD_DATA5	(AST_PECI + 0x54)
#define AST_PECI_RD_DATA6	(AST_PECI + 0x58)
#define AST_PECI_RD_DATA7	(AST_PECI + 0x5C)

#ifdef CONFIG_ASPEED_AST2500
static void ast2500_peci_init(void)
{
	unsigned int val;
	static bool is_init = false;

	/* init once only */
	if (is_init)
		return;

	/* release PECI reset */
	val = readl(AST_SCU_SYSRST_CTRL);
	val &= ~(0x1 << 10);
	writel(val, AST_SCU_SYSRST_CTRL);

	/*
	 * 1. disable PECI
	 * 2. set source clock to 24Mhz oscillator
	 * 3. set clock divider to 8
	 */
	writel(0x80301, AST_PECI_CTRL);

	/* set negotiate timing */
	writel(0x4040, AST_PECI_TIMING);

	/* enable interrupt */
	writel(0x1f, AST_PECI_INT_CTRL);

	/* enable PECI */
	writel(0x80311, AST_PECI_CTRL);

	is_init = true;
}
#endif

#ifdef CONFIG_ASPEED_AST2600
static void ast2600_peci_init(void)
{
	static bool is_init = false;

	/* init once only */
	if (is_init)
		return;

	/* release PECI reset */
	writel(0x10, AST_SCU_SYSRST_CLR2);

	/* 
	 * 1. disable PECI
	 * 2. set source clock to HCLK
	 * 3. set clock divider to 2
	 * 4. set 32-byte mode
	 */
	writel(0x80901, AST_PECI_CTRL);

	/* set negotiate timing */
	writel(0x0303, AST_PECI_TIMING);

	/* enable interrupt */
	writel(0x1f, AST_PECI_INT_CTRL);

	/* enable PECI */
	writel(0x80911, AST_PECI_CTRL);

	is_init = true;
}
#endif

static void ast_peci_init(void)
{
#if defined(CONFIG_ASPEED_AST2500)
	return ast2500_peci_init();
#elif defined(CONFIG_ASPEED_AST2600)
	return ast2600_peci_init();
#endif
}

static int do_ast_peci_ping(unsigned long client_addr)
{
	unsigned int rec_wfcs;
	unsigned int exp_wfcs;

	unsigned long val;
	unsigned long retry = 10;

	ast_peci_init();

	/* make PECI no operation */
	writel(0x0, AST_PECI_CMD);

	/* set client address */
	writel((client_addr & 0xff), AST_PECI_RW_LEN);

	/* clear interrupt status */
	writel(0x1f, AST_PECI_INT_STAT);

	/* fire PECI command */
	writel(0x1, AST_PECI_CMD);

	/* wait PECI done for 10 seconds */
	printf("Waiting PECI ... ");
	while (retry) {
		val = readl(AST_PECI_INT_STAT);
		if (val & 0x1)
			break;
		printf("Retry ... ");
		udelay(1000000);
		--retry;
	}

	if (retry == 0) {
		printf("Timeout\n");
		goto ping_clean_and_exit;
	}
	printf("Done\n");

	/* compare the expected WFCS and the captured one */
	exp_wfcs = readl(AST_PECI_EXP_FCS) & 0xFF;
	rec_wfcs = readl(AST_PECI_CAP_FCS) & 0xFF;
	if (exp_wfcs != rec_wfcs)
		printf("Mismatched WFCS: %02x, expected %02x\n", rec_wfcs, exp_wfcs);
	else
		printf("Ping: ACK\n");

ping_clean_and_exit:
	writel(0x1f, AST_PECI_INT_STAT);
	writel(0x0, AST_PECI_CMD);

	return 0;
}

static int do_ast_peci_getdib(unsigned long client_addr)
{
	unsigned char dib[8];

	unsigned long val;
	unsigned long retry = 10;

	unsigned int rec_wfcs;
	unsigned int exp_wfcs;

	ast_peci_init();

	memset(dib, 0, sizeof(dib));

	/* make PECI no operation */
	writel(0x0, AST_PECI_CMD);

	/* prepare GetDIB command code 0xF7 */
	writel(0xf7, AST_PECI_WR_DATA0);
	writel(0x00, AST_PECI_WR_DATA1);
	writel(0x00, AST_PECI_WR_DATA2);
	writel(0x00, AST_PECI_WR_DATA3);
	writel(0x00, AST_PECI_WR_DATA4);
	writel(0x00, AST_PECI_WR_DATA5);
	writel(0x00, AST_PECI_WR_DATA6);
	writel(0x00, AST_PECI_WR_DATA7);

	/* 
	 * 1. set read length to 8 bytes
	 * 2. set write length to 1 bytes
	 * 3. set client address
	 */
	writel((0x80100 | (client_addr & 0xff)), AST_PECI_RW_LEN);
	
	/* clear interrupt status */
	writel(0x1f, AST_PECI_INT_STAT);

	/* fire PECI command */
	writel(0x1, AST_PECI_CMD);

	/* wait PECI done for 10 seconds */
	printf("Waiting PECI ... ");
	while (retry) {
		val = readl(AST_PECI_INT_STAT);
		if (val & 0x1)
			break;
		printf("Retry ... ");
		udelay(1000000);
		--retry;
	}

	if (retry == 0) {
		printf("Timeout\n");
		goto getdib_clean_and_exit;
	}
	printf("Done\n");

	/* compare the expected WFCS and the captured one */
	exp_wfcs = readl(AST_PECI_EXP_FCS) & 0xFF;
	rec_wfcs = readl(AST_PECI_CAP_FCS) & 0xFF;
	if (exp_wfcs != rec_wfcs) {
		printf("Mismatched WFCS: %02x, expected %02x\n",
			   rec_wfcs, exp_wfcs);
		goto getdib_clean_and_exit;
	}

	*((unsigned int*)dib) = readl(AST_PECI_RD_DATA0);
	*((unsigned int*)(dib + 4)) = readl(AST_PECI_RD_DATA1);

	printf("DIB: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		   dib[0], dib[1], dib[2], dib[3],
		   dib[4], dib[5], dib[6], dib[7]);

getdib_clean_and_exit:
	writel(0x1f, AST_PECI_INT_STAT);
	writel(0x0, AST_PECI_CMD);

	return 0;
}

static int do_ast_peci(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *peci_cmd;
	unsigned long client_addr;

	if (argc < 3)
		return CMD_RET_USAGE;

	peci_cmd = argv[1];
	client_addr = strtoul(argv[2], 0, 0);

	if (client_addr > 0xFF) {
		printf("Invalid client address: %lu\n", client_addr);
		return CMD_RET_USAGE;
	}

	if (!strcmp(peci_cmd, "ping"))
		return do_ast_peci_ping(client_addr);

	if (!strcmp(peci_cmd, "getdib"))
		return do_ast_peci_getdib(client_addr);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(peci, 3, 0, do_ast_peci,
		   "ASPEED PECI general bus command test program",
		   "ping <client_addr> - Ping to check if the device at the targeted address can respond\n"
		   "peci getdib <client_addr> - Get 8-byte Device Information Bytes\n"
		   );
