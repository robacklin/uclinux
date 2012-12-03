#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_tsreputation.h>
#include <net/sock.h>


static bool tsreputation_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_tsreputation_info *const info = par->matchinfo;
	int32_t rep;

	if (skb != NULL && skb->sk != NULL)
		rep = skb->sk->sk_authd_tsreputation;
	else
		rep = 0;

	return info->invert ^ (rep <= info->reputation?1:0);
}

static bool tsreputation_mt_check(const struct xt_mtchk_param *par)
{
	return true;
}

static struct xt_match xt_tsreputation_mt_reg __read_mostly = {
	.name       = "tsreputation",
	.family     = NFPROTO_UNSPEC,
	.match      = tsreputation_mt,
	.checkentry = tsreputation_mt_check,
	.matchsize  = sizeof(struct xt_tsreputation_info),
	.me         = THIS_MODULE,
};

static int __init tsreputation_mt_init(void)
{
	return xt_register_match(&xt_tsreputation_mt_reg);
}

static void __exit tsreputation_mt_exit(void)
{
	xt_unregister_match(&xt_tsreputation_mt_reg);
}

module_init(tsreputation_mt_init);
module_exit(tsreputation_mt_exit);
MODULE_AUTHOR("Paul Dale <Paul_Dale@McAfee.com>");
MODULE_DESCRIPTION("Xtables: reputation based matching");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_tsreputation");
MODULE_ALIAS("ip6t_tsreputation");
