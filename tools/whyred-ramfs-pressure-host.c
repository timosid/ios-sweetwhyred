/* Adapters only. The generator inserts the exact kernel declarations/functions. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <time.h>
#define PAGE_SIZE 16384ULL
#define PAGE_MASK (PAGE_SIZE - 1)
#define RAMFS_HASH_SIZE 4096u
#define Z_WAITOK 1
#define Z_NOWAIT 2
#define Z_ZERO 4
#define Z_NOFAIL 8
#define KERN_SUCCESS 0
#define KERN_RESOURCE_SHORTAGE 3
#define KMA_KOBJECT 1
#define KMA_DATA 2
#define KMA_ZERO 4
#define KMA_NONE 0
#define KMA_NOPAGEWAIT 8
#define VM_KERN_MEMORY_FILE 1
#define IO_APPEND 1
#define NULLVP NULL
#ifndef ENOATTR
#define ENOATTR 93
#endif
typedef int kern_return_t;
typedef int kma_flags_t;
typedef uintptr_t vm_offset_t;
typedef int lck_mtx_t;
typedef void *mount_t;
typedef int64_t user_ssize_t;
enum vtype { VREG = 1, VDIR = 2, VLNK = 5 };
struct ramfs_node;
struct host_vnode { struct ramfs_node *node; unsigned recycled, cleared; off_t size; };
typedef struct host_vnode *vnode_t;
struct uio { off_t offset; user_ssize_t resid; char *buf; size_t cursor; bool input; };
struct vnop_write_args { vnode_t a_vp; struct uio *a_uio; int a_ioflag; };
struct vnop_inactive_args { vnode_t a_vp; };
struct vnop_reclaim_args { vnode_t a_vp; };
struct vnop_getxattr_args { vnode_t a_vp; const char *a_name; struct uio *a_uio; size_t *a_size; };
struct vnop_listxattr_args { vnode_t a_vp; struct uio *a_uio; size_t *a_size; };
static unsigned vm_page_free_count = 123, vm_page_wire_count = 456;
static _Atomic uint64_t ramfs_pages_in_use;
static void *kernel_map;
static unsigned held, log_lines, fail_slot_wait, fail_slot_nowait, fail_copy, fail_page;
static size_t allocated_pages;
static char log_text[4096];
static void host_lock(int *p) { (void)p; assert(held++ == 0); }
static void host_unlock(int *p) { (void)p; assert(held-- == 1); }
#define lck_mtx_lock(p) host_lock(p)
#define lck_mtx_unlock(p) host_unlock(p)
#define os_atomic_inc(p, order) ((void)++*(p))
#define os_atomic_dec(p, order) ((void)--*(p))
#define os_atomic_load(p, order) (*(p))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#ifndef bzero
#define bzero(p,n) memset(p,0,n)
#endif
static void *host_slots(size_t n, int flags) {
    if (flags & Z_WAITOK) assert(held == 0);
    unsigned *fail = flags & Z_WAITOK ? &fail_slot_wait : &fail_slot_nowait;
    if (*fail) { (*fail)--; return NULL; }
    return calloc(1,n);
}
static void *host_data(size_t n, int flags) {
    if (flags & Z_WAITOK) assert(held == 0);
    if ((flags & Z_NOWAIT) && fail_copy) { fail_copy--; return NULL; }
    return calloc(1,n);
}
#define kalloc_type(t,n,f) ((t *)host_slots((size_t)(n)*sizeof(t),(f)))
#define HOST_FREE2(t,p) free(p)
#define HOST_FREE3(t,n,p) free(p)
#define HOST_SELECT(_1,_2,_3,NAME,...) NAME
#define kfree_type(...) HOST_SELECT(__VA_ARGS__,HOST_FREE3,HOST_FREE2)(__VA_ARGS__)
#define kalloc_data(n,f) host_data(n,f)
#define kfree_data(p,n) free(p)
static int kmem_alloc(void *map, vm_offset_t *addr, size_t size, int flags, int tag) {
    (void)map; (void)tag;
    if (!(flags & KMA_NOPAGEWAIT)) assert(held == 0);
    if (fail_page) { fail_page--; return KERN_RESOURCE_SHORTAGE; }
    void *p = calloc(1,size);
    if (!p) return KERN_RESOURCE_SHORTAGE;
    allocated_pages++; *addr = (vm_offset_t)p; return KERN_SUCCESS;
}
static void kmem_free(void *map, vm_offset_t addr, size_t size) {
    (void)map; (void)size; assert(allocated_pages > 0); allocated_pages--; free((void *)addr);
}
static int host_printf(const char *format, ...) __attribute__((format(printf,1,2)));
static int host_printf(const char *format, ...) {
    assert(held == 0);
    va_list ap; va_start(ap,format);
    size_t len = strlen(log_text);
    int result = vsnprintf(log_text + len, sizeof(log_text) - len, format, ap);
    va_end(ap); log_lines++; return result;
}
#define printf host_printf
/* ACTUAL RAMFS DECLARATIONS */
#define RAMFS_NODE(vp) ((vp)->node)
static void ramfs_touch(struct ramfs_node *node, bool modified) { (void)node; (void)modified; }
static void ramfs_sync_mapping(vnode_t vp, struct ramfs_node *n) { (void)vp; (void)n; }
static user_ssize_t uio_resid(struct uio *u) { return u->resid; }
static off_t uio_offset(struct uio *u) { return u->offset; }
static void uio_setoffset(struct uio *u, off_t o) { u->offset = o; }
static void uio_setresid(struct uio *u, user_ssize_t r) { u->resid = r; }
static int uiomove(void *p, int n, struct uio *u) {
    assert(held == 0 && n >= 0 && n <= u->resid);
    if (u->input) memcpy(p,u->buf+u->cursor,(size_t)n);
    else memcpy(u->buf+u->cursor,p,(size_t)n);
    u->cursor += (size_t)n; u->offset += n; u->resid -= n; return 0;
}
static int ubc_setsize(vnode_t vp, off_t size) { assert(held == 0); vp->size = size; return 0; }
static int ubc_pages_resident(vnode_t vp) { (void)vp; return 0; }
static void ramfs_refresh_cache(vnode_t v, struct ramfs_node *n, uint64_t a, uint64_t b) {
    (void)v; (void)n; (void)a; (void)b; assert(!"UBC is not emulated");
}
static void vnode_recycle(vnode_t vp) { vp->recycled++; }
static void vnode_clearfsnode(vnode_t vp) { vp->cleared++; vp->node = NULL; }
/* ACTUAL RAMFS FUNCTIONS */
#undef printf
static unsigned checks;
#define CHECK(c) do { checks++; if (!(c)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c); return 1; } } while (0)
static void reset_log(void) { log_text[0] = 0; log_lines = 0; }
static void init_mount(struct ramfs_mount *rm, bool trace, uint64_t limit) {
    memset(rm,0,sizeof(*rm)); rm->rm_trace = trace; rm->rm_max_pages = limit;
    rm->rm_hash = calloc(RAMFS_HASH_SIZE,sizeof(*rm->rm_hash));
}
static struct ramfs_node *new_node(struct ramfs_mount *rm, uint64_t ino) {
    struct ramfs_node *node = calloc(1,sizeof(*node)); node->rn_mount = rm;
    node->rn_ino = ino; node->rn_nlink = 1; node->rn_type = VREG;
    TAILQ_INIT(&node->rn_entries); TAILQ_INIT(&node->rn_xattrs);
    host_lock(&rm->rm_lock); ramfs_hash_insert(rm,node); rm->rm_nodes++; host_unlock(&rm->rm_lock);
    return node;
}
static int write_bytes(vnode_t vp, char *bytes, size_t size, off_t offset, struct uio *u) {
    *u = (struct uio){.offset=offset,.resid=(user_ssize_t)size,.buf=bytes,.input=true};
    struct vnop_write_args ap = {.a_vp=vp,.a_uio=u}; return ramfs_write(&ap);
}
static void destroy_node(vnode_t vp) {
    vp->node->rn_nlink = 0;
    struct vnop_inactive_args inactive = {.a_vp=vp}; ramfs_inactive(&inactive);
    struct vnop_reclaim_args reclaim = {.a_vp=vp}; ramfs_reclaim(&reclaim);
}
/* Additional host fixtures for the actual kernel ownership snapshot. */
static void add_name(struct ramfs_node *dir, struct ramfs_node *node, const char *name) {
    struct ramfs_dirent *de=calloc(1,sizeof(*de));
    de->rd_node=node;de->rd_name=strdup(name);de->rd_namelen=(uint32_t)strlen(name);
    TAILQ_INSERT_TAIL(&dir->rn_entries,de,rd_link);
}
static void clear_names(struct ramfs_node *dir) {
    struct ramfs_dirent *de;
    while((de=TAILQ_FIRST(&dir->rn_entries))) {
        TAILQ_REMOVE(&dir->rn_entries,de,rd_link);free(de->rd_name);free(de);
    }
}
static int owners_checks(void) {
    char short_name[4]={0}, untouched='x';
    ramfs_pressure_name(short_name,sizeof(short_name),"abcdef",6);
    CHECK(!strcmp(short_name,"abc"));
    ramfs_pressure_name(&untouched,0,NULL,100);CHECK(untouched=='x');
    ramfs_pressure_name(short_name,sizeof(short_name),"\n\"\\",3);CHECK(!strcmp(short_name,"???"));
    CHECK(sizeof(struct ramfs_pressure_event)<=1024);
    struct ramfs_mount rm;struct uio u;
    char *payload=calloc(6,PAGE_SIZE);memset(payload,0x5a,6*PAGE_SIZE);
    init_mount(&rm,true,16);rm.rm_trace_top=true;
    struct host_vnode root={.node=new_node(&rm,2)},dir={.node=new_node(&rm,10)};
    root.node->rn_type=dir.node->rn_type=VDIR;rm.rm_root=root.node;
    root.node->rn_parent=root.node;dir.node->rn_parent=root.node;
    add_name(root.node,dir.node,"Logs");
    const uint64_t inos[]={4,4100,5,6,7};const unsigned pages[]={5,5,3,2,1};
    struct host_vnode files[5];
    for(unsigned i=0;i<5;i++) {
        files[i]=(struct host_vnode){.node=new_node(&rm,inos[i])};
        CHECK(write_bytes(&files[i],payload,pages[i]*PAGE_SIZE,0,&u)==0);
    }
    files[0].node->rn_nlink=0;
    files[1].node->rn_uid=501;files[1].node->rn_mapped_write=true;
    add_name(dir.node,files[1].node,"bad\"\n\\name");
    char long_name[90];memset(long_name,'L',sizeof(long_name)-1);long_name[sizeof(long_name)-1]=0;
    add_name(dir.node,files[2].node,long_name);add_name(root.node,files[3].node,"cache.db");
    add_name(dir.node,files[4].node,"small");
    struct host_vnode probe={.node=new_node(&rm,8)};
    reset_log();CHECK(write_bytes(&probe,payload,15400,0,&u)==ENOSPC);
    struct ramfs_pressure_event event=rm.rm_event;event.valid=true;
    CHECK(event.top_enabled&&event.top_count==4&&event.hash_pages==16&&event.unlinked_pages==5);
    CHECK(event.top[0].ino==4&&event.top[1].ino==4100&&event.top[2].ino==5&&event.top[3].ino==6);
    CHECK(event.top[0].pages==5&&!event.top[0].nlink&&!event.top[0].parent&&!event.top[0].name[0]);
    CHECK(event.top[1].uid==501&&event.top[1].mapped_write&&event.top[1].size==5*PAGE_SIZE);
    CHECK(!strcmp(event.top[1].name,"bad???name")&&!strcmp(event.top[1].directory,"Logs"));
    CHECK(event.top[1].parent==10&&!event.top[1].name_truncated);
    CHECK(event.top[2].name_truncated&&strlen(event.top[2].name)==47&&!event.top[2].dir_truncated);
    CHECK(event.top[3].parent==2&&!strcmp(event.top[3].directory,"/"));
    CHECK(!event.names_limited&&event.name_entries<=65536&&log_lines==8&&strstr(log_text,"WHYRED RAMFS TOP"));
    clear_names(root.node);clear_names(dir.node);
    for(unsigned i=0;i<5;i++)destroy_node(&files[i]);
    destroy_node(&probe);destroy_node(&dir);destroy_node(&root);free(rm.rm_hash);
    CHECK(!allocated_pages&&!ramfs_pages_in_use);
    reset_log();ramfs_pressure_emit(&event);
    CHECK(log_lines==8&&strstr(log_text,"bad???name")&&strstr(log_text,"pages=5"));

    /* Bounded name traversal, even with an oversized hard-link-heavy directory. */
    init_mount(&rm,true,1);rm.rm_trace_top=true;
    root=(struct host_vnode){.node=new_node(&rm,2)};root.node->rn_type=VDIR;rm.rm_root=root.node;
    root.node->rn_parent=root.node;
    struct host_vnode target={.node=new_node(&rm,9000)},other={.node=new_node(&rm,3)};
    probe=(struct host_vnode){.node=new_node(&rm,4)};
    CHECK(write_bytes(&target,payload,200,0,&u)==0);
    for(unsigned i=0;i<65537;i++)add_name(root.node,other.node,"skip");
    add_name(root.node,target.node,"beyond-budget");
    reset_log();CHECK(write_bytes(&probe,payload,200,0,&u)==ENOSPC);
    CHECK(rm.rm_event.top_count==1&&rm.rm_event.top[0].ino==9000&&rm.rm_event.names_limited);
    CHECK(rm.rm_event.name_entries==65536&&!rm.rm_event.top[0].parent&&!rm.rm_event.top[0].name[0]);
    clear_names(root.node);destroy_node(&target);destroy_node(&other);destroy_node(&probe);destroy_node(&root);free(rm.rm_hash);
    CHECK(!allocated_pages&&!ramfs_pages_in_use);

    init_mount(&rm,true,1);rm.rm_trace_top=true;probe=(struct host_vnode){.node=new_node(&rm,2)};
    fail_page=1;reset_log();CHECK(write_bytes(&probe,payload,200,0,&u)==ENOSPC);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_PAGE&&!rm.rm_event.top_enabled&&log_lines==3);
    rm.rm_trace=false;rm.rm_failure_seq=0;reset_log();fail_page=1;
    CHECK(write_bytes(&probe,payload,200,0,&u)==ENOSPC&&rm.rm_failure_seq==0&&!log_lines);
    destroy_node(&probe);free(rm.rm_hash);free(payload);
    CHECK(!allocated_pages&&!ramfs_pages_in_use&&!held);
    reset_log();return 0;
}

