/* Explicit developer diagnostic: render only a 16x16 process-local bitmap.
 * No windows, device state configuration, files or daemon changes. */
#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>
#import <QuartzCore/QuartzCore.h>
#if __has_include(<IOSurface/IOSurface.h>) && !defined(WHYRED_PROBE_IOSURFACE_DECLS)
#import <IOSurface/IOSurface.h>
#else
/* The iPhoneOS SDK ships the dylib without its C headers. Keep this small
 * declaration set aligned with the ABI of the tested IOSurface entry points. */
typedef struct __IOSurface *IOSurfaceRef;
extern const CFStringRef kIOSurfaceWidth, kIOSurfaceHeight;
extern const CFStringRef kIOSurfaceBytesPerElement, kIOSurfaceBytesPerRow;
extern const CFStringRef kIOSurfaceAllocSize, kIOSurfacePixelFormat;
extern IOSurfaceRef IOSurfaceCreate(CFDictionaryRef);
extern kern_return_t IOSurfaceLock(IOSurfaceRef, uint32_t, uint32_t *);
extern kern_return_t IOSurfaceUnlock(IOSurfaceRef, uint32_t, uint32_t *);
extern size_t IOSurfaceGetWidth(IOSurfaceRef), IOSurfaceGetHeight(IOSurfaceRef);
extern size_t IOSurfaceGetBytesPerRow(IOSurfaceRef);
extern void *IOSurfaceGetBaseAddress(IOSurfaceRef);

#endif
#import <TargetConditionals.h>
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <objc/runtime.h>

static int bitmap(void) {
    puts("WHYRED RENDER: software context begin");
    CIContext *context = [CIContext contextWithOptions:@{
        kCIContextUseSoftwareRenderer: @YES,
        kCIContextCacheIntermediates: @NO
    }];
    printf("WHYRED RENDER: software context=%s\n", context ? "created" : "nil");
    if (!context) return 1;
    CIColor *red = [CIColor colorWithRed:1 green:0 blue:0 alpha:1];
    CGRect bounds = CGRectMake(0, 0, 16, 16);
    CIImage *image = [[CIImage imageWithColor:red] imageByCroppingToRect:bounds];
    unsigned char pixels[16 * 16 * 4];
    memset(pixels, 0x6d, sizeof(pixels));
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    if (!color) return 1;
    puts("WHYRED RENDER: software render begin");
    [context render:image toBitmap:pixels rowBytes:16 * 4 bounds:bounds
             format:kCIFormatRGBA8 colorSpace:color];
    CGColorSpaceRelease(color);
    unsigned matches = 0;
    for (unsigned i = 0; i < 16 * 16; i++) {
        const unsigned char *p = pixels + 4 * i;
        if (p[0] == 255 && p[1] == 0 && p[2] == 0 && p[3] == 255) matches++;
    }
    printf("WHYRED RENDER: software pixels=%u/256 first=%u,%u,%u,%u files_written=0\n",
           matches, pixels[0], pixels[1], pixels[2], pixels[3]);
    return matches == 256 ? 0 : 1;
}

static int eagl(void) {
#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
    unsigned successes = 0;
    for (unsigned api = 2; api <= 3; api++) {
        printf("WHYRED RENDER: EAGL api=%u begin\n", api);
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:(EAGLRenderingAPI)api];
        BOOL current = context && [EAGLContext setCurrentContext:context];
        printf("WHYRED RENDER: EAGL api=%u context=%s current=%d\n",
               api, context ? "created" : "nil", current);
        if (current) {
            const GLubyte *renderer = glGetString(GL_RENDERER);
            GLenum error = glGetError();
            printf("WHYRED RENDER: EAGL api=%u renderer=%.*s error=0x%x\n",
                   api, 256, renderer ? (const char *)renderer : "(null)", error);
            if (renderer && error == GL_NO_ERROR) successes++;
        }
        [EAGLContext setCurrentContext:nil];
    }
    return successes ? 0 : 1;
#else
    puts("WHYRED RENDER: EAGL unavailable for this build target");
    return 2;
#endif
}

