/*
 * susfs_supercall.c - SUSFS v2 userspace dispatch (outside KSU submodule)
 *
 * The userspace ksu_susfs tool talks to SUSFS v2 via
 * reboot(0xDEADBEEF, SUSFS_MAGIC, CMD_SUSFS_*, arg). The pinned backslashxx
 * KSU submodule's ksu_handle_sys_reboot() only understands KSU_INSTALL_MAGIC2
 * and has no SUSFS branch, so those calls fall through unhandled and the
 * tool misdetects the kernel as susfs v1.5.2 (prctl ABI).
 *
 * This file provides the missing v2 dispatch from our own side:
 * kernel/reboot.c calls susfs_handle_supercall() first (after the
 * CAP_SYS_BOOT check, before the LINUX_REBOOT_MAGIC check that would
 * reject 0xDEADBEEF with -EINVAL). Non-SUSFS calls return false and the
 * normal reboot path continues untouched.
 *
 * Two opcodes have no provider in this backport and are consumed as no-ops:
 * CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY (0x55572) and CMD_SUSFS_ADD_TRY_UMOUNT
 * (0x55580, deprecated upstream).
 */

#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/susfs.h>
#include <linux/susfs_def.h>

/* == KSU_INSTALL_MAGIC1, local so we never depend on the submodule. */
#define SUSFS_SUPERCALL_MAGIC1 0xDEADBEEF

bool susfs_handle_supercall(int magic1, int magic2, unsigned int cmd,
			    void __user **arg)
{
	if (magic1 != SUSFS_SUPERCALL_MAGIC1 || magic2 != SUSFS_MAGIC)
		return false;
	if (current_uid().val != 0)
		return false;

	switch (cmd) {
#ifdef CONFIG_KSU_SUSFS_SUS_PATH
	case CMD_SUSFS_ADD_SUS_PATH:
		susfs_add_sus_path(arg);
		return true;
	case CMD_SUSFS_ADD_SUS_PATH_LOOP:
		susfs_add_sus_path_loop(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
	case CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS:
		susfs_set_hide_sus_mnts_for_non_su_procs(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
	case CMD_SUSFS_ADD_SUS_KSTAT:
		susfs_add_sus_kstat(arg);
		return true;
	case CMD_SUSFS_UPDATE_SUS_KSTAT:
		susfs_update_sus_kstat(arg);
		return true;
#endif
	case CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY:
		/* no provider in this backport */
		return true;
	case CMD_SUSFS_ADD_TRY_UMOUNT:
		/* deprecated upstream, no provider */
		return true;
#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
	case CMD_SUSFS_SET_UNAME:
		susfs_set_uname(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
	case CMD_SUSFS_ENABLE_LOG:
		susfs_enable_log(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
	case CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG:
		susfs_set_cmdline_or_bootconfig(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
	case CMD_SUSFS_ADD_OPEN_REDIRECT:
		susfs_add_open_redirect(arg);
		return true;
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_MAP
	case CMD_SUSFS_ADD_SUS_MAP:
		susfs_add_sus_map(arg);
		return true;
#endif
	case CMD_SUSFS_ENABLE_AVC_LOG_SPOOFING:
		susfs_set_avc_log_spoofing(arg);
		return true;
	case CMD_SUSFS_SHOW_ENABLED_FEATURES:
		susfs_get_enabled_features(arg);
		return true;
	case CMD_SUSFS_SHOW_VARIANT:
		susfs_show_variant(arg);
		return true;
	case CMD_SUSFS_SHOW_VERSION:
		susfs_show_version(arg);
		return true;
	default:
		/* Unknown root magic: consume so it never hits -EINVAL. */
		return true;
	}
}
