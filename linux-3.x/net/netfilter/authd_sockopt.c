/*
 * Copyright (c) 2009 McAfee, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/netfilter/authd.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_zones.h>

#ifdef CONFIG_NETFILTER_XT_MATCH_SOCKOPT_MODULE
static int get_u32(__u32 *addr, void __user *user, unsigned int len)
{
	__u32 t;

	if (len < sizeof(__u32))
		return -EINVAL;
	if (copy_from_user(&t, user, sizeof(__u32)))
		return -EFAULT;
	if (t == 0)
		return -EINVAL;
	*addr = t;
	return 0;
}
#endif


#ifdef CONFIG_NETFILTER_XT_MATCH_TSREPUTATION_MODULE
static int get_s32(__s32 *addr, void __user *user, unsigned int len)
{
	__s32 t;

	if (len < sizeof(__s32))
		return -EINVAL;
	if (copy_from_user(&t, user, sizeof(__s32)))
		return -EFAULT;
	*addr = t;
	return 0;
}
#endif


#ifdef CONFIG_IP_NF_SET_URLFRAG_MODULE
static int get_string(const char **s, void __user *user, unsigned int len)
{
	char *buf;

	if (*s != NULL) {
		kfree(*s);
		*s = NULL;
	}
	if (len == 0)
		return 0;

	if (len < 2 * sizeof(char))
		return -EINVAL;
	buf = kmalloc(len, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;
	if (copy_from_user(buf, user, len))
		return -EFAULT;
	*s = buf;
	return 0;
}
#endif

#ifdef CONFIG_IP_NF_SET_POLYNUM_MODULE
/* Grab a list of unsigned integers and build an array of them.
 * The first element of the array is used to specify the number of numbers
 * in the list.
 */
