#include <userlib.h>

/*----------------------------------------------------------------------
    Static
----------------------------------------------------------------------*/
static MemoryManager um;

/*----------------------------------------------------------------------
    Global
----------------------------------------------------------------------*/
MonaApplication* monaApp;

/*----------------------------------------------------------------------
    entry point for application
----------------------------------------------------------------------*/
int user_start() {

    int result;
    um.initialize(0xC0000000, 0xC0000000 + 8 * 1024 * 1024);
    List<char*>* arg = new HList<char*>();
    setupArguments(arg);
    result = MonaMain(arg);
    delete arg;
    exit(result);
    return 0;
}

void setupArguments(List<char*>* arg) {

    char* str;
    int num = syscall_get_arg_count();

    for (int i = 0; i < num; i++) {

        str = (char*)malloc(32);
        if (syscall_get_arg(str, i) == 1) break;
        arg->add(str);
    }
}

/*----------------------------------------------------------------------
    messageLoop for MonaApplication
----------------------------------------------------------------------*/
void messageLoop() {

    volatile MessageInfo message;

    for (;;) {
        if (!Message::receive((MessageInfo*)(&message))) {

            switch(message.header) {

            case MSG_KEY_VIRTUAL_CODE:

                monaApp->onKeyDown(message.arg1, message.arg2);
                break;

            case MSG_MOUSE_INFO:

                if (message.arg3 & 0x01) {
                    monaApp->onMouseClick((int)(message.arg1), (int)(message.arg2));
                }
                break;
            default:

                /* ignore this message */
                break;
            }
        }
    }
}

/*----------------------------------------------------------------------
    MonaApplication
----------------------------------------------------------------------*/
MonaApplication::MonaApplication() {

    monaApp = this;

    int id = syscall_mthread_create((dword)MESSAGE_LOOP);
    syscall_mthread_join(id);

    mypid_ = System::getThreadID();

    /* create message for KEYBDMNG.SVR */
    MessageInfo info;
    info.header = MSG_KEY_REGIST_TO_SERVER;
    info.arg1   = mypid_;

    /* send */
    if (Message::send(KEYBOARD_SERVER, &info)) {
        printf("MonaApp: key regist error\n");
    }
}

MonaApplication::~MonaApplication() {

    /* create message for KEYBDMNG.SVR */
    MessageInfo info;
    info.header = MSG_KEY_UNREGIST_FROM_SERVER;
    info.arg1   = mypid_;

    /* send */
    if (Message::send(KEYBOARD_SERVER, &info)) {
        printf("MonaApp: key unregist error\n");
    }
}

void MonaApplication::run() {

    volatile MessageInfo message;

    for (;;) {
        if (!Message::receive((MessageInfo*)(&message))) {

            switch(message.header) {

            case MSG_KEY_VIRTUAL_CODE:

                this->onKeyDown(message.arg1, message.arg2);
                break;

            case MSG_MOUSE_INFO:

                if (message.arg3 & 0x01) {
                    this->onMouseClick((int)(message.arg1), (int)(message.arg2));
                }
                break;
            default:

                /* ignore this message */
                break;
            }
        }
    }
}

/*----------------------------------------------------------------------
    system call wrappers
----------------------------------------------------------------------*/
int sleep(dword ms) {

    dword tick = ms / 10;

    if (tick <= 0)
    {
        tick = 1;
    }

    return syscall_sleep(tick);
}

int print(const char* msg) {
    return syscall_print(msg);
}

int kill() {
    return syscall_kill();
}

int exit(int error) {
    return syscall_kill();
}

int mthread_create(dword f) {
    return syscall_mthread_create(f);
}

int mthread_join(dword id) {
    return syscall_mthread_join(id);
}

/*----------------------------------------------------------------------
    Observer/Observable
----------------------------------------------------------------------*/
void Observable::addObserver(Observer* o) {
    this->observers.add(o);
}

void Observable::deleteObservers() {

    for (int i = this->observers.size() - 1; i >=0; i--) {
        this->observers.removeAt(i);
    }
}

void Observable::setChanged() {
    this->changed = true;
}

void Observable::clearChanged() {
    this->changed = false;
}

