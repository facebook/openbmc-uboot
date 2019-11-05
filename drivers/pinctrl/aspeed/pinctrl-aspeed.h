struct aspeed_sig_desc {
	u32 offset;
	u32 reg_set;
	int clr;
};

struct aspeed_group_config {
	char *group_name;
	int ndescs;
	struct aspeed_sig_desc *descs;
};

