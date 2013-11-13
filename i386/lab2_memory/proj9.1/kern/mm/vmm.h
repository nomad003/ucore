#ifndef __KERN_MM_VMM_H__
#define __KERN_MM_VMM_H__

#include <defs.h>
#include <list.h>
#include <memlayout.h>
#include <rb_tree.h>
#include <sync.h>
#include <shmem.h>

//pre define
struct mm_struct;

// the virtual continuous memory area(vma)
struct vma_struct {
    struct mm_struct *vm_mm; // the set of vma using the same PDT 
    uintptr_t vm_start;      //	start addr of vma	
    uintptr_t vm_end;        // end addr of vma
    uint32_t vm_flags;       // flags of vma
    rb_node rb_link;         // redblack link which sorted by start addr of vma
    list_entry_t list_link;  // linear list link which sorted by start addr of vma
    struct shmem_struct *shmem;
    size_t shmem_off;
};

#define le2vma(le, member)                  \
    to_struct((le), struct vma_struct, member)

#define rbn2vma(node, member)               \
    to_struct((node), struct vma_struct, member)

#define VM_READ                 0x00000001
#define VM_WRITE                0x00000002
#define VM_EXEC                 0x00000004
#define VM_STACK                0x00000008
#define VM_SHARE                0x00000010

// the control struct for a set of vma using the same PDT
struct mm_struct {
    list_entry_t mmap_list;        // linear list link which sorted by start addr of vma
    rb_tree *mmap_tree;            // redblack tree link which sorted by start addr of vma
    struct vma_struct *mmap_cache; // current accessed vma, used for speed purpose
    pde_t *pgdir;                  // the PDT of these vma
    int map_count;                 // the count of these vma
    uintptr_t swap_address;
};

#define RB_MIN_MAP_COUNT        32 // If the count of vma >32 then redblack tree link is used

struct vma_struct *find_vma(struct mm_struct *mm, uintptr_t addr);
struct vma_struct *vma_create(uintptr_t vm_start, uintptr_t vm_end, uint32_t vm_flags);
void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma);

struct mm_struct *mm_create(void);
void mm_destroy(struct mm_struct *mm);

void vmm_init(void);
int mm_map(struct mm_struct *mm, uintptr_t addr, size_t len, uint32_t vm_flags,
        struct vma_struct **vma_store);
int mm_map_shmem(struct mm_struct *mm, uintptr_t addr, uint32_t vm_flags,
        struct shmem_struct *shmem, struct vma_struct **vma_store);
int mm_unmap(struct mm_struct *mm, uintptr_t addr, size_t len);
int dup_mmap(struct mm_struct *to, struct mm_struct *from);
void exit_mmap(struct mm_struct *mm);
uintptr_t get_unmapped_area(struct mm_struct *mm, size_t len);

int do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr);
bool user_mem_check(struct mm_struct *mm, uintptr_t start, size_t len, bool write);

#endif /* !__KERN_MM_VMM_H__ */

