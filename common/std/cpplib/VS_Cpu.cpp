/**
 **************************************************************************
 * \file VS_Cpu.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Implementation of functions for CPU capabilities detection
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 13.02.2004
 *
 * $Revision: 7 $
 *
 * $History: VS_Cpu.cpp $
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 27.05.11   Time: 17:10
 * Updated in $/VSNA/std/cpplib
 * - fix cpu info (0x00000142 error)
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 17.02.11   Time: 15:34
 * Updated in $/VSNA/std/cpplib
 * - fix number of cpu cores for benchmark
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 20.07.10   Time: 19:10
 * Updated in $/VSNA/std/cpplib
 * - were added detect number of cpu cores
 * - SystemBenchmark now detetct number of cpu cores
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 28.10.09   Time: 15:34
 * Updated in $/VSNA/std/cpplib
 * VS_GetCPUType fixed unresolved symbol export
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 28.10.09   Time: 15:18
 * Updated in $/VSNA/std/cpplib
 * VS_GetCPUInternalName fixed unresolved symbol export
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 28.10.09   Time: 14:07
 * Updated in $/VSNA/std/cpplib
 * - VS_GetCPUInternalName return type changed
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/

#if defined(_WIN32) // Not ported yet

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Cpu.h"

#include <Windows.h>
#include "intrin.h"

#include <cstdlib>

/****************************************************************************
 * Defines
 ****************************************************************************/
#define NCAPINTS	4	///< Currently we have 4 32-bit words worth of info

/**
 **************************************************************************
 * \brief CPU info structure
 ****************************************************************************/
typedef struct _cpuinfo_x86{
	unsigned char x86;			///< CPU family
	unsigned char 	x86_vendor;	///< CPU vendor
	unsigned char 	x86_model;
	unsigned char 	x86_mask;
	char	wp_works_ok;		///< It doesn't on 386's
	char	hlt_works_ok;		///< Problems on some 486Dx4's and old 386's
	char	hard_math;
	char	rfu;
	int	cpuid_level;			///< Maximum supported CPUID level, -1=no CPUID
	int	x86_capability[NCAPINTS];
	char	x86_model_id[64];
	int 	x86_cache_size;		///< in KB - valid for CPUS which support this call
	int	fdiv_bug;
	int	f00f_bug;
	int	coma_bug;
	unsigned long *pgd_quick;
	unsigned long *pmd_quick;
	unsigned long *pte_quick;
	unsigned long pgtable_cache_sz;
} cpuinfo_x86;


static _inline int constant_test_bit(int nr, const volatile void * addr)
{
	return ((1UL << (nr & 31)) & (((const volatile unsigned int *) addr)[nr >> 5])) != 0;
}

#define test_bit(nr, addr) constant_test_bit((nr), (addr))

//@{ \name EFLAGS bits
/****************************************************/
#define X86_EFLAGS_CF	0x00000001 ///< Carry Flag
#define X86_EFLAGS_PF	0x00000004 ///< Parity Flag
#define X86_EFLAGS_AF	0x00000010 ///< Auxillary carry Flag
#define X86_EFLAGS_ZF	0x00000040 ///< Zero Flag
#define X86_EFLAGS_SF	0x00000080 ///< Sign Flag
#define X86_EFLAGS_TF	0x00000100 ///< Trap Flag
#define X86_EFLAGS_IF	0x00000200 ///< Interrupt Flag
#define X86_EFLAGS_DF	0x00000400 ///< Direction Flag
#define X86_EFLAGS_OF	0x00000800 ///< Overflow Flag
#define X86_EFLAGS_IOPL	0x00003000 ///< IOPL mask
#define X86_EFLAGS_NT	0x00004000 ///< Nested Task
#define X86_EFLAGS_RF	0x00010000 ///< Resume Flag
#define X86_EFLAGS_VM	0x00020000 ///< Virtual Mode
#define X86_EFLAGS_AC	0x00040000 ///< Alignment Check
#define X86_EFLAGS_VIF	0x00080000 ///< Virtual Interrupt Flag
#define X86_EFLAGS_VIP	0x00100000 ///< Virtual Interrupt Pending
#define X86_EFLAGS_ID	0x00200000 ///< CPUID detection flag
//@}



