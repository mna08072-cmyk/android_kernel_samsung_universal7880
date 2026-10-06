/*
 * susfs_ksu_glue.c - KSU+SUSFS integration glue for 3.18 (lineage-21)
 *
 * Glue-only KernelSU integration (submodule + symlink + drivers/Makefile
 * + drivers/Kconfig, no manual kernel hooks) leaves four SUSFS link-time
 * prerequisites without a provider:
 *
 *   - u32 susfs_ksu_sid / u32 susfs_priv_app_sid
 *       (security/selinux/avc.c log-spoof guard, CONFIG_KSU_SUSFS)
 *   - bool susfs_is_current_ksu_domain(void)
 *       (fs/susfs.c, fs/namespace.c, fs/proc_namespace.c, fs/super.c, ...)
 *   - bool is_ksu_transition(old_tsec, new_tsec)
 *       (security/selinux/hooks.c check_nnp_nosuid guard, CONFIG_KSU)
 *
 * This file provides that contract from the kernel side so the pinned
 * backslashxx KernelSU source stays untouched.
 *
 * SID model: "u:r:su:s0" is the KSU domain on this tree (KERNEL_SU_DOMAIN
 * "ksu" in KernelSU's selinux.h); priv-app spoof target
 * "u:r:priv_app:s0:c512,c768" matches the avc.c guard. SIDs are resolved
 * lazily via security_context_to_sid() because the SELinux policy is
 * loaded by userspace init, long after kernel initcalls run. Resolution
 * is only attempted from sleepable context; atomic/IRQ callers fail
 * closed (return false / keep SID 0) until a sleepable caller succeeds.
 *
 * is_ksu_transition() semantic: true when the exec credentials touch the
 * KSU domain on either side, so check_nnp_nosuid() does not block KSU's
 * own credential switches.
 */

#include <linux/cred.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/security.h>
#include <linux/string.h>
#include <linux/types.h>

/* Verified against security/selinux/include/objsec.h of this tree:
 * struct task_security_struct { u32 osid, sid, ... } */
struct task_security_struct {
	u32 osid;
	u32 sid;
};

/* Declared in security/selinux/include/security.h (internal header, not
 * on fs/'s include path); signature verified against this tree. */
int security_context_to_sid(const char *scontext, u32 scontext_len,
			    u32 *sid, gfp_t gfp);
/* security/selinux/include/avc_ss.h: set to 1 once the SELinux policy is
 * loaded (services.c:2076). Before that no task can be in the KSU domain
 * and any SID lookup would sleep/fail, so bail out early. */
extern int ss_initialized;

extern bool is_ksu_domain(void);

u32 susfs_ksu_sid;
u32 susfs_priv_app_sid;

static bool susfs_glue_sids_resolved;

static void susfs_glue_resolve_sids(void)
{
	u32 sid;
	int rc;

	if (susfs_glue_sids_resolved)
		return;
	/* Policy not loaded yet: no task can be in the KSU domain, and
	 * SID lookups would sleep/fail (early mount path). Fail closed. */
	if (!ss_initialized)
		return;
	if (in_interrupt() || in_atomic())
		return;

	rc = security_context_to_sid("u:r:su:s0", strlen("u:r:su:s0"),
				     &sid, GFP_KERNEL);
	if (!rc)
		susfs_ksu_sid = sid;
	rc = security_context_to_sid("u:r:priv_app:s0:c512,c768",
				     strlen("u:r:priv_app:s0:c512,c768"),
				     &sid, GFP_KERNEL);
	if (!rc)
		susfs_priv_app_sid = sid;
	if (susfs_ksu_sid && susfs_priv_app_sid)
		susfs_glue_sids_resolved = true;
}

bool susfs_is_current_ksu_domain(void)
{
	bool ret;

	/* Fast fail-closed: before policy load no domain check is valid. */
	if (!ss_initialized)
		return false;
	susfs_glue_resolve_sids();
	ret = is_ksu_domain();
	return ret;
}

bool is_ksu_transition(const struct task_security_struct *old_tsec,
		       const struct task_security_struct *new_tsec)
{
	susfs_glue_resolve_sids();
	if (!susfs_ksu_sid)
		return false;
	/* Pre-policy execs (e.g. early /init) never touch the KSU domain. */
	if (!ss_initialized)
		return false;
	return old_tsec->sid == susfs_ksu_sid ||
	       new_tsec->sid == susfs_ksu_sid;
}
