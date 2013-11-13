#ifndef __KERN_MM_VMM_H__
#define __KERN_MM_VMM_H__

#include <defs.h>
#include <list.h>
#include <memlayout.h>
#include <rb_tree.h>
#include <sync.h>
#include <shmem.h>
#include <atomic.h>

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

struct mm_struct {
    list_entry_t mmap_list;
    rb_tree *mmap_tree;
    struct vma_struct *mmap_cache;
    pgd_t *pgdir;
    int map_count;
    uintptr_t swap_address;
    atomic_t mm_count;
    lock_t mm_lock;
    uintptr_t brk_start, brk;
    list_entry_t proc_mm_link;
};

#define le2mm(le, member)                   \
    to_struct((le), struct mm_struct, member)

#define RB_MIN_MAP_COUNT        32 // If the count of vma >32 then redblack tree link is used

struct vma_struct *find_vma(struct mm_struct *mm, uintptr_t addr);
struct vma_struct *find_vma_intersection(struct mm_struct *mm, uintptr_t start, uintptr_t end);
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
int mm_brk(struct mm_struct *mm, uintptr_t addr, size_t len);

int do_pgfault(struct mm_struct *mm, uint64_t error_code, uintptr_t addr);
bool user_mem_check(struct mm_struct *mm, uintptr_t start, size_t len, bool write);

static inline int
mm_count(struct mm_struct *mm) {
    return atomic_read(&(mm->mm_count));
}

static inline void
set_mm_count(struct mm_struct *mm, int val) {
    atomic_set(&(mm->mm_count), val);
}

static inline int
mm_count_inc(struct mm_struct *mm) {
    return atomic_add_return(&(mm->mm_count), 1);
}

static inline int
mm_count_dec(struct mm_struct *mm) {
    return atomic_sub_return(&(mm->mm_count), 1);
}

static inline void
lock_mm(struct mm_struct *mm) {
    if (mm != NULL) {
        lock(&(mm->mm_lock));
    }
}

static inline void
unlock_mm(struct mm_struct *mm) {
    if (mm != NULL) {
        unlock(&(mm->mm_lock));
    }
}

#endif /* !__KERN_MM_VMM_H__ */