//@{ \name Intel-defined CPU features, CPUID level 0x00000001, word 0
/****************************************************/
#define X86_FEATURE_FPU		(0*32+ 0)	///< Onboard FPU
#define X86_FEATURE_VME		(0*32+ 1)	///< Virtual Mode Extensions
#define X86_FEATURE_DE		(0*32+ 2)	///< Debugging Extensions
#define X86_FEATURE_PSE 	(0*32+ 3)	///< Page Size Extensions
#define X86_FEATURE_TSC		(0*32+ 4)	///< Time Stamp Counter
#define X86_FEATURE_MSR		(0*32+ 5)	///< Model-Specific Registers, RDMSR, WRMSR
#define X86_FEATURE_PAE		(0*32+ 6)	///< Physical Address Extensions
#define X86_FEATURE_MCE		(0*32+ 7)	///< Machine Check Architecture
#define X86_FEATURE_CX8		(0*32+ 8)	///< CMPXCHG8 instruction
#define X86_FEATURE_APIC	(0*32+ 9)	///< Onboard APIC
#define X86_FEATURE_SEP		(0*32+11)	///< SYSENTER/SYSEXIT
#define X86_FEATURE_MTRR	(0*32+12)	///< Memory Type Range Registers
#define X86_FEATURE_PGE		(0*32+13)	///< Page Global Enable
#define X86_FEATURE_MCA		(0*32+14)	///< Machine Check Architecture
#define X86_FEATURE_CMOV	(0*32+15)	///< CMOV instruction (FCMOVCC and FCOMI too if FPU present)
#define X86_FEATURE_PAT		(0*32+16)	///< Page Attribute Table
#define X86_FEATURE_PSE36	(0*32+17)	///< 36-bit PSEs
#define X86_FEATURE_PN		(0*32+18)	///< Processor serial number
#define X86_FEATURE_CLFLSH	(0*32+19)	///< Supports the CLFLUSH instruction
#define X86_FEATURE_DTES	(0*32+21)	///< Debug Trace Store
#define X86_FEATURE_ACPI	(0*32+22)	///< ACPI via MSR
#define X86_FEATURE_MMX		(0*32+23)	///< Multimedia Extensions
#define X86_FEATURE_FXSR	(0*32+24)	///< FXSAVE and FXRSTOR instructions (fast save and restore
										// of FPU context), and CR4.OSFXSR available
#define X86_FEATURE_XMM		(0*32+25)	///< Streaming SIMD Extensions
#define X86_FEATURE_XMM2	(0*32+26)	///< Streaming SIMD Extensions-2
#define X86_FEATURE_SELFSNOOP (0*32+27) ///< CPU self snoop
#define X86_FEATURE_ACC		(0*32+29)	///< Automatic clock control
#define X86_FEATURE_IA64	(0*32+30)	///< IA-64 processor
//@}

//@{ \name AMD-defined CPU features, CPUID level 0x80000001, word 1
// Don't duplicate feature flags which are redundant with Intel!
#define X86_FEATURE_SYSCALL	(1*32+11)	///< SYSCALL/SYSRET
#define X86_FEATURE_MMXEXT	(1*32+22)	///< AMD MMX extensions
#define X86_FEATURE_LM		(1*32+29)	///< Long Mode (x86-64)
#define X86_FEATURE_3DNOWEXT  (1*32+30) ///< AMD 3DNow! extensions
#define X86_FEATURE_3DNOW	(1*32+31)	///< 3DNow!
//@}