bool Observable::hasChanged() const {
    return this->changed;
}

int Observable::countObservers() {
    return observers.size();
}

void Observable::notifyObservers() {

    this->notifyObservers(NULL);
}

void Observable::notifyObservers(void* arg) {

    for (int i = 0; i < this->observers.size(); i++) {
        this->observers.get(i)->update(this, arg);
    }
    clearChanged();
}

/*----------------------------------------------------------------------
    Floppy
----------------------------------------------------------------------*/
Floppy::Floppy(int device) {
}

Floppy::~Floppy() {
}

int Floppy::open() {
    return syscall_fdc_open();
}

int Floppy::close() {
    return syscall_fdc_close();
}

int Floppy::read(dword lba,  byte* buf, dword blocknum) {;
    return syscall_fdc_read(lba, buf, blocknum);
}

int Floppy::write(dword lba, byte* buf, dword blocknum) {
    return syscall_fdc_write(lba, buf, blocknum);
}

bool Floppy::diskChanged() {
    return syscall_fdc_disk_changed();
}

int Floppy::ioctl(void* p) {

    /* not supported */
    return 0;
}

/*----------------------------------------------------------------------
    malloc / free
----------------------------------------------------------------------*/
void* malloc(unsigned long size) {
    return um.allocate(size);
}

void free(void * address) {
    um.free(address);
    return;
}

/*----------------------------------------------------------------------
    for C++
----------------------------------------------------------------------*/
int atexit( void (*func)(void)) {
    return -1;
}

void __cxa_pure_virtual() {
    print("__cxa_pure_virtual called\n");
}

void _pure_virtual() {
    print("_pure_virtual called\n");
}

void __pure_virtual() {
    print("_pure_virtual called\n");
}

/*----------------------------------------------------------------------
    I/O
----------------------------------------------------------------------*/
byte inportb(dword port) {

    byte ret;
    asm volatile ("inb %%dx, %%al": "=a"(ret): "d"(port));
    return ret;
}

void outportb(dword port, byte value) {
   asm volatile ("outb %%al, %%dx": :"d" (port), "a" (value));
}

/*----------------------------------------------------------------------
    operator new/delete
----------------------------------------------------------------------*/
void* operator new(size_t size) {
    return um.allocate(size);
}

void operator delete(void* address) {
    um.free(address);
    return;
}

void* operator new[](size_t size) {
    return um.allocate(size);
}

void operator delete[](void* address) {
    um.free(address);
    return;
}

/*----------------------------------------------------------------------
    Mutex
----------------------------------------------------------------------*/
Mutex::Mutex() {
}

Mutex::~Mutex() {
}

int Mutex::init() {

    mutexId_ = syscall_mutex_create();
    return mutexId_;
}

int Mutex::lock() {
    return syscall_mutex_lock(mutexId_);
}

int Mutex::unlock() {
    return syscall_mutex_unlock(mutexId_);
}

int Mutex::tryLock() {
    return syscall_mutex_trylock(mutexId_);
}

int Mutex::destory() {
    return syscall_mutex_destroy(mutexId_);
}

/*----------------------------------------------------------------------
    VirtualScreen
----------------------------------------------------------------------*/
VirtualScreen::VirtualScreen() : Screen() {

    int x = getWidth();
    int y = getHeight();
    vram_ = new byte[x * y * bpp_ / 8];
    if (vram_ == NULL) printf("vitual vram error\n");
}

VirtualScreen::VirtualScreen(dword vramsize) : Screen() {

    vram_ = new byte[vramsize];
    if (vram_ == NULL) printf("vitual vram error\n");
}


VirtualScreen::~VirtualScreen() {

    if (vram_) {
        delete vram_;
    }
}

/*----------------------------------------------------------------------
    Screen
----------------------------------------------------------------------*/
Screen::Screen() {

    volatile ScreenInfo sinfo;

    /* get and set vram information */
    syscall_get_vram_info(&sinfo);
    vram_        = (byte*)sinfo.vram;
    bpp_         = sinfo.bpp;
    xResolution_ = sinfo.x;
    yResolution_ = sinfo.y;
}

Screen::~Screen() {
}

