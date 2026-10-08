# patches/ — standalone 3.18 ports for reuse on other 3.18 kernels

Apply order matters: SUSFS first, then NoMount.

## 1. SUSFS v2.3.0 (backported to 3.18)

- `susfs-3.18-v2.3.0.patch` — full port, 25 files, applies on top of a
  clean 3.18 tree (verified on lineage-21 base `91b43c0e`):
  `patch -p1 < patches/susfs-3.18-v2.3.0.patch`
- `susfs-3.18-v2.3.0-defconfig.fragment` — 10 `CONFIG_KSU_SUSFS*` lines,
  append to your defconfig.

Prerequisite: KernelSU core (`drivers/kernelsu`). The glue file
`fs/susfs_ksu_glue.c` links against KSU-core `is_ksu_domain()`.
Without KSU core, provide a stub:

    bool is_ksu_domain(void) { return false; }

Upstream reference: `simonpunk/susfs4ksu`, branch `gki-android14-6.1`
at `24743360` (14 commits past the v2.3.0 bump, all triaged:
9 already in-port, 2 N/A on 3.18, 1 NULL-deref bug fixed in-port).

## 2. NoMount 3.18 compat

- `nomount-3.18-combined.patch` — full 3.18 port for NoMount tip `c7f63e3f`
  (single file; was 01/02/03, merged byte-identical):
  shims (iterate/hash/xattr/actor/iov_iter/C89) + symlink delegation
  (follow_link/put_link/readlink to real inode, put_link NULL-guarded)
  + classic `.read/.write` via `new_sync_read/write` (NOT `do_sync_*`)
  + `mutex_init(&nomount_mutex)` in `nomount_init()`. mmap untouched.
  1. `git clone --depth 1 https://github.com/maxsteeel/nomount.git NoMount`
  2. `ln -sfn ../NoMount/kernel/src fs/nomount`
  3. wire `obj-$(CONFIG_NOMOUNT)` + Kconfig source (see CI step
     "Setup NoMount" in `.github/workflows/build.yml`)
  4. `patch -p1 --forward < patches/nomount-3.18-combined.patch`
- `nomount-3.18-defconfig.fragment` — `CONFIG_NOMOUNT=y`
  (single-option subsystem; upstream default is y).

Note: the compat patch is pinned to the tested tip SHA. A newer NoMount
tip may need shim updates — re-run the `nomount.o` compile check
(see CI step "Verify KSU+SUSFS linked": `test -f out/fs/nomount/built-in.o`
+ `nm | grep nomount_init`).

## CI consumption (this repo)

- `patches/nomount-3.18-*.patch` is applied by CI after the ephemeral
  NoMount clone; NoMount source is never committed.
- SUSFS files are in-tree (not patched in CI).
- Fragments are reference only; this repo's defconfig already carries
  the options.