static int get_u32_list(__u32 **s, void __user *user, unsigned int len)
{
	__u32 *buf;
	const __u32 n = len / sizeof(__u32);

	if (*s != NULL) {
		kfree(*s);
		*s = NULL;
	}
	if (len == 0)
		return 0;

	if (n < 1 || len % sizeof(__u32))
		return -EINVAL;
	buf = kmalloc((n+1) * sizeof(__u32), GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;
	if (copy_from_user(buf+1, user, len))
		return -EFAULT;
	buf[0] = n;
	*s = buf;
	return 0;
}
#endif

static int authd_sockfn_set(struct sock *sk, int optval, void __user *user,
			    unsigned int len)
{
	int option;
	int res = 0;

	/* Check a few things, then read our option index */
	if (optval != SO_AUTHD)
		return -EBADF;
	if (len < sizeof(option))
		return -EINVAL;
	if (copy_from_user(&option, user, sizeof(option)))
		return -EFAULT;
	user += sizeof(option);
	len -= sizeof(option);

	bh_lock_sock(sk);
	switch (option) {
	default:
		printk(KERN_ERR "authd-sockopt: bad setsockopt option %d\n",
		       option);
		res = -EBADMSG;
		break;

	case AUTHD_OP_EXTIRPATE:	/* Extirpate unrequited data */
#ifdef CONFIG_IP_NF_SET_URLFRAG_MODULE
		if (sk->sk_authd_url != NULL) {
			kfree(sk->sk_authd_url);
			sk->sk_authd_url = NULL;
		}
#endif
#ifdef CONFIG_IP_NF_SET_POLYNUM_MODULE
		if (sk->sk_authd_groups != NULL) {
			kfree(sk->sk_authd_groups);
			sk->sk_authd_groups = NULL;
		}
		if (sk->sk_authd_ts_categories != NULL) {
			kfree(sk->sk_authd_ts_categories);
			sk->sk_authd_ts_categories = NULL;
		}
#endif
#ifdef CONFIG_NETFILTER_XT_MATCH_SOCKOPT_MODULE
		sk->sk_authd_dev = 0;
		sk->sk_authd_saddr = 0;
		sk->sk_authd_daddr = 0;
#endif
#ifdef CONFIG_NETFILTER_XT_MATCH_TSREPUTATION_MODULE
		sk->sk_authd_tsreputation = 0;
#endif
		break;

#ifdef CONFIG_NETFILTER_XT_MATCH_SOCKOPT_MODULE
	case AUTHD_OP_DEV:
		res = get_u32(&sk->sk_authd_dev, user, len);

	case AUTHD_OP_SADDR:
		res = get_u32(&sk->sk_authd_saddr, user, len);
		break;

	case AUTHD_OP_DADDR:
		res = get_u32(&sk->sk_authd_daddr, user, len);
		break;
#endif

#ifdef CONFIG_IP_NF_SET_URLFRAG_MODULE
	case AUTHD_OP_URL:
		res = get_string(&sk->sk_authd_url, user, len);
		break;
#endif

#ifdef CONFIG_IP_NF_SET_POLYNUM_MODULE
	case AUTHD_OP_GROUPS:
		res = get_u32_list(&sk->sk_authd_groups, user, len);
		break;
#endif
#ifdef CONFIG_IP_NF_SET_POLYNUM_MODULE
	case AUTHD_OP_TSCATEGORIES:
		res = get_u32_list(&sk->sk_authd_ts_categories, user, len);
		break;
#endif
#ifdef CONFIG_NETFILTER_XT_MATCH_TSREPUTATION_MODULE
	case AUTHD_OP_TSREPUTATION:
		res = get_s32(&sk->sk_authd_tsreputation, user, len);
		break;
#endif
	}
	bh_unlock_sock(sk);
	return res;
}

static int authd_sock_get_connmark(struct sock *sk, void __user *user, int *len)
{
	const struct inet_sock *inet = inet_sk(sk);
	const struct nf_conntrack_tuple_hash *h;
	struct nf_conntrack_tuple tuple;
	struct nf_conn *ct;
	u_int32_t mark;

	memset(&tuple, 0, sizeof(tuple));
	tuple.src.u3.ip = inet->inet_rcv_saddr;
	tuple.src.u.tcp.port = inet->inet_sport;
	tuple.dst.u3.ip = inet->inet_daddr;
	tuple.dst.u.tcp.port = inet->inet_dport;
	tuple.src.l3num = PF_INET;
	tuple.dst.protonum = IPPROTO_TCP;

	/* We only do TCP at the moment: is there a better way? */
	if (strcmp(sk->sk_prot->name, "TCP")) {
		pr_debug("SO_AUTHD: Not a TCP socket\n");
		return -ENOPROTOOPT;
	}

	if ((unsigned int) *len < sizeof(mark)) {
		pr_debug("SO_AUTHD: len %d not %Zu\n", *len, sizeof(mark));
		return -EINVAL;
	}

	h = nf_conntrack_find_get(sock_net(sk), NF_CT_DEFAULT_ZONE, &tuple);
	if (!h) {
		pr_debug("SO_AUTHD: Can't find %pI4/%u-%pI4/%u.\n",
			 &tuple.src.u3.ip, ntohs(tuple.src.u.tcp.port),
		 &tuple.dst.u3.ip, ntohs(tuple.dst.u.tcp.port));
		return -ENOENT;
	}

	ct = nf_ct_tuplehash_to_ctrack(h);
	mark = ct->mark;
	nf_ct_put(ct);

	if (copy_to_user(user, &mark, sizeof(mark)) != 0)
		return -EFAULT;

	*len = sizeof(mark);
	return 0;
}

static int authd_sockfn_get(struct sock *sk, int optval, void __user *user,
			    int *len)
{
	int r;

	if (optval != SO_AUTHD)
		return -EBADF;

	bh_lock_sock(sk);
	r = authd_sock_get_connmark(sk, user, len);
	bh_unlock_sock(sk);

	return r;
}

static struct nf_sockopt_ops so_set = {
	.pf 		= PF_INET,
	.get_optmin 	= SO_AUTHD,
	.get_optmax 	= SO_AUTHD + 1,
	.get 		= &authd_sockfn_get,
	.set_optmin 	= SO_AUTHD,
	.set_optmax 	= SO_AUTHD + 1,
	.set 		= &authd_sockfn_set,
};


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dr Paul Dale <pauli@snapgear.com>");
MODULE_DESCRIPTION("module implementing authd sockopts");

static int __init init(void)
{
	int res;

	res = nf_register_sockopt(&so_set);
	if (res != 0) {
		printk(KERN_ERR "authd-sockopt: SO_AUTHD registry failed: %d\n",
		       res);
		return res;
	}
	return 0;
}

static void __exit fini(void)
{
	nf_unregister_sockopt(&so_set);
	printk(KERN_INFO "authd-sockopt: module unloaded\n");
}

module_init(init);
module_exit(fini);
