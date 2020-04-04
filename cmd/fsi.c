#include <common.h>
#include <command.h>
#include <aspeed_fsi.h>
#include <dm/device.h>
#include <dm/uclass.h>

struct fsi_master_aspeed *fsi;

static void do_break(void)
{
	debug("%s\n", __func__);
	aspeed_fsi_break(fsi, 0);
}

static void do_status(void)
{
	debug("%s\n", __func__);
	aspeed_fsi_status(fsi);
}

static void do_getcfam(int argc, char *const argv[])
{
	int rc;
	uint32_t addr, val;

	if (argc != 3) {
		printf("invalid arguments to getcfam\n");
		return;
	}

	addr = simple_strtoul(argv[2], NULL, 16);

	debug("%s %08x\n", __func__, addr);
	rc = aspeed_fsi_read(fsi, 0, addr, &val, 4);
	if (rc) {
		printf("error reading: %d\n", rc);
		return;
	}

	printf("0x%08x\n", be32_to_cpu(val));
}

static void do_putcfam(int argc, char *const argv[])
{
	int rc;
	uint32_t addr, val;

	if (argc != 4) {
		printf("invalid arguments to putcfam\n");
		return;
	}

	addr = simple_strtoul(argv[2], NULL, 16);
	val = simple_strtoul(argv[3], NULL, 16);

	debug("%s %08x %08x\n", __func__, addr, val);
	rc = aspeed_fsi_write(fsi, 0, addr, &val, 4);
	if (rc)
		printf("error writing: %d\n", rc);
}

static void do_divisor(int argc, char *const argv[])
{
	int rc;
	uint32_t val;

	if (argc == 2) {
		rc = aspeed_fsi_divisor(fsi, 0);
		if (rc > 0)
			printf("divsior: %d\n", rc);
	} else if (argc == 3) {
		val = simple_strtoul(argv[2], NULL, 0);
		rc = aspeed_fsi_divisor(fsi, val);
	} else {
		printf("invalid arguments to divisor\n");
		return;
	}

	if (rc < 0)
		printf("divisor error: %d\n", rc);
}

static struct fsi_master_aspeed *do_probe(int argc, char *const argv[])
{
	struct udevice *dev;
	const char *devices[] = {"fsi@1e79b000", "fsi@1e79b100"};
	int rc, id;

	if (argc > 3) {
		printf("invalid arguments to probe\n");
		return NULL;
	}

	if (argc == 2)
		id = 0;
	else
		id = simple_strtoul(argv[2], NULL, 10);

	if (id > 1) {
		printf("valid devices: 0, 1\n");
		return NULL;
	}

	rc = uclass_get_device_by_name(UCLASS_MISC, devices[id], &dev);
	if (rc) {
		printf("fsi device %s not found\n", devices[id]);
		return NULL;
	}
	return dev_get_priv(dev);
}


static int do_fsi(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{

	if (!strcmp(argv[1], "probe")) {
		fsi = do_probe(argc, argv);
		return 0;
	}

	if (fsi == NULL) {
		printf("Run probe first\n");
		return -EINVAL;
	}

	if (!strcmp(argv[1], "break"))
		do_break();
	else if (!strcmp(argv[1], "status"))
		do_status();
	else if (!strncmp(argv[1], "put", 3))
		do_putcfam(argc, argv);
	else if (!strncmp(argv[1], "get", 3))
		do_getcfam(argc, argv);
	else if (!strncmp(argv[1], "div", 3))
		do_divisor(argc, argv);

	return 0;
}

static char fsi_help_text[] =
	"fsi probe [<n>]\n"
	"fsi break\n"
	"fsi getcfam <addr>\n"
	"fsi putcfam <addr> <value>\n"
	"fsi divisor [<divisor>]\n"
	"fsi status\n";

U_BOOT_CMD(
	fsi, 4, 1, do_fsi,
	"IBM FSI commands",
	fsi_help_text
);