void Screen::putPixel16(int x, int y, dword color) {

    int bytesPerPixel = bpp_ / 8;
    byte* vram       = vram_;

    vram += (x + y * xResolution_) * bytesPerPixel;
    *((word*)vram) = (word)color;
}

void Screen::fillRect16(int x, int y, int w, int h, dword color) {

        dword bytesPerPixel = bpp_ / 8;

        byte* position = vram_ + (x + y * xResolution_) * bytesPerPixel;
        byte* temp     = position;

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                *((word*)temp) = color;
                temp += bytesPerPixel;
            }
            position += xResolution_ * bytesPerPixel;
            temp = position;
        }
}

bool Screen::bitblt(Screen* destScreen, int destX, int destY, int width, int height
                    , Screen* sourceScreen, int sourceX, int sourceY) {

    /* check range */
    /* not yet     */

    byte* dvram      = destScreen->getVRAM();
    byte* svram      = sourceScreen->getVRAM();
    int xResolution  = destScreen->getWidth();
    int bitsPerPixel = destScreen->getBpp();

    switch(bitsPerPixel) {

    case(16):
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                copyPixel16(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    case(24):
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {

                copyPixel24(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    case(32):

        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {

                copyPixel32(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    default:
        return false;
    }
    return true;
}

bool Screen::bitblt(Screen* destScreen, int destX, int destY, int width, int height
                    , Screen* sourceScreen, int sourceX, int sourceY, int raster) {

    /* check range */
    /* not yet     */

    byte* dvram      = destScreen->getVRAM();
    byte* svram      = sourceScreen->getVRAM();
    int xResolution  = destScreen->getWidth();
    int bitsPerPixel = destScreen->getBpp();

    switch(bitsPerPixel) {

    case(16):
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                copyPixel16XOR(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    case(24):
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {

                copyPixel24XOR(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    case(32):
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {

                copyPixel32XOR(dvram, destX + w, destY + h, svram, sourceX + w, sourceY + h, xResolution);
            }
        }
        break;

    default:
        return false;
    }
    return true;
}

/* from orange pekoe */
void Screen::circle16(int x, int y, int r, dword color) {

    int w, h, f;
    w = r;
    h = 0;
    f = -2 * r + 3;

    while (w >= h) {
        putPixel16(x + w, y + h, color);
        putPixel16(x - w, y + h, color);
        putPixel16(x + h, y + w, color);
        putPixel16(x - h, y + w, color);
        putPixel16(x - w, y - h, color);
        putPixel16(x - h, y - w, color);
        putPixel16(x + h, y - w, color);
        putPixel16(x + w, y - h, color);

        if (f >= 0) {
            w--;
            f -= 4 * w;
        }
        h ++;
        f += 4 * h + 2;
    }
}

void Screen::fillCircle16(int x, int y, int r, dword color) {

    int i;
    int w, h;
    w = r;
    h = 0;
    int f = -2 * r + 3;

    while(w >= h) {
        for (i = x - w; i <= x + w; i ++) {
            putPixel16(i, y + h, color);
            putPixel16(i, y - h, color);
        }
        for (i = x - h; i <= x + h; i ++) {
            putPixel16(i, y - w, color);
            putPixel16(i, y + w, color);
        }
        if (f >= 0) {

            w--;
            f -= 4 * w;
        }
        h ++;
        f += 4 * h + 2;
    }
}

/*----------------------------------------------------------------------
    Message
----------------------------------------------------------------------*/
int Message::send(dword id, MessageInfo* info) {
    return syscall_send(id, info);
}

int Message::receive(MessageInfo* info) {

    int result = syscall_receive(info);
    if (result != 0) {
         syscall_mthread_yeild_message();
         result = syscall_receive(info);
    }
    return result;
}

dword Message::lookup(const char* name) {
    return syscall_lookup(name);
}

void Message::create(MessageInfo* info, dword header, dword arg1, dword arg2, dword arg3, char* str) {

    info->header = header;
    info->arg1 = arg1;
    info->arg2 = arg2;
    info->arg3 = arg3;

    if (str) {
        strncpy(info->str, str, sizeof(info->str));
    }
    return;
}

bool Message::exist()
{
    return (syscall_exist_message() == 1);
}

/*----------------------------------------------------------------------
    printf
----------------------------------------------------------------------*/
void printf(const char *format, ...) {

    void** list = (void **)&format;

    ((char**)list) += 1;
    for (int i = 0; format[i] != '\0'; i++) {

        if (format[i] == '%') {
            i++;

            switch (format[i]) {
              case 's':
                  print((char *)*list);
                  ((char**)list) += 1;
                  break;
              case 'd':
                  putInt((int)*list, 10);
                  ((int*)list) += 1;
                  break;
              case 'x':
                  print("0x");
                  putInt((int)*list, 16);
                  ((int*)list) += 1;
                  break;
              case 'c':
                  putCharacter((char)(int)(*list));
                  ((char*)list) += 1;
                  break;
              case '%':
                  putCharacter('%');
                  break;
              case '\0':
                  i--;
                  break;
            }
        } else {
            putCharacter(format[i]);
        }
    }
}

#if 0
void printf(const char *format, ...) {

    char str[512];
    str[0] = '\0';
    va_list args;
    int result;

    va_start(args, format);
    result = vsprintf(str, format, args);
    va_end(args);
    if(result > 512){
      /*buffer overflow*/
    }

    print(str);
}
#endif

void putInt(size_t n, int base) {

    static char buf[256];
    int geta;
    int num = n;

    if (base != 16) {

        for (geta = 0; num; num /= 10, geta++);
        if ((int)n < 0) {
            geta++;
            base *= -1;
        }
    } else {
        geta = 8;
    }

    char* p = ltona(n, buf, geta, base);

    for (; *p != '\0'; p++) {
        putCharacter(*p);
    }
}

void putCharacter(char ch) {
    char* str = " ";
    str[0] = ch;
    print(str);
}

void printInt(int num) {

    char revstr[20];
    char str[20];
    int  i = 0;
    int  j = 0;

    /* negative number */
    if (num < 0) {
        str[0] = '-';
        j++;
        num = num * -1;
    }

    /* number to buffer */
    do {
        revstr[i] = '0' + (int)(num % 10);
        num = num / 10;
        i++;
    } while (num != 0);
    revstr[i] = '\0';

    /* revert */
    for (; i >= 0; j++) {
        str[j] = revstr[i - 1];
        i--;
    }

    /* print */
    print(str);
    return;
}

size_t _power(size_t x, size_t y) {

    size_t result = x;

    for (size_t i = 1; i < y; i++) {
        result *= x;
    }
    return (int)result;
}

/*----------------------------------------------------------------------
    FileInputStream
----------------------------------------------------------------------*/
FileInputStream::FileInputStream(char* file) : file_(file), fileSize_(0), isOpen_(false) {
}

FileInputStream::~FileInputStream() {
}

int FileInputStream::open() {

    int result;

    if (isOpen_) {
        return 0;
    }

    result = syscall_file_open(file_, 1, &fileSize_);

    if (result == 0) {
        isOpen_ = true;
    }

    return result;
}

dword FileInputStream::getFileSize() const {
    return fileSize_;
}

dword FileInputStream::getReadSize() const {
    return readSize_;
}

int FileInputStream::read(byte* buf, int size) {
    return syscall_file_read((char*)buf, size, &readSize_);
}

void FileInputStream::close() {

    if (!isOpen_) {
        return;
    }

    syscall_file_close();
    isOpen_ = false;
    return;
}

/*----------------------------------------------------------------------
    System call
----------------------------------------------------------------------*/
int syscall_mthread_create(dword f)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MTHREAD_CREATE, result, f);
    return result;
}

int syscall_mthread_join(dword id)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MTHREAD_JOIN, result, id);
    return result;
}

int syscall_sleep(dword tick)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_PROCESS_SLEEP, result, tick);
    return result;
}

int syscall_print(const char* msg)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_PRINT, result, msg);
    return result;
}