//@{ \name Transmeta-defined CPU features, CPUID level 0x80860001, word 2
#define X86_FEATURE_RECOVERY (2*32+ 0)	///< CPU in recovery mode
#define X86_FEATURE_LONGRUN	(2*32+ 1)	///< Longrun power control
#define X86_FEATURE_LRTI	(2*32+ 3)	///< LongRun table interface
//@}

//@{ \name Other features, Linux-defined mapping, word 3
// This range is used for feature bits which conflict or are synthesized
#define X86_FEATURE_CXMMX	(3*32+ 0)		///< Cyrix MMX extensions
#define X86_FEATURE_K6_MTRR	(3*32+ 1)		///< AMD K6 nonstandard MTRRs
#define X86_FEATURE_CYRIX_ARR	(3*32+ 2)	///< Cyrix ARRs (= MTRRs)
#define X86_FEATURE_CENTAUR_MCR	(3*32+ 3)	///< Centaur MCRs (= MTRRs)
//@}

/**
 ******************************************************************************
 * \fn cpuid_eax
 * \fn cpuid_ebx
 * \fn cpuid_ecx
 * \fn cpuid_edx
 * CPUID routines
 * \return cpuid result for each registers
 *
 * \param op					[IN]  - input to move register number
 *
 *  \date    15-10-2002
 ******************************************************************************
*/
static _inline unsigned int cpuid_eax(unsigned int op)
{
	int a[4];
	__cpuid(a, op);
	return a[0];
}

static _inline unsigned int cpuid_ebx(unsigned int op)
{
	int a[4];
	__cpuid(a, op);
	return a[1];
}

static _inline unsigned int cpuid_ecx(unsigned int op)
{
	int a[4];
	__cpuid(a, op);
	return a[2];
}

static _inline unsigned int cpuid_edx(unsigned int op)
{
	int a[4];
	__cpuid(a, op);
	return a[3];
}


/**
 ******************************************************************************
 * CPUID routines
 * \return none
 *
 * \param op					[IN]  - input to move register number
 * \param eax					[out]  - pointer to move from register
 * \param ebx					[out]  - pointer to move from register
 * \param ecx					[out]  - pointer to move from register
 * \param edx					[out]  - pointer to move from register
 *
 *  \date    15-10-2002
 ******************************************************************************
*/
static _inline void cpuid(unsigned int op, unsigned int *eax,unsigned  int *ebx,unsigned  int *ecx,unsigned  int *edx)
{
	int a[4];
	__cpuid(a, op);
	*eax = a[0]; *ebx = a[1]; *ecx = a[2]; *edx = a[3];
}

//!< main cpuid routines
/******************************************/

struct cpu_model_info
{
	int vendor;
	int family;
	char *model_names[16];
};

/**
 ******************************************************************************
 * get model name
 * \return 1 or 0
 *
 * \param c						[IN]  - cpu info structure
 *
 *  \date    15-10-2002
 ******************************************************************************
*/
int get_model_name(cpuinfo_x86 *c)
{
	unsigned int *v;
	char *p, *q;

	if (cpuid_eax(0x80000000) < 0x80000004)	return 0;

	v = (unsigned int *) c->x86_model_id;
	cpuid(0x80000002, &v[0], &v[1], &v[2], &v[3]);
	cpuid(0x80000003, &v[4], &v[5], &v[6], &v[7]);
	cpuid(0x80000004, &v[8], &v[9], &v[10], &v[11]);
	c->x86_model_id[48] = 0;

	/* Intel chips right-justify this string for some dumb reason;
	   undo that brain damage */
	p = q = &c->x86_model_id[0];
	while ( *p == ' ' )
	     p++;
	if ( p != q ) {
	     while ( *p )
		  *q++ = *p++;
	     while ( q <= &c->x86_model_id[48] )
		  *q++ = '\0';	/* Zero-pad the rest */
	}

	return 1;
}


/**
 ******************************************************************************
 * Standard macro to see if a specific flag is changeable
 * \return changeble?
 *
 * \param flag						[IN]  - flag to check
 *
 *  \date    15-10-2002
 ******************************************************************************
*/


