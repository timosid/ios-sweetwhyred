/* Explicit single wake request for the installed whyred97 development kernel.
 * Uses the existing SpringBoard undim or BackBoard unblank service. No keys,
 * pixel drawing, renderer flags, device unlocking or partition writes. */
#include <TargetConditionals.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_time.h>
#include <time.h>
#include <errno.h>

int main(int argc, char **argv)
{
    if (argc != 2 || (strcmp(argv[1], "wake") && strcmp(argv[1], "unblank") && strcmp(argv[1], "power-key"))) {
        fputs("usage: whyred-display-wake {wake|unblank|power-key}\n", stderr); return 2;
    }
#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    puts("WHYRED WAKE: requires the physical iOS target"); return 1;
#else
    char uuid[80] = {0}; size_t size = sizeof(uuid);
    if (sysctlbyname("kern.uuid", uuid, &size, NULL, 0) ||
        strcmp(uuid, "1444AADE-E57B-3F55-88A9-866FFA6086A0")) {
        puts("WHYRED WAKE: incompatible kernel; no request sent"); return 1;
    }
    /* Dispatch API and entitlement reference: Apple's IOHIDFamily tools/
     * IOHIDEventSystemMonitor.c and IOHIDEventSystemMonitor-Entitlements.plist.
     * Existing service sender only; this does not claim physical key support. */
    if (!strcmp(argv[1], "power-key")) {
        void *library = dlopen("/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit", RTLD_NOW | RTLD_LOCAL);
        CFTypeRef (*createClient)(CFAllocatorRef) = library ? dlsym(library, "IOHIDEventSystemClientCreate") : NULL;
        CFTypeRef (*createKey)(CFAllocatorRef, uint64_t, uint32_t, uint32_t, int, uint32_t) = library ? dlsym(library, "IOHIDEventCreateKeyboardEvent") : NULL;
        void (*dispatch)(CFTypeRef, CFTypeRef) = library ? dlsym(library, "IOHIDEventSystemClientDispatchEvent") : NULL;
        void (*setSender)(CFTypeRef, uint64_t) = library ? dlsym(library, "IOHIDEventSetSenderID") : NULL;
        CFMutableDictionaryRef (*matching)(const char *) = library ? dlsym(library, "IOServiceMatching") : NULL;
        uint32_t (*getService)(uint32_t, CFDictionaryRef) = library ? dlsym(library, "IOServiceGetMatchingService") : NULL;
        int (*getID)(uint32_t, uint64_t *) = library ? dlsym(library, "IORegistryEntryGetRegistryEntryID") : NULL;
        int (*releaseService)(uint32_t) = library ? dlsym(library, "IOObjectRelease") : NULL;
        if (!createClient || !createKey || !dispatch || !setSender || !matching || !getService || !getID || !releaseService) {
            puts("WHYRED WAKE: HID API absent; no event sent");
            if (library) dlclose(library);
            return 1;
        }
        uint32_t service = getService(0, matching("SweetHIDButtons"));
        uint64_t sender = 0;
        int error = service ? getID(service, &sender) : -1;
        if (service) releaseService(service);
        mach_timebase_info_data_t tb = {0};
        if (error || !sender || mach_timebase_info(&tb) || !tb.numer || !tb.denom) {
            puts("WHYRED WAKE: no existing buttons service/timebase; no event sent"); dlclose(library); return 1;
        }
        CFTypeRef client = createClient(kCFAllocatorDefault);
        uint64_t now = mach_absolute_time();
        CFTypeRef down = client ? createKey(kCFAllocatorDefault, now, 0x0c, 0x30, 1, 0) : NULL;
        CFTypeRef up = down ? createKey(kCFAllocatorDefault, now + 100000000ULL * tb.denom / tb.numer, 0x0c, 0x30, 0, 0) : NULL;
        if (!client || !down || !up) {
            puts("WHYRED WAKE: HID allocation/client refused; no event sent");
            if (up) CFRelease(up);
            if (down) CFRelease(down);
            if (client) CFRelease(client);
            dlclose(library); return 1;
        }
        setSender(down, sender); setSender(up, sender);
        puts("WHYRED WAKE: sending one diagnostic power-key down/up pair"); fflush(stdout);
        dispatch(client, down);
        struct timespec delay = {0, 100000000};
        while (nanosleep(&delay, &delay) && errno == EINTR) {}
        dispatch(client, up);
        puts("WHYRED WAKE: HID pair dispatched delivery_and_visible_output_unverified=1");
        CFRelease(up); CFRelease(down); CFRelease(client); dlclose(library); return 0;
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
