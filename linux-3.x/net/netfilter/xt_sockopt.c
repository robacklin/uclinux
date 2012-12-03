#include <linux/module.h>
#include <linux/if.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter/xt_sockopt.h>
#include <linux/netfilter/x_tables.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Philip Craig <philipc@snapgear.com>");
MODULE_DESCRIPTION("Xtables: sockopt match");
MODULE_ALIAS("ipt_sockopt");

static bool
sockopt_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_sockopt_mtinfo *info = par->matchinfo;
	unsigned long ret;
	__be32 addr;
	__u32 dev;

	if (info->match & XT_SOCKOPT_ORIGDEV) {
		dev = skb->sk ? skb->sk->sk_authd_dev : 0;
		ret = dev == info->origdev;
		if (ret ^ !(info->invert & XT_SOCKOPT_ORIGDEV))
			return false;
	}

	if (info->match & XT_SOCKOPT_ORIGSRC) {
		addr = skb->sk ? skb->sk->sk_authd_saddr : 0;
		if (info->match & XT_SOCKOPT_SRCRANGE)
			ret = addr >= info->origsrc_addr.ip && addr <= info->origsrc_mask.ip;
		else
			ret = (addr & info->origsrc_mask.ip) == info->origsrc_addr.ip;
		if (ret ^ !(info->invert & XT_SOCKOPT_ORIGSRC))
			return false;
	}

	if (info->match & XT_SOCKOPT_ORIGDST) {
		addr = skb->sk ? skb->sk->sk_authd_daddr : 0;
		if (info->match & XT_SOCKOPT_DSTRANGE)
			ret = addr >= info->origdst_addr.ip && addr <= info->origdst_mask.ip;
		else
			ret = (addr & info->origdst_mask.ip) == info->origdst_addr.ip;
		if (ret ^ !(info->invert & XT_SOCKOPT_ORIGDST))
			return false;
	}

	return true;
}

static int sockopt_mt_check(const struct xt_mtchk_param *par)
{
	return 0;
}

static struct xt_match sockopt_mt_reg __read_mostly = {
	.name       = "sockopt",
	.revision   = 0,
	.family     = NFPROTO_IPV4,
	.checkentry = sockopt_mt_check,
	.match      = sockopt_mt,
	.matchsize  = sizeof(struct xt_sockopt_mtinfo),
	.me         = THIS_MODULE,
};

static int __init sockopt_mt_init(void)
{
	return xt_register_match(&sockopt_mt_reg);
}

static void __exit sockopt_mt_exit(void)
{
	xt_unregister_match(&sockopt_mt_reg);
}

module_init(sockopt_mt_init);
module_exit(sockopt_mt_exit);
