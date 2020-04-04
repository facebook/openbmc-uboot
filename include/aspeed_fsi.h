struct fsi_master_aspeed;

int aspeed_fsi_read(struct fsi_master_aspeed *aspeed, int link,
		    uint32_t addr, void *val, size_t size);

int aspeed_fsi_write(struct fsi_master_aspeed *aspeed, int link,
		     uint32_t addr, const void *val, size_t size);

int aspeed_fsi_break(struct fsi_master_aspeed *aspeed, int link);

int aspeed_fsi_status(struct fsi_master_aspeed *aspeed);

int aspeed_fsi_divisor(struct fsi_master_aspeed *aspeed, uint16_t divisor);

