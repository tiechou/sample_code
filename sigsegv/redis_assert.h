#include<stdio.h>
#include <unistd.h>
/* test for backtrace() */
#if defined(__APPLE__) || defined(__linux__)
#define HAVE_BACKTRACE 1
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#include <ucontext.h>
#endif /* HAVE_BACKTRACE */
 
/* Anti-warning macro... */
#define REDIS_NOTUSED(V) ((void) V)

/* We can print the stacktrace, so our assert is defined this way: */
#define redisAssert(_e) ((_e)?(void)0 : (_redisAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define redisPanic(_e) _redisPanic(#_e,__FILE__,__LINE__),_exit(1)
void _redisAssert(char *estr, char *file, int line);
void _redisPanic(char *msg, char *file, int line);

static void sigtermHandler(int sig) {
    REDIS_NOTUSED(sig);
    fprintf(stderr,"Received SIGTERM, scheduling shutdown...\n");
}

#ifdef HAVE_BACKTRACE
    static void *getMcontextEip(ucontext_t *uc) {
#if defined(__FreeBSD__)
        return (void*) uc->uc_mcontext.mc_eip;
#elif defined(__dietlibc__)
        return (void*) uc->uc_mcontext.eip;
#elif defined(__APPLE__) && !defined(MAC_OS_X_VERSION_10_6)
#if __x86_64__
        return (void*) uc->uc_mcontext->__ss.__rip;
#else
        return (void*) uc->uc_mcontext->__ss.__eip;
#endif
#elif defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
#if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
        return (void*) uc->uc_mcontext->__ss.__rip;
#else
        return (void*) uc->uc_mcontext->__ss.__eip;
#endif
#elif defined(__i386__)
        return (void*) uc->uc_mcontext.gregs[14]; /* Linux 32 */
#elif defined(__X86_64__) || defined(__x86_64__)
        return (void*) uc->uc_mcontext.gregs[16]; /* Linux 64 */
#elif defined(__ia64__) /* Linux IA64 */
        return (void*) uc->uc_mcontext.sc_ip;
#else
        return NULL;
#endif
    }
static void sigsegvHandler(int sig, siginfo_t *info, void *secret) {
    void *trace[100];
    char **messages = NULL;
    int i, trace_size = 0;
    ucontext_t *uc = (ucontext_t*) secret;
    struct sigaction act;
    REDIS_NOTUSED(info);

    fprintf(stderr,"======= Ooops! Redis got signal: -%d- =======\n", sig);

    trace_size = backtrace(trace, 100);
    /* overwrite sigaction with caller's address */
    if (getMcontextEip(uc) != NULL) {
        trace[1] = getMcontextEip(uc);
    }
    messages = backtrace_symbols(trace, trace_size);

    for (i=1; i<trace_size; ++i){
        fprintf(stderr,"%s \n", messages[i]);
    }

    /* free(messages); Don't call free() with possibly corrupted memory. */

    /* Make sure we exit with the right signal at the end. So for instance
     *      * the core will be dumped if enabled. */
    sigemptyset (&act.sa_mask);
    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction
     *      * is used. Otherwise, sa_handler is used */
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    act.sa_handler = SIG_DFL;
    sigaction (sig, &act, NULL);
    kill(getpid(),sig);
}
#endif /* HAVE_BACKTRACE */

void setupSignalHandlers(void) {
    struct sigaction act;

    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
     *      * Otherwise, sa_handler is used. */
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    act.sa_handler = sigtermHandler;
    sigaction(SIGTERM, &act, NULL);

#ifdef HAVE_BACKTRACE
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = sigsegvHandler;
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
#endif
    return; 
}
/* The End */
void _redisAssert(char *estr, char *file, int line) { 
    fprintf(stderr,"=== ASSERTION FAILED ===\n");
    fprintf(stderr,"==> %s:%d '%s' is not true\n",file,line,estr);
#ifdef HAVE_BACKTRACE
    fprintf(stderr,"(forcing SIGSEGV in order to print the stack trace)\n");
    *((char*)-1) = 'x';
#endif
}

void _redisPanic(char *msg, char *file, int line) { 
    fprintf(stderr,"!!! Software Failure. Press left mouse button to continue\n");
    fprintf(stderr,"Guru Meditation: %s #%s:%d\n",msg,file,line);
#ifdef HAVE_BACKTRACE
    fprintf(stderr,"(forcing SIGSEGV in order to print the stack trace)\n");
    *((char*)-1) = 'x';
#endif
}


void foo(){
    int t = 1;
    redisAssert(t==2);
}
void test(){
    foo();
}

int main()
{
    //signal(SIGHUP, SIG_IGN);
    //signal(SIGPIPE, SIG_IGN);
    setupSignalHandlers();
    test();
}
