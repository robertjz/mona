#include <monapi.h>
#define MUNIT_GLOBAL_VALUE_DEFINED
#include <monapi/MUnit.h>
#include <monapi/net.h>
#include <monapi/Buffer.h>

using namespace MonAPI;

enum {
    MSG_SEND_TEST
};

void testDate()
{
    uint64_t msec1 = Date::nowInMsec();
    sleep(100);
    uint64_t msec2 = Date::nowInMsec();
    uint64_t diff = msec2 - msec1;
    EXPECT_TRUE(diff > 70);
    EXPECT_TRUE(diff < 130);
}

void testThreadSelf()
{
    uintptr_t self = syscall_mthread_self();
    EXPECT_EQ(self, System::getThreadID());
}

static void __fastcall infiniteThread(void* arg)
{
    for (;;) {
        MessageInfo msg;
        if (Message::receive(&msg) != M_OK) {
            continue;
        }
    }
    exit(0);
}


void testThreadKill()
{
    uintptr_t tid = syscall_mthread_create_with_arg(infiniteThread, NULL);
    EXPECT_EQ(M_OK, Message::send(tid, MSG_SEND_TEST));
    EXPECT_EQ(M_OK, syscall_mthread_kill(tid));
    SKIP(EXPECT_EQ(M_BAD_THREAD_ID, Message::send(tid, MSG_SEND_TEST)));
}

void testNet()
{
    EXPECT_EQ(0x3412, htons(0x1234));
}

static void testISO9600_file_read()
{
    intptr_t handle = monapi_file_open("/LIBS/SCHEME/fib.scm", 0);
    monapi_cmemoryinfo* cmi = monapi_file_read(handle, 4096);
    ASSERT_TRUE(cmi != NULL);
    EXPECT_TRUE(cmi->Size > 0);
    monapi_cmemoryinfo_dispose(cmi);
    monapi_cmemoryinfo_delete(cmi);

    // reached EOF
    monapi_cmemoryinfo* cmi2 = monapi_file_read(handle, 4096);
    EXPECT_TRUE(NULL == cmi2);
    monapi_file_close(handle);
}

static void test_Page_fault_handler_should_be_fast()
{
    monapi_cmemoryinfo* mi = monapi_cmemoryinfo_new();
    const int SIZE = 3 * 1024 * 1024;
    _logprintf("%s %s:%d\n", __func__, __FILE__, __LINE__);
    ASSERT_EQ(M_OK, monapi_cmemoryinfo_create(mi, SIZE, MONAPI_FALSE, 1));
    uint64_t s1 = MonAPI::Date::nowInMsec();
    memset(mi->Data, 0, SIZE);
    uint64_t e1 = MonAPI::Date::nowInMsec();
    EXPECT_TRUE((e1 - s1) <= 10);

    uint64_t s2 = MonAPI::Date::nowInMsec();
    memset(mi->Data, 0, SIZE);
    uint64_t e2 = MonAPI::Date::nowInMsec();
    EXPECT_EQ(0, e2 - s2);
    monapi_cmemoryinfo_dispose(mi);
    monapi_cmemoryinfo_delete(mi);

}

static void testBuffer()
{
    const int BUFFER_SIZE = 32;;
    uint8_t data1[BUFFER_SIZE];
    uint8_t data2[BUFFER_SIZE];
    data1[0] = 0xff;
    data1[BUFFER_SIZE - 1] = 0xee;
    Buffer buf1(data1, BUFFER_SIZE);
    EXPECT_EQ(BUFFER_SIZE, buf1.size());
    buf1.fillZero();
    EXPECT_EQ(0, data1[0]);
    EXPECT_EQ(0, data1[BUFFER_SIZE - 1]);

    Buffer buf2(data2, BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        data2[i] = i & 0xff;
    }

    EXPECT_TRUE(Buffer::copy(buf1, 0, buf2, 1, 3));
    EXPECT_EQ(1, data1[0]);
    EXPECT_EQ(2, data1[1]);
    EXPECT_EQ(3, data1[2]);
}

static void testBufferOverCopyShouldFail()
{
    const int BUFFER_SIZE = 32;;
    uint8_t data1[BUFFER_SIZE];
    uint8_t data2[BUFFER_SIZE];
    Buffer buf1(data1, BUFFER_SIZE);
    buf1.fillZero();
    Buffer buf2(data2, BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        data2[i] = i & 0xff;
    }

    EXPECT_EQ(true, Buffer::copy(buf1, 0, buf2, 0, BUFFER_SIZE));
    EXPECT_EQ(false, Buffer::copy(buf1, 0, buf2, 0, BUFFER_SIZE + 1));
    EXPECT_EQ(true, Buffer::copy(buf1, 5, buf2, 0, BUFFER_SIZE - 5));
    EXPECT_EQ(false, Buffer::copy(buf1, 5, buf2, 0, BUFFER_SIZE - 4));
    EXPECT_EQ(true, Buffer::copy(buf1, 0, buf2, 5, BUFFER_SIZE - 5));
    EXPECT_EQ(false, Buffer::copy(buf1, 0, buf2, 5, BUFFER_SIZE - 4));
    EXPECT_EQ(true, Buffer::copy(buf1, 5, buf2, 5, BUFFER_SIZE - 5));
    EXPECT_EQ(false, Buffer::copy(buf1, 5, buf2, 5, BUFFER_SIZE - 4));
}


int main(int argc, char *argv[])
{
    testDate();
    testThreadSelf();
    testNet();
    testThreadKill();
    testISO9600_file_read();
    test_Page_fault_handler_should_be_fast();
    testBuffer();
    testBufferOverCopyShouldFail();
    TEST_RESULTS(monapi_misc);
    return 0;
}
