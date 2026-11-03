/* Explicit single wake request for the installed whyred97 development kernel.
 * Uses the existing SpringBoard undim or BackBoard unblank service. No keys,
 * pixel drawing, renderer flags, device unlocking or partition writes. */
#include <TargetConditionals.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int main(int argc, char **argv)
{
    if (argc != 2 || (strcmp(argv[1], "wake") && strcmp(argv[1], "unblank"))) {
        fputs("usage: whyred-display-wake {wake|unblank}\n", stderr); return 2;
    }
#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    puts("WHYRED WAKE: requires the physical iOS target"); return 1;
#else
    char uuid[80] = {0}; size_t size = sizeof(uuid);
    if (sysctlbyname("kern.uuid", uuid, &size, NULL, 0) ||
        strcmp(uuid, "1444AADE-E57B-3F55-88A9-866FFA6086A0")) {
        puts("WHYRED WAKE: incompatible kernel; no request sent"); return 1;
    }
    if (!strcmp(argv[1], "unblank")) {
        void *library = dlopen("/System/Library/PrivateFrameworks/BackBoardServices.framework/BackBoardServices", RTLD_NOW | RTLD_LOCAL);
        /* Exact20F66 wrapper forwards its boolean argument to the display MIG
         * client. Do not interpret an unspecified C return as wake success. */
        void (*unblank)(int) = library ? dlsym(library, "BKSDisplayServicesSetScreenBlanked") : NULL;
        if (!unblank) {
            puts("WHYRED WAKE: display API absent; no request sent");
            if (library) dlclose(library);
            return 1;
        }
        puts("WHYRED WAKE: sending one BackBoard unblank request"); fflush(stdout);
        unblank(0);
        puts("WHYRED WAKE: unblank request returned visible_output_unverified=1");
        dlclose(library);
        return 0;
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