int syscall_load_process(const char* path, const char* name, CommandOption* list)
{
    int result;
    SYSCALL_3(SYSTEM_CALL_LOAD_PROCESS, result, path, name, list);
    return result;
}

int syscall_kill()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_KILL, result);

    /* not reached */
    return result;
}

int syscall_send(dword pid, MessageInfo* message)
{
    int result;
    SYSCALL_2(SYSTEM_CALL_SEND, result, pid, message);
    return result;
}

int syscall_receive(MessageInfo* message)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_RECEIVE, result, message);
    return result;
}

int syscall_mutex_create()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_MUTEX_CREATE, result);
    return result;
}

int syscall_mutex_trylock(int id)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MUTEX_TRYLOCK, result, id);
    return result;
}

int syscall_mutex_lock (int id )
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MUTEX_LOCK, result, id);
    return result;
}

int syscall_mutex_unlock(int id)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MUTEX_UNLOCK, result, id);
    return result;;
}

int syscall_mutex_destroy(int id)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MUTEX_DESTROY, result, id);
    return result;
}

dword syscall_lookup(const char* name)
{
    dword pid;
    SYSCALL_1(SYSTEM_CALL_LOOKUP, pid, name);
    return pid;
}

int syscall_get_vram_info(volatile ScreenInfo* info)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_GET_VRAM_INFO, result, info);
    return result;
}

