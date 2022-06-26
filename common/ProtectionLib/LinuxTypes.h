#pragma once

#if !defined(__linux__)
#	error This file should be used only on Linux
#endif

#include <linux/types.h>

// Definition of several structures used in the Linux kernel in user API.

// Structure used by getdents64 syscall.
// See linux-4.9.193/include/linux/dirent.h
struct linux_dirent64
{
	__u64 d_ino;
	__s64 d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[0];
};

// Constants used in linux_dirent64::d_type field
// See linux-4.9.193/include/linux/fs.h
static const unsigned char linux_DT_UNKNOWN = 0;
static const unsigned char linux_DT_FIFO    = 1;
static const unsigned char linux_DT_CHR     = 2;
static const unsigned char linux_DT_DIR     = 4;
static const unsigned char linux_DT_BLK     = 6;
static const unsigned char linux_DT_REG     = 8;
static const unsigned char linux_DT_LNK     = 10;
static const unsigned char linux_DT_SOCK    = 12;
static const unsigned char linux_DT_WHT     = 14;

// Structure used by sysinfo syscall.
// See linux-4.9.193/include/uapi/linux/sysinfo.h
struct linux_sysinfo
{
	__kernel_long_t uptime;
	__kernel_ulong_t loads[3];
	__kernel_ulong_t totalram;
	__kernel_ulong_t freeram;
	__kernel_ulong_t sharedram;
	__kernel_ulong_t bufferram;
	__kernel_ulong_t totalswap;
	__kernel_ulong_t freeswap;
	__u16 procs;
	__u16 pad;
	__kernel_ulong_t totalhigh;
	__kernel_ulong_t freehigh;
	__u32 mem_unit;
	char _f[20-2*sizeof(__kernel_ulong_t)-sizeof(__u32)];
};

#if __i386__

// Structure used by stat,lstat,fstat syscalls on x86.
// See linux-4.9.193/arch/x86/include/uapi/asm/stat.h
struct linux_stat
{
	unsigned long  st_dev;
	unsigned long  st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned long  st_rdev;
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  st_atime_nsec;
	unsigned long  st_mtime;
	unsigned long  st_mtime_nsec;
	unsigned long  st_ctime;
	unsigned long  st_ctime_nsec;
	unsigned long  __unused4;
	unsigned long  __unused5;
};

// Structure used by stat64,lstat64,fstat64,fstatat64 syscalls on x86.
// See linux-4.9.193/arch/x86/include/uapi/asm/stat.h
struct linux_stat64
{
	unsigned long long st_dev;
	unsigned char      __pad0[4];
	unsigned long      __st_ino;
	unsigned int       st_mode;
	unsigned int       st_nlink;
	unsigned long      st_uid;
	unsigned long      st_gid;
	unsigned long long st_rdev;
	unsigned char      __pad3[4];
	long long          st_size;
	unsigned long      st_blksize;
	unsigned long long st_blocks;
	unsigned long      st_atime;
	unsigned long      st_atime_nsec;
	unsigned long      st_mtime;
	unsigned int       st_mtime_nsec;
	unsigned long      st_ctime;
	unsigned long      st_ctime_nsec;
	unsigned long long st_ino;
};

// Structure used by statfs,fstatfs syscalls on x86.
// See linux-4.9.193/include/uapi/asm-generic/statfs.h
struct linux_statfs
{
	__u32           f_type;
	__u32           f_bsize;
	__u32           f_blocks;
	__u32           f_bfree;
	__u32           f_bavail;
	__u32           f_files;
	__u32           f_ffree;
	__kernel_fsid_t f_fsid;
	__u32           f_namelen;
	__u32           f_frsize;
	__u32           f_flags;
	__u32           f_spare[4];
};

// Structure used by statfs64,fstatfs64 syscalls on x86.
// See linux-4.9.193/include/uapi/asm-generic/statfs.h
struct linux_statfs64
{
	__u32           f_type;
	__u32           f_bsize;
	__u64           f_blocks;
	__u64           f_bfree;
	__u64           f_bavail;
	__u64           f_files;
	__u64           f_ffree;
	__kernel_fsid_t f_fsid;
	__u32           f_namelen;
	__u32           f_frsize;
	__u32           f_flags;
	__u32           f_spare[4];
};

#elif __x86_64__

// Structure used by stat,lstat,fstat,newfstatat syscalls on x86_64.
// See linux-4.9.193/arch/x86/include/uapi/asm/stat.h
struct linux_stat
{
	__kernel_ulong_t st_dev;
	__kernel_ulong_t st_ino;
	__kernel_ulong_t st_nlink;
	unsigned int     st_mode;
	unsigned int     st_uid;
	unsigned int     st_gid;
	unsigned int     __pad0;
	__kernel_ulong_t st_rdev;
	__kernel_long_t  st_size;
	__kernel_long_t  st_blksize;
	__kernel_long_t  st_blocks;
	__kernel_ulong_t st_atime;
	__kernel_ulong_t st_atime_nsec;
	__kernel_ulong_t st_mtime;
	__kernel_ulong_t st_mtime_nsec;
	__kernel_ulong_t st_ctime;
	__kernel_ulong_t st_ctime_nsec;
	__kernel_long_t  __unused[3];
};

// Structure used by statfs,fstatfs syscalls on x86_64.
// See linux-4.9.193/include/uapi/asm-generic/statfs.h
struct linux_statfs
{
	__kernel_long_t f_type;
	__kernel_long_t f_bsize;
	__kernel_long_t f_blocks;
	__kernel_long_t f_bfree;
	__kernel_long_t f_bavail;
	__kernel_long_t f_files;
	__kernel_long_t f_ffree;
	__kernel_fsid_t f_fsid;
	__kernel_long_t f_namelen;
	__kernel_long_t f_frsize;
	__kernel_long_t f_flags;
	__kernel_long_t f_spare[4];
};

#else
#	error Unknown architecture
#endif
