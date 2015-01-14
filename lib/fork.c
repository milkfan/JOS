// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	int pn = PGNUM( addr );
	if( ( err & FEC_WR ) == 0 || ( uvpd[PDX( addr )] & PTE_P ) == 0 || ( uvpt[pn] & PTE_COW ) == 0 )
		panic( "lib/fork.c: pgfault()" );

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if( ( r = sys_page_alloc( 0 , ( void * ) PFTEMP , PTE_U | PTE_P | PTE_W )) < 0 )
		panic( "lib/fork.c: pgfault()" );
	addr = ROUNDDOWN( addr , PGSIZE );
	memcpy( PFTEMP , addr , PGSIZE );
	if( ( r = sys_page_map( 0 , PFTEMP , 0 , addr , PTE_U | PTE_P | PTE_W ))< 0 )
		panic( "lib/fork.c: pgfault()" );
	if( ( r = sys_page_unmap( 0 , PFTEMP ) ) < 0 )
		panic( "lib/fork.c: pgfault()" );
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");
	void *addr = (void *)((uint32_t)pn*PGSIZE); 
	pte_t pte = uvpt[PGNUM(addr)];

	if ((pte & PTE_W) > 0 || (pte & PTE_COW) > 0) { 
		// map the va into the envid's address space
		r = sys_page_map(0, addr, envid, addr, PTE_U|PTE_P|PTE_COW); 
		if (r < 0)
			panic ("lib/fork.c/duppage(): sys_page_map (new) failed: %e", r);
		//map the va (again) into the envid's address space 
		//this is a mechanism to guarantee consistency of the shared page, 
		//i.e., if the old process want to modify the page, it has to COW as well! 
		r = sys_page_map(0, addr, 0, addr, PTE_U|PTE_P|PTE_COW);
		if (r < 0) 
			panic ("lib/fork.c/duppage(): sys_page_map (old) failed: %e", r); 
	} 
	else { 
	//map the va (read-only) 
	r = sys_page_map(0, addr, envid, addr, PTE_U|PTE_P); 
	if (r < 0)
		panic ("lib/fork.c/duppage(): sys_page_map (read-only) failed: %e", r);
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
	set_pgfault_handler(pgfault);
	envid_t envid;
	if ((envid = sys_exofork()) < 0) 
		panic("lib/fork.c/fork(): %e", envid);
	if (envid == 0) {
		// We're the child.
		thisenv = &envs[ENVX(sys_getenvid())]; 
		return 0;
	}
	// We're the parent. 
	uint32_t addr;
	for (addr = UTEXT; addr < UXSTACKTOP - PGSIZE; addr += PGSIZE) { 
		if ( (uvpd[PDX(addr)] & PTE_P) > 0 && (uvpt[PGNUM(addr)] & PTE_P) > 0 && (uvpt[PGNUM(addr)] & PTE_U) > 0) 
			duppage (envid, PGNUM(addr)); 
	}
	// alloc a page for child's exception stack
	int r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U|PTE_W|PTE_P);
	if (r < 0)
		panic ("lib/fork.c/fork(): sys_page_alloc failed: %e", r);
	//Why? Because the new env cannot set its _pgfault_upcall by itself! 
	//When it go out of here, it will behave the same as its father. 
	extern void _pgfault_upcall(void); 
	sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
	r = sys_env_set_status(envid, ENV_RUNNABLE); 
	if (r < 0)
		panic("lib/fork.c/fork(): set child env status failed : %e", r);
	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