#pragma warning( disable : 4731) //<! epb changed
static _inline int flag_is_changeable_p(int flag)
{
	__writeeflags(__readeflags() | flag);
	return __readeflags() & flag;
}
#pragma warning( default : 4731)


/**
 ******************************************************************************
 * Probe for the CPUID instruction
 * \return TRUE if have
 *
 *  \date    15-10-2002
 ******************************************************************************
*/


static int   have_cpuid_p(void)
{
	return flag_is_changeable_p(X86_EFLAGS_ID);
}


/**
 ******************************************************************************
 * Try to detect a CPU with disabled CPUID, and if so, enable.  This routine
 * may also be used to detect non-CPUID processors and fill in some of
 * the information manually.
 *
 * \return have cpuid?
 *
 * \param c						[out]  - cpu info structure to fill
 *
 *  \date    15-10-2002
 ******************************************************************************
*/


static int   id_and_try_enable_cpuid(cpuinfo_x86 *c)
{
	/* First of all, decide if this is a 486 or higher */
	/* It's a 486 if we can modify the AC flag */
	if ( flag_is_changeable_p(X86_EFLAGS_AC) )
		c->x86 = 4;
	else
		c->x86 = 3;

	return have_cpuid_p();	/* Check to see if CPUID now enabled? */
}



void   identify_cpu(cpuinfo_x86 *c)
{
	unsigned int junk ;
	unsigned int xlvl, tfms;
	char c_[16];
	c->x86_cache_size = -1;
	c->cpuid_level = -1;			/* CPUID not detected */
	c->x86_model = c->x86_mask = 0;	/* So far unknown... */

	c->x86_model_id[0] = '\0';		/* Unset */
	memset(&c->x86_capability, 0, sizeof c->x86_capability);


	if ( !have_cpuid_p() && !id_and_try_enable_cpuid(c) ) {
		/* CPU doesn't have CPUID */

		/* If there are any capabilities, they're vendor-specific */
		/* enable_cpuid() would have set c->x86 for us. */

	} else {
		/* CPU does have CPUID */
		/* Get vendor name */
		cpuid(0x00000000, (unsigned int *)&c->cpuid_level,
		      (unsigned int *)&c_[0],
		      (unsigned int *)&c_[8],
		      (unsigned int *)&c_[4]);

		/* Intel-defined flags: level 0x00000001 */
		if ( c->cpuid_level >= 0x00000001 ) {
			cpuid(0x00000001, &tfms, &junk, &junk,
			      (unsigned int*)&c->x86_capability[0]);
			c->x86 = (tfms >> 8) & 15;
			c->x86_model = (tfms >> 4) & 15;
			c->x86_mask = tfms & 15;
		} else {
			/* Have CPUID level 0 only - unheard of */
			c->x86 = 4;
		}

		/* AMD-defined flags: level 0x80000001 */
		xlvl = cpuid_eax(0x80000000);
		if ( (xlvl & 0xffff0000) == 0x80000000 ) {
			if ( xlvl >= 0x80000001 )
				c->x86_capability[1] = cpuid_edx(0x80000001);
			if ( xlvl >= 0x80000004 )
				get_model_name(c); /* Default name */
		}

		/* Transmeta-defined flags: level 0x80860001 */
		xlvl = cpuid_eax(0x80860000);
		if ( (xlvl & 0xffff0000) == 0x80860000 ) {
			if (  xlvl >= 0x80860001 )
				c->x86_capability[2] = cpuid_edx(0x80860001);
		}
	}
}