int syscall_map(dword pid, dword sharedId, dword linearAddress, dword size)
{
    int result;
    SYSCALL_4(SYSTEM_CALL_MAP, result, pid, sharedId, linearAddress, size);
    return result;
}

int syscall_get_cursor(int* x, int* y)
{
    int result;
    SYSCALL_2(SYSTEM_CALL_GET_CURSOR, result, x, y);
    return result;
}

int syscall_set_cursor(int x, int y)
{
    int result;
    SYSCALL_2(SYSTEM_CALL_SET_CURSOR, result, x, y);
    return result;
}

int syscall_file_open(char* path, int mode, volatile dword* size)
{
    int result;
    SYSCALL_3(SYSTEM_CALL_FILE_OPEN, result, path, mode, size);
    return result;
}

int syscall_file_read(char* buf, dword size, dword* readSize)
{
    int result;
    SYSCALL_3(SYSTEM_CALL_FILE_READ, result, buf, size, readSize);
    return result;
}

int syscall_file_close()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_FILE_CLOSE, result);
    return result;
}

int syscall_fdc_read(dword lba, byte* buffer, dword blocknum)
{
    int result;
    SYSCALL_3(SYSTEM_CALL_FDC_READ, result, lba, buffer, blocknum);
    return result;
}

int syscall_fdc_write(dword lba, byte* buffer, dword blocknum)
{
    int result;
    SYSCALL_3(SYSTEM_CALL_FDC_WRITE, result, lba, buffer, blocknum);
    return result;
}

int syscall_fdc_open()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_FDC_OPEN, result);
    return result;
}

int syscall_fdc_close()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_FDC_CLOSE, result);
    return result;
}

int syscall_map2(MappingInfo* map)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_MAP_TWO, result, map);
    return result;
}

int syscall_unmap2(dword sharedId)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_UNMAP_TWO, result, sharedId);
    return result;
}

dword syscall_get_pid()
{
    dword result;
    SYSCALL_0(SYSTEM_CALL_GET_PID, result);
    return result;
}

dword syscall_get_tid()
{
    dword result;
    SYSCALL_0(SYSTEM_CALL_GET_TID, result);
    return result;
}

int syscall_get_arg_count()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_ARGUMENTS_NUM, result);
    return result;
}

int syscall_get_arg(char* buf, int n)
{
    int result;
    SYSCALL_2(SYSTEM_CALL_GET_ARGUMENTS, result, buf, n);
    return result;
}

int syscall_mthread_yeild_message()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_MTHREAD_YIELD_M, result);
    return result;
}

int syscall_get_date(KDate* date)
{
    int result;
    SYSCALL_1(SYSTEM_CALL_DATE, result, date);
    return result;
}

int syscall_get_io()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_GET_IO, result);
    return result;
}

int syscall_exist_message()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_EXIST_MESSAGE, result);
    return result;
}

int syscall_fdc_disk_changed()
{
    int result;
    SYSCALL_0(SYSTEM_CALL_FDC_DISK_CHANGED, result);
    return result;
}
