#include <userlib.h>
#include <MemoryManager.h>

int sleep(dword tick) {return syscall_sleep(tick);}
int heavy() {return syscall_heavy();}
int print(const char* msg) {return syscall_print(msg);}
int _put_pixel(int x, int y, char color) {return syscall_put_pixel(x, y, color);}
int kill() {return syscall_kill();}
int exit(int error) {return syscall_kill();}
int _send(const char* name, Message* message) {return syscall_send(name, message);}
int _receive(Message* message) {return syscall_receive(message);}


int monamain();

static MemoryManager um;

int user_start() {
    int result;
    //    um.initialize(0xC0000000,0xC0000000 + 1024 * 1024);

    result = monamain();
    exit(result);
    for (;;);
}

int syscall_sleep(dword tick) {

    int result;

    asm volatile("movl $%c2, %%ebx \n"
                 "movl %1  , %%esi \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"m"(tick), "g"(SYSTEM_CALL_PROCESS_SLEEP)
                 );

    return result;
}

int syscall_heavy() {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_HEAVEY)
                 );

    return result;
}

int syscall_print(const char* msg) {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "movl %2  , %%esi \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_PRINT), "m"(msg)
                 );

    return result;
}

int syscall_put_pixel(int x, int y, char color) {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "movl %2  , %%esi \n"
                 "movl %3  , %%ecx \n"
                 "movl %4  , %%edi \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_PUT_PIXEL), "m"(x), "m"(y), "m"(color)
                 );

    return result;
}

int syscall_kill() {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_KILL)
                 );

    /* don't come here */
    return result;
}

int syscall_send(const char* name, Message* message) {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "movl %2  , %%esi \n"
                 "movl %3  , %%ecx \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_SEND), "m"(name), "m"(message)
                 );

    return result;
}
int syscall_receive(Message* message) {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "movl %2  , %%esi \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_RECEIVE), "m"(message)
                 );

    return result;
}

void* umalloc(unsigned long size) {

    return um.allocate(size);
}

void ufree(void * address) {

    um.free(address);
    return;
}