static int layer_draw(unsigned char *pixels) {
    /* Test QuartzCore's layer-to-CoreGraphics path separately from CoreImage. */
    CGColorSpaceRef color = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGContextRef bitmap = CGBitmapContextCreate(pixels, 16, 16, 8, 16 * 4, color,
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(color);
    if (!bitmap) return 1;
    puts("WHYRED RENDER: layer bitmap created");
    CALayer *root = [CALayer layer];
    root.frame = CGRectMake(0, 0, 16, 16);
    CGColorRef red = CGColorCreateSRGB(1, 0, 0, 1);
    CGColorRef green = CGColorCreateSRGB(0, 1, 0, 1);
    root.backgroundColor = red;
    CALayer *child = [CALayer layer];
    child.frame = CGRectMake(4, 4, 8, 8);
    child.backgroundColor = green;
    [root addSublayer:child];
    CGColorRelease(red); CGColorRelease(green);
    puts("WHYRED RENDER: layer render begin");
    [root renderInContext:bitmap];
    CGContextRelease(bitmap);
    printf("WHYRED RENDER: layer first=%u,%u,%u,%u center=%u,%u,%u,%u\n", pixels[0], pixels[1], pixels[2], pixels[3], pixels[544], pixels[545], pixels[546], pixels[547]);
    unsigned red_count = 0, green_count = 0;
    for (unsigned i = 0; i < 256; i++) {
        const unsigned char *p = pixels + i * 4;
        if (p[0] == 255 && p[1] == 0 && p[2] == 0 && p[3] == 255) red_count++;
        if (p[0] == 0 && p[1] == 255 && p[2] == 0 && p[3] == 255) green_count++;
    }
    printf("WHYRED RENDER: layer red=%u/192 green=%u/64 files_written=0\n",
           red_count, green_count);
    return red_count == 192 && green_count == 64 ? 0 : 1;
}

static int layer_bitmap(void) {
    unsigned char pixels[16 * 16 * 4] = {0};
    return layer_draw(pixels);
}

static int surface_bitmap(void) {
    NSDictionary *properties = @{
        (NSString *)kIOSurfaceWidth: @16, (NSString *)kIOSurfaceHeight: @16,
        (NSString *)kIOSurfaceBytesPerElement: @4,
        (NSString *)kIOSurfaceBytesPerRow: @64,
        (NSString *)kIOSurfaceAllocSize: @1024,
        (NSString *)kIOSurfacePixelFormat: @(0x52474241u)
    };
    IOSurfaceRef surface = IOSurfaceCreate((__bridge CFDictionaryRef)properties);
    printf("WHYRED RENDER: surface created=%d\n", surface != NULL);
    if (!surface) return 1;
    uint32_t seed = 0;
    kern_return_t kr = IOSurfaceLock(surface, 0, &seed);
    printf("WHYRED RENDER: surface lock=0x%x width=%zu height=%zu stride=%zu\n",
           kr, IOSurfaceGetWidth(surface), IOSurfaceGetHeight(surface),
           IOSurfaceGetBytesPerRow(surface));
    if (kr || IOSurfaceGetWidth(surface) != 16 || IOSurfaceGetHeight(surface) != 16 ||
        IOSurfaceGetBytesPerRow(surface) != 64 || !IOSurfaceGetBaseAddress(surface)) {
        if (!kr) IOSurfaceUnlock(surface, 0, NULL);
        CFRelease(surface); return 1;
    }
    unsigned char expected[1024];
    unsigned char *base = IOSurfaceGetBaseAddress(surface);
    memset(base, 0, sizeof(expected));
    int result = layer_draw(base);
    memcpy(expected, base, sizeof(expected));
    kr = IOSurfaceUnlock(surface, 0, &seed);
    if (kr) result = 1;
    kern_return_t lock_kr = IOSurfaceLock(surface, 1 /* read-only lock */, &seed);
    int preserved = !lock_kr && IOSurfaceGetBaseAddress(surface) &&
        !memcmp(expected, IOSurfaceGetBaseAddress(surface), sizeof(expected));
    printf("WHYRED RENDER: surface unlock=0x%x relock=0x%x preserved=%d display_changed=0\n",
           kr, lock_kr, preserved);
    if (!lock_kr) IOSurfaceUnlock(surface, 1 /* read-only lock */, NULL);
    CFRelease(surface);
    return result || !preserved ? 1 : 0;
}

static int renderer_methods(void) {
    const char *classes[] = {"CAWindowServer", "CAWindowServerDisplay", "CARenderer"};
    for (unsigned c = 0; c < sizeof(classes) / sizeof(classes[0]); c++) {
        Class cls = objc_getClass(classes[c]);
        printf("WHYRED RENDER: class=%s present=%d\n", classes[c], cls != Nil);
        if (!cls) continue;
        for (unsigned meta = 0; meta < 2; meta++) {
            unsigned count = 0, printed = 0;
            Method *methods = class_copyMethodList(meta ? object_getClass(cls) : cls, &count);
            for (unsigned i = 0; i < count && printed < 128; i++) {
                const char *name = sel_getName(method_getName(methods[i]));
                if (!strcasestr(name, "render") && !strcasestr(name, "display") &&
                    !strcasestr(name, "init") && !strcasestr(name, "server")) continue;
                IMP imp = method_getImplementation(methods[i]);
                Dl_info info = {0};
                if (!dladdr((const void *)imp, &info) || !info.dli_fbase) continue;
                printf("WHYRED RENDER: method=%c[%s %s] offset=0x%llx types=%s\n",
                       meta ? '+' : '-', classes[c], name,
                       (unsigned long long)((uintptr_t)imp - (uintptr_t)info.dli_fbase),
                       method_getTypeEncoding(methods[i]));
                printed++;
            }
            free(methods);
        }
    }
    return 0;
}

static int capture_display(void) {
    /* Same five-argument call used by this system's backboardd screenshot path.
     * The destination is our own surface; this does not present a frame. */
    typedef int (*RenderDisplay)(mach_port_t, CFStringRef, IOSurfaceRef, int, int);
    RenderDisplay render = (RenderDisplay)dlsym(RTLD_DEFAULT, "CARenderServerRenderDisplay");
    if (!render) { puts("WHYRED RENDER: display capture API absent"); return 1; }
    const size_t width = 1125, height = 2436, stride = 4544;
    NSDictionary *properties = @{
        (NSString *)kIOSurfaceWidth: @(width), (NSString *)kIOSurfaceHeight: @(height),
        (NSString *)kIOSurfaceBytesPerElement: @4,
        (NSString *)kIOSurfaceBytesPerRow: @(stride),
        (NSString *)kIOSurfaceAllocSize: @(stride * height),
        (NSString *)kIOSurfacePixelFormat: @(0x42475241u)
    };
    IOSurfaceRef surface = IOSurfaceCreate((__bridge CFDictionaryRef)properties);
    if (!surface) return 1;
    kern_return_t kr = IOSurfaceLock(surface, 0, NULL);
    if (kr || !IOSurfaceGetBaseAddress(surface) || IOSurfaceGetBytesPerRow(surface) != stride ||
        IOSurfaceGetWidth(surface) != width || IOSurfaceGetHeight(surface) != height) {
        if (!kr) IOSurfaceUnlock(surface, 0, NULL);
        CFRelease(surface); return 1;
    }
    memset(IOSurfaceGetBaseAddress(surface), 0, stride * height);
    puts("WHYRED RENDER: display capture LCD begin display_changed=0");
    int accepted = render(MACH_PORT_NULL, CFSTR("LCD"), surface, 0, 0);
    size_t rgb = 0, alpha = 0;
    const unsigned char *base = IOSurfaceGetBaseAddress(surface);
    for (size_t y = 0; y < height; y++) for (size_t x = 0; x < width; x++) {
        const unsigned char *p = base + y * stride + x * 4;
        rgb += (p[0] | p[1] | p[2]) != 0;
        alpha += p[3] != 0;
    }
    printf("WHYRED RENDER: display capture accepted=%d rgb=%zu alpha=%zu pixels=%zu\n",
           accepted, rgb, alpha, width * height);
    kr = IOSurfaceUnlock(surface, 0, NULL);
    CFRelease(surface);
    return accepted && rgb && !kr ? 0 : 1;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    @autoreleasepool {
        @try {
            if (argc == 2 && !strcmp(argv[1], "software")) return bitmap();
            if (argc == 2 && !strcmp(argv[1], "eagl")) return eagl();
            if (argc == 2 && !strcmp(argv[1], "layer")) return layer_bitmap();
            if (argc == 2 && !strcmp(argv[1], "surface")) return surface_bitmap();
            if (argc == 2 && !strcmp(argv[1], "methods")) return renderer_methods();
            if (argc == 2 && !strcmp(argv[1], "capture")) return capture_display();
            fputs("usage: whyred-render-probe software | eagl | layer | surface | methods | capture\n", stderr);
            return 2;
        } @catch (NSException *exception) {
            printf("WHYRED RENDER: exception=%.*s reason=%.*s\n", 256,
                   exception.name.UTF8String, 512, exception.reason.UTF8String);
            return 1;
        }
    }
}
