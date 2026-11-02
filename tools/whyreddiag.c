/* Bounded diagnostics for a developer-owned whyred device.
 * Sampling is read-only; the explicit xattrs command uses one temporary file. */
#include <mach/mach.h>
#include <mach/thread_info.h>
#include <mach/vm_statistics.h>
#if defined(__aarch64__)
#include <mach/arm/thread_status.h>
#else
#include <mach/i386/thread_status.h>
#endif
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/xattr.h>
#include <sys/wait.h>
#include <errno.h>
#include <grp.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int memory_stats(void) {
    host_t host = mach_host_self();
    vm_statistics64_data_t vm;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_size_t page_size = 0;
    kern_return_t kr = host_page_size(host, &page_size);
    if (kr == KERN_SUCCESS) kr = host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm, &count);
    mach_port_deallocate(mach_task_self(), host);
    if (kr != KERN_SUCCESS) { fprintf(stderr, "host VM statistics: %d %s\n", kr, mach_error_string(kr)); return 1; }
    printf("VM page_size=%lu free=%u active=%u inactive=%u wired=%u compressor=%u internal=%u external=%u pageins=%llu pageouts=%llu swapins=%llu swapouts=%llu\n",
        (unsigned long)page_size, vm.free_count, vm.active_count, vm.inactive_count, vm.wire_count,
        vm.compressor_page_count, vm.internal_page_count, vm.external_page_count,
        (unsigned long long)vm.pageins, (unsigned long long)vm.pageouts,
        (unsigned long long)vm.swapins, (unsigned long long)vm.swapouts);
    return 0;
}

static int sample(pid_t pid) {
    task_t task = MACH_PORT_NULL;
    kern_return_t kr;
    if (pid == getpid()) task = mach_task_self();
    else {
        kr = task_for_pid(mach_task_self(), pid, &task);
        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "task_for_pid(%d): %d %s\n", pid, kr, mach_error_string(kr));
            return 1;
        }
    }
    thread_act_array_t threads = NULL;
    mach_msg_type_number_t count = 0;
    kr = task_threads(task, &threads, &count);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "task_threads: %d %s\n", kr, mach_error_string(kr));
        if (pid != getpid()) mach_port_deallocate(mach_task_self(), task);
        return 1;
    }
    printf("pid=%d threads=%u live_sample=1 suspended=0\n", pid, count);
    unsigned sampled = 0;
    for (unsigned i = 0; i < count; i++) {
        if (i < 256) {
            thread_basic_info_data_t info;
            mach_msg_type_number_t n = THREAD_BASIC_INFO_COUNT;
            kr = thread_info(threads[i], THREAD_BASIC_INFO, (thread_info_t)&info, &n);
            printf("thread=%u info_kr=%d", i, kr);
            if (kr == KERN_SUCCESS) printf(" state=%d flags=%d suspend=%d user_us=%lld sys_us=%lld",
                info.run_state, info.flags, info.suspend_count,
                (long long)info.user_time.seconds * 1000000 + info.user_time.microseconds,
                (long long)info.system_time.seconds * 1000000 + info.system_time.microseconds);
            uint64_t pc = 0, sp = 0, fp = 0, lr = 0;
#if defined(__aarch64__)
            arm_thread_state64_t state;
            n = ARM_THREAD_STATE64_COUNT;
            kr = thread_get_state(threads[i], ARM_THREAD_STATE64, (thread_state_t)&state, &n);
            if (kr == KERN_SUCCESS) {
                pc = arm_thread_state64_get_pc(state); sp = arm_thread_state64_get_sp(state);
                fp = arm_thread_state64_get_fp(state); lr = arm_thread_state64_get_lr(state);
            }
#else
            x86_thread_state64_t state;
            n = x86_THREAD_STATE64_COUNT;
            kr = thread_get_state(threads[i], x86_THREAD_STATE64, (thread_state_t)&state, &n);
            if (kr == KERN_SUCCESS) { pc = state.__rip; sp = state.__rsp; fp = state.__rbp; }
#endif
            printf(" state_kr=%d pc=0x%" PRIx64 " sp=0x%" PRIx64 " fp=0x%" PRIx64 " lr=0x%" PRIx64, kr, pc, sp, fp, lr);
            if (kr == KERN_SUCCESS) {
                sampled++;
                /* Unsuspended frames can change; bounds prevent loops and runaway reads. */
                uint64_t start = sp;
                for (unsigned j = 0; j < 24 && fp >= start && fp - start < 8 * 1024 * 1024 && !(fp & 7); j++) {
                    uint64_t frame[2]; vm_size_t got = 0;
                    kr = vm_read_overwrite(task, (vm_address_t)fp, sizeof(frame), (vm_address_t)frame, &got);
                    if (kr != KERN_SUCCESS || got != sizeof(frame)) break;
                    printf(" bt%u=0x%" PRIx64, j, frame[1]);
                    if (frame[0] <= fp) break;
                    fp = frame[0];
                }
            }
            putchar('\n');
        }
        mach_port_deallocate(mach_task_self(), threads[i]);
    }
    if (threads) vm_deallocate(mach_task_self(), (vm_address_t)threads, count * sizeof(*threads));
    if (pid != getpid()) mach_port_deallocate(mach_task_self(), task);
    return sampled ? 0 : 1;
}

static double now(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) { perror("clock_gettime"); exit(1); }
    return t.tv_sec + t.tv_nsec / 1e9;
}