/**
 *****************************************************************************
 * Check Intel CPU for SIMD instructions
 *
 * \return features
 *
 *  \date    15-10-2002
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
	int VS_GetCPUType()
	{
		cpuinfo_x86 cpu_info = {0, 0, 0, 0, -1, 1, 0, 0, -1};
		identify_cpu(&cpu_info);
		int ret = VS_CPU_PURE;
		if (test_bit(X86_FEATURE_XMM2, cpu_info.x86_capability)) ret|= VS_CPU_SSE2;
		if (test_bit(X86_FEATURE_XMM, cpu_info.x86_capability)) ret|= VS_CPU_SSE;
		if (test_bit(X86_FEATURE_MMXEXT, cpu_info.x86_capability)) ret|= VS_CPU_ISSE;
		if (test_bit(X86_FEATURE_MMX, cpu_info.x86_capability)) ret|= VS_CPU_MMX;
		return ret;
	}
#ifdef __cplusplus
}
#endif

/**
 *****************************************************************************
 * Check Intel CPU for intrnal name
 *
 * \return true
 *
 *  \date    15-10-2002
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

	int VS_GetCPUInternalName(char* name, int size)
	{
		if (!name) return 0;
		*name = 0;
		if (size < 128) return 0;
		cpuinfo_x86 cpu_info = { 0, 0, 0, 0, -1, 1, 0, 0, -1 };
		identify_cpu(&cpu_info);
		memcpy(name, cpu_info.x86_model_id, 64);
		name[64] = 0;
		return 1;
	}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif

	typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

	DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
		DWORD i;

		for (i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest)?1:0);
			bitTest/=2;
		}

		return bitSetCount;
	}

	void VS_GetNumCPUCores(unsigned int *phcores, unsigned int *lcores)
	{
		BOOL ret = 0;
		LPFN_GLPI glpi = 0;
		glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetLogicalProcessorInformation");

		if (glpi != NULL) {
			*lcores = 0;
			*phcores = 0;
			DWORD len = 0, offset = 0, offset_info = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuffer = 0, pCurr;
			ret = glpi(pBuffer, &len);
			if (ret == FALSE && len > 0) {
				pBuffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(len);
				ret = glpi(pBuffer, &len);
				if (ret == TRUE) {
					pCurr = pBuffer;
					while (offset + offset_info <= len) {
						switch (pCurr->Relationship)
						{
						case RelationNumaNode: break;
						case RelationProcessorCore:
							*phcores += 1;
							*lcores += CountSetBits(pCurr->ProcessorMask);
							break;
						case RelationCache: break;
						case RelationProcessorPackage: break;
						default: break;
						}
						offset += offset_info;
						pCurr++;
					}
					free(pBuffer);
					ret = TRUE;
				}
			}
		}

		if (ret == FALSE) {
			SYSTEM_INFO sysInfo;

			GetSystemInfo(&sysInfo);
			*phcores = sysInfo.dwNumberOfProcessors;
			*lcores = 1;

			if (sysInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) {
				*lcores = sysInfo.dwNumberOfProcessors;
				return;
			}

			//unsigned int eax, ebx, ecx, edx;

			//cpuid(0x00000001, &eax, &ebx, &ecx, &edx);

			//if (edx & 0x10000000)
			//	*lcores = (ebx & 0x00ff0000) >> 16;

			//*phcores /= *lcores;

			*lcores = sysInfo.dwNumberOfProcessors;
		}
	}
#ifdef __cplusplus
}
#endif

#else

#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <cstring>
#include <algorithm>

class ICpuDetector {

	public:

		virtual std::string get_model_name() = 0;
		virtual void get_num_cpu_cores(unsigned *phcores, unsigned *lcores) = 0;

		virtual ~ICpuDetector() {}

		static ICpuDetector *Instance();

	private:
		static ICpuDetector* s_instance;

};

ICpuDetector* ICpuDetector::s_instance = nullptr;

class ProcCpuInfoDetector : public ICpuDetector  {

	class CoreInfo {
	 public:

		CoreInfo() {};

		std::string get_model_name() {
			return m_core_info_map["model name"];
		}
		int get_physical_id() {
			try {
				return std::stoi(m_core_info_map["physical id"]);
			}
			catch (...) {
				return -1;
			}
		}
		bool empty() {
			return m_core_info_map.empty();
		}
		void operator << (std::istream& is) {
			std::string line;
			for (std::getline(is, line); !line.empty(); std::getline(is, line)) {
				auto left_str = line.substr(0, line.find_first_of(":"));
				auto right_str = line.substr(line.find_first_of(":") + 1, std::string::npos);
				m_core_info_map[left_str.substr(0, left_str.find_last_not_of(" \t") + 1)] = right_str.substr(0, right_str.find_last_not_of(" \t") + 1);
			}
		}

	 private:
		std::map<std::string, std::string> m_core_info_map;
	};


	void read_proc_cpu_info() {
		if (m_core_infos.empty()) {
			std::ifstream fcpuinfo("/proc/cpuinfo");
			if (!fcpuinfo.is_open()) {
				return;
			}
			while (!fcpuinfo.eof()) {
				CoreInfo core_info;
				core_info << fcpuinfo;
				if (!core_info.empty()) {
					m_core_infos.push_back(core_info);
				}
			}
		}
	}

	std::string get_model_name() override {
		read_proc_cpu_info();
		return !m_core_infos.empty() ? m_core_infos.front().get_model_name() : "";
	}

	void get_num_cpu_cores(unsigned *phcores, unsigned *lcores) override {
		read_proc_cpu_info();
		std::set<int> physical_ids;
		int physical_id;
		for (auto& core_info : m_core_infos) {
			if ((physical_id = core_info.get_physical_id()) != -1) {
				physical_ids.insert(physical_id);
			}
		}
		*phcores = physical_ids.size();
		*lcores = m_core_infos.size();
	}

	std::vector<CoreInfo> m_core_infos;
};

class DefaultCpuDetector : public ICpuDetector {

	std::string get_model_name() override {
		std::ifstream fcpuinfo("/proc/cpuinfo");
		for (std::string line; !fcpuinfo.eof(); std::getline(fcpuinfo, line)) {
			if (line.find("model name") != std::string::npos) {
				return line.substr(line.find_first_of(":") + 1, std::string::npos);
			}
		}
		return ""; // 'model name' not found
	}

	void get_num_cpu_cores(unsigned *phcores, unsigned *lcores) override {
		std::set<int> physical_id_set;
		for (*lcores = 0;
			read_physical_id(*lcores, physical_id_set);
			(*lcores)++);
		*phcores = physical_id_set.size();
	}

	bool read_physical_id(int logical_core_id, std::set<int>& physical_id_set) {
		std::ifstream id_filestream(get_physical_id_filename(logical_core_id), std::ios::binary);
		if (id_filestream.is_open()) {
			int id;
			id_filestream >> id;
			physical_id_set.insert(id);
		}
		return id_filestream.is_open();
	}

	std::string get_physical_id_filename(int logical_core_id) {
		//template path to file with physical core id
		static const char path_to_id[] = "/sys/devices/system/cpu/cpu%d/topology/physical_package_id";
		char id_filename[256];
		sprintf(id_filename, path_to_id, logical_core_id);
		return id_filename;
	}

};

ICpuDetector *ICpuDetector::Instance()
{
	if (s_instance == nullptr) {
		s_instance = new DefaultCpuDetector();
	}
	return s_instance;
}

#ifdef __cplusplus
extern "C" {
#endif

	int VS_GetCPUInternalName(char* name, int size) {
		auto model_name = ICpuDetector::Instance()->get_model_name();
		if (size > 0) {

			size = std::min(static_cast<int>(model_name.length()), size - 1);
			memcpy(name, model_name.c_str(), size);
			name[size] = '\0';
		}
		return size>0?1:0;
	}

	void VS_GetNumCPUCores(unsigned int *phcores, unsigned int *lcores) {
		ICpuDetector::Instance()->get_num_cpu_cores(phcores, lcores);
	}

#ifdef __cplusplus
}
#endif

#endif
