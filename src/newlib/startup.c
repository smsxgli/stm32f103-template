#include <newlib.h>

#if defined(_HAVE_INITFINI_ARRAY)
#define _init __libc_init_array
#define _fini __libc_fini_array
#endif /* defined(_HAVE_INITFINI_ARRAY) */

extern void __crt_startup(void) __attribute__((__weak__));
extern void hardware_init_hook(void) __attribute__((__weak__));
extern void software_init_hook(void) __attribute__((__weak__));
extern void __initialize_args(int *, char ***);
extern int main(int argc, char *argv[]);
extern void exit(int) __attribute__((__noreturn__));
#if defined(__USES_INITFINI__)
extern void _init(void);
extern void _fini(void)
#if defined(_LITE_EXIT)
    __attribute__((__weak__));
extern int atexit(void (*)(void)) __attribute__((__weak__))
#endif /* defined(_LITE_EXIT) */
;
#endif /* defined (__USES_INITFINI__) */

void __crt_startup(void) {
  int code = 0;
  int argc;
  char **argv;
  if (0 != &hardware_init_hook) {
    hardware_init_hook();
  }
  if (0 != &software_init_hook) {
    software_init_hook();
  }
#if defined(__USES_INITFINI__)
#if defined(_LITE_EXIT)
  if (0 != &atexit) {
    atexit(_fini);
  }
  _init();
#endif /* defined(_LITE_EXIT) */
#endif /* defined (__USES_INITFINI__) */
  __initialize_args(&argc, &argv);
  code = main(argc, argv);
  exit(code);
}

#if defined(_HAVE_INITFINI_ARRAY)
#undef _init
#undef _fini
#if defined(__USES_INITFINI__)
void __attribute__((__weak__)) _init(void) {}
void __attribute__((__weak__)) _fini(void) {}
#endif /* defined (__USES_INITFINI__) */
#endif /* defined(_HAVE_INITFINI_ARRAY) */