static int kernel_info(void) {
    char uuid[128] = {0}; size_t size = sizeof(uuid) - 1;
    if (sysctlbyname("kern.uuid", uuid, &size, NULL, 0)) { perror("kern.uuid"); return 1; }
    uuid[sizeof(uuid) - 1] = 0;
    printf("WHYRED DIAG: kernel_uuid=%s\n", uuid);
    return 0;
}

static int xattr_probe(const char *directory) {
    char path[1024];
    if (directory[0] != '/' || snprintf(path, sizeof(path), "%s/whyred-xattrs.XXXXXX", directory) >= (int)sizeof(path)) return 2;
    int fd = mkstemp(path);
    if (fd < 0) { perror("xattrs mkstemp"); return 1; }
    int result = 1; unsigned checks = 0;
    const char *name = "user.whyred.snapshot";
    unsigned char bytes[16384], got[16384]; char names[4096];
    const size_t sizes[] = {0, 1, 68, 511, 4096, 16384, 68, 1, 0};
#define XCHECK(condition) do { checks++; if (!(condition)) { \
    fprintf(stderr, "WHYRED DIAG: xattrs check=%u failed line=%d errno=%d\n", checks, __LINE__, errno); goto cleanup; } } while (0)
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        size_t size = sizes[i];
        for (size_t j = 0; j < size; j++) bytes[j] = (unsigned char)(j * 17 + i);
        XCHECK(fsetxattr(fd, name, bytes, size, 0, 0) == 0);
        XCHECK(fgetxattr(fd, name, NULL, 0, 0, 0) == (ssize_t)size);
        memset(got, 0x7e, sizeof(got));
        if (size > 1) {
            errno = 0;
            XCHECK(fgetxattr(fd, name, got, size - 1, 0, 0) == -1 && errno == ERANGE);
            XCHECK(got[0] == 0x7e);
        }
        XCHECK(fgetxattr(fd, name, got, sizeof(got), 0, 0) == (ssize_t)size);
        XCHECK(memcmp(bytes, got, size) == 0);
        ssize_t count = flistxattr(fd, names, sizeof(names), 0);
        XCHECK(count > 0 && count <= (ssize_t)sizeof(names));
        int found = 0;
        for (size_t at = 0; at < (size_t)count;) {
            size_t length = strnlen(names + at, (size_t)count - at);
            XCHECK(length < (size_t)count - at);
            if (!strcmp(names + at, name)) found = 1;
            at += length + 1;
        }
        XCHECK(found);
    }
    XCHECK(fremovexattr(fd, name, 0) == 0);
    errno = 0;
    XCHECK(fgetxattr(fd, name, got, sizeof(got), 0, 0) == -1 && errno == ENOATTR);
    printf("WHYRED DIAG: xattrs PASS checks=%u max_bytes=%zu temporary_file=1\n", checks, sizeof(bytes));
    result = 0;
cleanup:
    if (close(fd)) { perror("xattrs close"); result = 1; }
    if (unlink(path)) { perror("xattrs cleanup"); result = 1; }
#undef XCHECK
    return result;
}

static int run_mobile(char **args) {
    if (getuid() || args[0][0] != '/') { fprintf(stderr, "run-mobile requires root and an absolute executable\n"); return 2; }
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }
    if (!child) {
        if (setpgid(0, 0) || initgroups("mobile", 501) || setgid(501) || setuid(501)) { perror("mobile credentials"); _exit(126); }
        struct rlimit no_core = {0, 0};
        if (setrlimit(RLIMIT_CORE, &no_core)) { perror("setrlimit"); _exit(126); }
        if (setenv("HOME", "/var/mobile", 1) || setenv("USER", "mobile", 1) || setenv("LOGNAME", "mobile", 1) || chdir("/var/mobile")) { perror("mobile environment"); _exit(126); }
        execv(args[0], args); perror("execv"); _exit(127);
    }
    double deadline = now() + 30;
    int timed_out = 0, killed = 0, status;
    for (;;) {
        pid_t got = waitpid(child, &status, WNOHANG);
        if (got == child) break;
        if (got < 0 && errno != EINTR) { perror("waitpid"); return 1; }
        double t = now();
        if (!timed_out && t >= deadline) { kill(-child, SIGTERM); timed_out = 1; }
        if (timed_out && !killed && t >= deadline + 2) { kill(-child, SIGKILL); killed = 1; }
        struct timespec delay = {0, 100000000}; nanosleep(&delay, NULL);
    }
    fprintf(stderr, "WHYRED DIAG: child_status=0x%x exit=%d signal=%d timeout=%d\n", status,
        WIFEXITED(status) ? WEXITSTATUS(status) : -1, WIFSIGNALED(status) ? WTERMSIG(status) : 0, timed_out);
    return timed_out ? 124 : WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "kernel")) return kernel_info();
    if (argc == 3 && !strcmp(argv[1], "xattrs")) return xattr_probe(argv[2]);
    if (argc == 2 && !strcmp(argv[1], "memory")) return memory_stats();
    if (argc == 2 && !strcmp(argv[1], "selftest")) return memory_stats() || sample(getpid());
    if (argc == 3 && !strcmp(argv[1], "threads")) {
        char *end; errno = 0; long pid = strtol(argv[2], &end, 10);
        if (errno || *end || pid <= 0 || pid > 0x7fffffff) return 2;
        return sample((pid_t)pid);
    }
    if (argc >= 3 && !strcmp(argv[1], "run-mobile")) return run_mobile(argv + 2);
    fprintf(stderr, "usage: whyreddiag selftest | memory | kernel | xattrs /temporary-directory | threads PID | run-mobile /executable [args]\n"); return 2;
}