int main(void) {
    struct ramfs_mount rm; struct uio u;
    CHECK(owners_checks()==0);
    char *data = malloc(PAGE_SIZE*4); memset(data,0x5a,PAGE_SIZE*4);
    init_mount(&rm,false,1);
    struct host_vnode a = {.node=new_node(&rm,10)}, b = {.node=new_node(&rm,11)};
    CHECK(write_bytes(&a,data,400,0,&u)==0 && u.resid==0);
    CHECK(write_bytes(&b,data,400,0,&u)==ENOSPC && u.resid==400 && u.offset==0);
    CHECK(rm.rm_failure_seq==0 && rm.rm_reports==0 && log_lines==0 && rm.rm_pages==1);
    destroy_node(&a); destroy_node(&b); free(rm.rm_hash);
    CHECK(allocated_pages==0 && ramfs_pages_in_use==0);

    init_mount(&rm,true,3); a=(struct host_vnode){.node=new_node(&rm,20)};
    b=(struct host_vnode){.node=new_node(&rm,21)};
    CHECK(write_bytes(&a,data,PAGE_SIZE*2,0,&u)==0);
    CHECK(rm.rm_pages==2 && rm.rm_added==2 && rm.rm_peak==2);
    CHECK(write_bytes(&b,data,PAGE_SIZE*2,0,&u)==0); /* short success at quota */
    CHECK(u.resid==PAGE_SIZE && u.offset==PAGE_SIZE && b.node->rn_size==PAGE_SIZE);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_QUOTA && rm.rm_event.pages==3);
    CHECK(rm.rm_event.hash_pages==3 && rm.rm_event.limit==3 && rm.rm_failures[0]==1);
    CHECK(rm.rm_event.vm_free==123 && rm.rm_event.vm_wired==456);
    CHECK(log_lines==3 && strchr(log_text,'\n')!=NULL && held==0);
    reset_log(); CHECK(write_bytes(&a,data,500,0,&u)==0 && rm.rm_pages==3 && log_lines==0);
    host_lock(&rm.rm_lock); ramfs_resize(a.node,PAGE_SIZE+3); ramfs_resize(a.node,PAGE_SIZE+8);
    unsigned char tail[8]; ramfs_fetch(a.node,PAGE_SIZE,tail,sizeof(tail)); host_unlock(&rm.rm_lock);
    CHECK(tail[0]==0x5a && tail[2]==0x5a && tail[3]==0 && tail[7]==0);
    host_lock(&rm.rm_lock); ramfs_resize(a.node,0); host_unlock(&rm.rm_lock);
    CHECK(rm.rm_pages==1 && rm.rm_freed==2 && a.node->rn_nalloc==0);
    b.node->rn_nlink=0; /* open-unlinked file remains readable until vnode reclaim */
    host_lock(&rm.rm_lock); ramfs_fetch(b.node,0,tail,sizeof(tail)); host_unlock(&rm.rm_lock);
    CHECK(tail[0]==0x5a && rm.rm_pages==1);
    fail_page=1; CHECK(write_bytes(&a,data,395,0,&u)==ENOSPC);
    CHECK(u.resid==395 && u.offset==0 && a.node->rn_size==0 && rm.rm_pages==1);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_PAGE && rm.rm_event.unlinked_pages==1 && rm.rm_event.unlinked_nodes==1);
    CHECK(strstr(log_text,"kr=3")!=NULL && rm.rm_event.hash_pages==rm.rm_pages);
    reset_log(); destroy_node(&b);
    CHECK(b.recycled==1 && b.cleared==1 && rm.rm_pages==0 && rm.rm_reclaimed==1);
    CHECK(rm.rm_added==rm.rm_freed && ramfs_pages_in_use==0);
    CHECK(write_bytes(&a,data,550,0,&u)==0 && rm.rm_pages==1);
    struct vnop_reclaim_args reclaim={.a_vp=&a};
    struct ramfs_node *named=a.node; ramfs_reclaim(&reclaim);
    CHECK(rm.rm_pages==1 && named->rn_hashed && named->rn_nlink==1 && rm.rm_reclaimed==1);
    a.node=named; destroy_node(&a); CHECK(rm.rm_pages==0 && rm.rm_nodes==0);
    free(rm.rm_hash);

    init_mount(&rm,true,3); a=(struct host_vnode){.node=new_node(&rm,30)};
    fail_slot_wait=1; fail_slot_nowait=1; reset_log();
    CHECK(write_bytes(&a,data,400,0,&u)==ENOSPC);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_SLOTS && rm.rm_event.pages==0 && rm.rm_event.length==4*sizeof(void *));
    CHECK(u.resid==400 && u.offset==0 && a.node->rn_nslots==0 && allocated_pages==0);
    CHECK(write_bytes(&a,data,400,0,&u)==0 && rm.rm_pages==1);
    struct ramfs_xattr *xa=calloc(1,sizeof(*xa));
    xa->rx_name=strdup("test"); xa->rx_namelen=4; xa->rx_size=16; xa->rx_data=calloc(1,16);
    TAILQ_INSERT_TAIL(&a.node->rn_xattrs,xa,rx_link); rm.rm_xattr_bytes=16;
    char attr[32]; memset(attr,0x7e,sizeof(attr)); size_t attrsize=0;
    u=(struct uio){.resid=sizeof(attr),.buf=attr};
    struct vnop_getxattr_args get={.a_vp=&a,.a_name="test",.a_uio=&u,.a_size=&attrsize};
    fail_copy=1; reset_log(); CHECK(ramfs_getxattr(&get)==ENOMEM);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_GETXATTR && u.resid==sizeof(attr) && attr[0]==0x7e);
    CHECK(ramfs_getxattr(&get)==0 && u.resid==sizeof(attr)-16 && attr[0]==0);
    u=(struct uio){.resid=sizeof(attr),.buf=attr};
    struct vnop_listxattr_args list={.a_vp=&a,.a_uio=&u,.a_size=&attrsize};
    fail_copy=1; CHECK(ramfs_listxattr(&list)==ENOMEM);
    CHECK(rm.rm_event.reason==RAMFS_PRESSURE_LISTXATTR && rm.rm_xattr_bytes==16);
    CHECK(ramfs_listxattr(&list)==0 && memcmp(attr,"test",5)==0);
    destroy_node(&a); CHECK(rm.rm_pages==0 && rm.rm_xattr_bytes==0 && rm.rm_nodes==0);
    free(rm.rm_hash);

    init_mount(&rm,true,3); a=(struct host_vnode){.node=new_node(&rm,40)};
    ramfs_reserve_slots(&rm,a.node,0);
    for (unsigned i=0;i<1100000;i++) {
        void *spare=NULL; struct ramfs_pressure_event e={0};
        host_lock(&rm.rm_lock);
        int error=ramfs_store(a.node,0,data,400,&spare);
        ramfs_pressure_take_locked(&rm,&e); host_unlock(&rm.rm_lock);
        CHECK(error==ENOSPC && rm.rm_pages==0 && spare==NULL);
        if (e.valid) {
            CHECK(e.seq<=16 || (e.seq & (e.seq-1))==0);
            reset_log(); ramfs_pressure_emit(&e); CHECK(log_lines==3);
        }
    }
    CHECK(rm.rm_failure_seq==1100000 && rm.rm_failures[RAMFS_PRESSURE_PAGE]==1100000 && rm.rm_reports==32);
    destroy_node(&a); free(rm.rm_hash);
    init_mount(&rm,true,3);
    for (unsigned i=0;i<10000;i++) {
        a=(struct host_vnode){.node=new_node(&rm,100+i)};
        CHECK(write_bytes(&a,data,500,0,&u)==0); destroy_node(&a);
        CHECK(rm.rm_pages==0 && rm.rm_nodes==0 && ramfs_pages_in_use==0 && allocated_pages==0);
    }
    CHECK(rm.rm_added==10000 && rm.rm_freed==10000 && rm.rm_reclaimed==10000);
    free(rm.rm_hash); free(data);
    CHECK(held==0 && allocated_pages==0 && ramfs_pages_in_use==0);
    printf("PASS: %u actual RAMFS pressure/write/reclaim checks (host adapters)\n",checks);
    return 0;
}
