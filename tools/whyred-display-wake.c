/* Explicit single wake request for the installed whyred97 development kernel.
 * This uses SpringBoard's existing undim service; it does not implement keys,
 * draw pixels, change renderer flags, unlock the device or write partitions. */
#include <TargetConditionals.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int main(int argc, char **argv)
{
    if (argc != 2 || strcmp(argv[1], "wake")) {
        fputs("usage: whyred-display-wake wake\n", stderr); return 2;
    }
#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    puts("WHYRED WAKE: requires the physical iOS target"); return 1;
#else
    char uuid[80] = {0}; size_t size = sizeof(uuid);
    if (sysctlbyname("kern.uuid", uuid, &size, NULL, 0) ||
        strcmp(uuid, "1444AADE-E57B-3F55-88A9-866FFA6086A0")) {
        puts("WHYRED WAKE: incompatible kernel; no request sent"); return 1;
    }
    void *library = dlopen("/System/Library/PrivateFrameworks/SpringBoardServices.framework/SpringBoardServices", RTLD_NOW | RTLD_LOCAL);
    /* Exact20F66 no-argument wrapper obtains the server port, supplies w1=0,
     * and tail-calls the MIG client. Its return is the request status only. */
    int32_t (*undim)(void) = library ? dlsym(library, "SBSUndimScreen") : NULL;
    if (!undim) {
        puts("WHYRED WAKE: service API absent; no request sent");
        if (library) dlclose(library);
        return 1;
    }
    puts("WHYRED WAKE: sending one SpringBoard undim request"); fflush(stdout);
    int32_t status = undim();
    printf("WHYRED WAKE: request_status=0x%x visible_output_unverified=1\n", (unsigned)status);
    dlclose(library);
    return status ? 1 : 0;
#endif
}
