/*!
  \file   ihandlers.cpp
  \brief  interrupt handlers

  interrupt handlers

  Copyright (c) 2002, 2003 and 2004 Higepon
  All rights reserved.
  License=MIT/X Licnese

  \author  HigePon
  \version $Revision$
  \date   create:2002/07/25 update:$Date$
*/

#include <global.h>
#include <io.h>
#include <ihandlers.h>
#include <KeyBoardManager.h>
#include <tester.h>
#include <Process.h>

/*!
  \brief mouse handler

  mouse handler IRQ 12

  \author HigePon
  \date   create:2004/02/05 update:
*/
void mouseHandler() {

    static int counter = 0;
//     static bool sendFlag = false;
    if (Mouse::waitReadable()) {
        g_console->printf("mouse time out");
    }

    byte data = inportb(0x60);

    MessageInfo message;

    memset(&message, 0, sizeof(MessageInfo));

    int type = counter % 3;

    if (type == 0) {
        message.header = MSG_MOUSE_1;
    } else if (type == 1) {
        message.header = MSG_MOUSE_2;
    } else {
        message.header = MSG_MOUSE_3;
    }

    message.arg1   = data;

    if (g_messenger->send("MOUSE.SVR", &message)) {
        g_console->printf("mouse send failed");
    }

    counter++;

    /* EOI is below for IRQ 8-15 */
    outportb(0xA0, 0x20);
    outportb(0x20, 0x20);
}

/*!
  \brief key stroke handler

  key storoke handler IRQ 1

  \author HigePon
  \date   create:2002/07/25 update:2003/10/03
*/
void keyStrokeHandler(dword scancode) {

    MessageInfo message;

    memset(&message, 0, sizeof(MessageInfo));

    message.header = MSG_KEY_SCANCODE;
    message.arg1   = scancode;

    if (g_messenger->send("KEYBDMNG.SVR", &message)) {
        g_console->printf("send failed");
    }

    /* EOI is below for IRQ 0-7 */
    outportb(0x20, 0x20);
}

/*!
  \brief fault0d handler

  fault0d handler

  \author HigePon
  \date   create:2002/09/06 update:2003/01/26
*/
void fault0dHandler(dword error) {

    dokodemoView();
    g_console->printf("error=%x\n", error);
    panic("fault0d");
}

/*!
  \brief dummy handler

  dummy handler

  \author HigePon
  \date   create:2002/07/25 update:2003/03/24
*/
void dummyHandler() {

    g_console->printf("dummy Handler\n");

    /* EOI is below for IRQ 8-15 */
    //    outportb(0xA0, 0x20);
    //    outportb(0x20, 0x20);
}

/*!
  \brief timer handler

  timer handler
  at this function task switch occurs.

  \author HigePon
  \date   create:2002/11/21 update:2003/02/24
*/
void timerHandler() {

    /* EOI is below for IRQ 8-15 */
    //    outportb(0xA0, 0x20);
    outportb(0x20, 0x20);

    g_scheduler->tick();
    dword tick = g_scheduler->getTick();
    bool isProcessChange = (tick % 10) ? g_scheduler->schedule2() : g_scheduler->schedule();

    ThreadOperation::switchThread(isProcessChange);

    /* does not come here */
}

/*!
  \brief MFDC handlerp

  \author HigePon
  \date   create:2003/02/09 update:2003/09/19
*/
void MFDCHandler(void) {

    /* even if FDCDriver has no instance, it works */
    FDCDriver::interrupt();

    /* thx! K-tan */
    outportb(0x20, 0x66);
}

/* IRQ Handler (expr) */
#define IRQHANDLERMaster(x) void irqHandler_##x(void) {  int i ; i = x ; g_console->printf("IRQMaster:%d\n",i); g_irqHandlers[x](); outportb(0x20, 0x20); }

#define IRQHANDLERSlave(x) void irqHandler_##x(void) {  int i ; i = x ;  g_console->printf("IRQSlave:%d\n",i);  g_irqHandlers[x]();outportb(0xA0, 0x20); outportb(0x20, 0x20); }

IRQHANDLERMaster(0)
    IRQHANDLERMaster(1)
    IRQHANDLERMaster(2)
    IRQHANDLERMaster(3)
    IRQHANDLERMaster(4)
    IRQHANDLERMaster(5)
    IRQHANDLERMaster(6)
    IRQHANDLERMaster(7)
    IRQHANDLERSlave(8)
    IRQHANDLERSlave(9)
    IRQHANDLERSlave(10)
    IRQHANDLERSlave(11)
    IRQHANDLERSlave(12)
    IRQHANDLERSlave(13)
    IRQHANDLERSlave(14)
    IRQHANDLERSlave(15)

void cpufaultHandler_0(void) {
    dokodemoView();
    panic("unhandled:fault00 - devied by 0");
}

void cpufaultHandler_1(void){
    dokodemoView();
    panic("unhandled:fault01 - debug");
}

void cpufaultHandler_5(void){
    dokodemoView();
    panic("unhandled:fault05 - BOUND");

}

void cpufaultHandler_6(void){
    dokodemoView();
    panic("unhandled:fault06 - invalid op code");

}

void cpufaultHandler_7(void){
    dokodemoView();
    panic("unhandled:fault07 no co-processor presents");
}

void cpufaultHandler_8(void){
    dokodemoView();
    panic("unhandled:abort08 - double fault");
}

void cpufaultHandler_a(void){
    dokodemoView();
    panic("unhandled:fault0A - invalid TSS");
}


void cpufaultHandler_b(void){
    dokodemoView();
    panic("unhandled:fault0B - segment not presents");
}

void cpufaultHandler_c(dword error){
    dokodemoView();
    panic("unhandled:fault0C - stack fault");
}

void cpufaultHandler_e(dword address, dword error){

    g_console->printf("page fault");
    if (!g_page_manager->pageFaultHandler(address, error)) {

        dokodemoView();
        panic("unhandled:fault0E - page fault");
    }
}

void cpufaultHandler_10(void){
    dokodemoView();
    panic("unhandled:fault10 - co-processor error");
}

void cpufaultHandler_11(void){
    dokodemoView();
    panic("unhandled:fault11 - arign check");
}

void dokodemoView() {

    DokodemoView* i = &g_dokodemo_view;
    StackView*    j = &g_stack_view;

    g_console->printf("\n");
    g_console->printf("eax=%x ebx=%x ecx=%x edx=%x\n", i->eax, i->ebx, i->ecx, i->edx);
    g_console->printf("esp=%x ebp=%x esi=%x edi=%x\n", i->esp, i->ebp, i->esi, i->edi);
    g_console->printf("cs =%x ds =%x ss =%x cre=%x\n", i->cs , i->ds , i->ss , i->cr3);
    g_console->printf("eflags=%x\n", i->eflags);
    g_console->printf("stack 0(%x) 1(%x) 2(%x) 3(%x)\n", j->stack0, j->stack1, j->stack2, j->stack3);
    g_console->printf("stack 4(%x) 6(%x) 5(%x) 7(%x)\n", j->stack4, j->stack5, j->stack6, j->stack7);
}

/*! \def global handler list */
InterruptHandlers handlers[IHANDLER_NUM] = {
    {0x00, &arch_cpufaulthandler_0}
    , {0x01, &arch_cpufaulthandler_1}
    , {0x02, &arch_dummyhandler}
    , {0x03, &arch_dummyhandler}
    , {0x04, &arch_dummyhandler}
    , {0x05, &arch_cpufaulthandler_5}
    , {0x06, &arch_cpufaulthandler_6}
    , {0x07, &arch_cpufaulthandler_7}
    , {0x08, &arch_cpufaulthandler_8}
    , {0x09, &arch_dummyhandler}
    , {0x0A, &arch_cpufaulthandler_a}
    , {0x0B, &arch_cpufaulthandler_b}
    , {0x0C, &arch_cpufaulthandler_c}
    , {0x0D, &arch_fault0dhandler}
    , {0x0E, &arch_cpufaulthandler_e}
    , {0x0F, &arch_dummyhandler}
    , {0x10, &arch_cpufaulthandler_10}
    , {0x11, &arch_cpufaulthandler_11}
    , {0x12, &arch_dummyhandler}
    , {0x13, &arch_dummyhandler}
    , {0x14, &arch_dummyhandler}
    , {0x15, &arch_dummyhandler}
    , {0x16, &arch_dummyhandler}
    , {0x17, &arch_dummyhandler}
    , {0x18, &arch_dummyhandler}
    , {0x19, &arch_dummyhandler}
    , {0x1A, &arch_dummyhandler}
    , {0x1B, &arch_dummyhandler}
    , {0x1C, &arch_dummyhandler}
    , {0x1D, &arch_dummyhandler}
    , {0x1E, &arch_dummyhandler}
    , {0x1F, &arch_dummyhandler}
    , {0x20, &arch_dummyhandler}
    , {0x21, &arch_dummyhandler}
    , {0x22, &arch_dummyhandler}
    , {0x23, &arch_dummyhandler}
    , {0x24, &arch_dummyhandler}
    , {0x25, &arch_dummyhandler}
    , {0x26, &arch_dummyhandler}
    , {0x27, &arch_dummyhandler}
    , {0x28, &arch_irqhandler_8}
    , {0x29, &arch_irqhandler_9}
    , {0x2A, &arch_irqhandler_10}
    , {0x2B, &arch_irqhandler_11}
    , {0x2C, &arch_mousehandler}
    , {0x2D, &arch_irqhandler_13}
    , {0x2E, &arch_irqhandler_14}
    , {0x2F, &arch_irqhandler_15}
    , {0x30, &arch_dummyhandler}
    , {0x31, &arch_dummyhandler}
    , {0x32, &arch_dummyhandler}
    , {0x33, &arch_dummyhandler}
    , {0x34, &arch_dummyhandler}
    , {0x35, &arch_dummyhandler}
    , {0x36, &arch_dummyhandler}
    , {0x37, &arch_dummyhandler}
    , {0x38, &arch_dummyhandler}
    , {0x39, &arch_dummyhandler}
    , {0x3A, &arch_dummyhandler}
    , {0x3B, &arch_dummyhandler}
    , {0x3C, &arch_dummyhandler}
    , {0x3D, &arch_dummyhandler}
    , {0x3E, &arch_dummyhandler}
    , {0x3F, &arch_dummyhandler}
    , {0x40, &arch_dummyhandler}
    , {0x41, &arch_dummyhandler}
    , {0x42, &arch_dummyhandler}
    , {0x43, &arch_dummyhandler}
    , {0x44, &arch_dummyhandler}
    , {0x45, &arch_dummyhandler}
    , {0x46, &arch_dummyhandler}
    , {0x47, &arch_dummyhandler}
    , {0x48, &arch_dummyhandler}
    , {0x49, &arch_dummyhandler}
    , {0x4A, &arch_dummyhandler}
    , {0x4B, &arch_dummyhandler}
    , {0x4C, &arch_dummyhandler}
    , {0x4D, &arch_dummyhandler}
    , {0x4E, &arch_dummyhandler}
    , {0x4F, &arch_dummyhandler}
    , {0x50, &arch_dummyhandler}
    , {0x51, &arch_dummyhandler}
    , {0x52, &arch_dummyhandler}
    , {0x53, &arch_dummyhandler}
    , {0x54, &arch_dummyhandler}
    , {0x55, &arch_dummyhandler}
    , {0x56, &arch_dummyhandler}
    , {0x57, &arch_dummyhandler}
    , {0x58, &arch_dummyhandler}
    , {0x59, &arch_dummyhandler}
    , {0x5A, &arch_dummyhandler}
    , {0x5B, &arch_dummyhandler}
    , {0x5C, &arch_dummyhandler}
    , {0x5D, &arch_dummyhandler}
    , {0x5E, &arch_dummyhandler}
    , {0x5F, &arch_dummyhandler}
    , {0x60, &arch_timerhandler} /* IRQ 0 */
#ifdef MJT
    , {0x61, &arch_irqhandler_1}
#else
    , {0x61, &arch_keystrokehandler}
#endif
    , {0x62, &arch_irqhandler_2}
    , {0x63, &arch_irqhandler_3}
    , {0x64, &arch_irqhandler_4}
    , {0x65, &arch_irqhandler_5}
    , {0x66, &arch_fdchandler}
    , {0x67, &arch_irqhandler_7}
    , {0x68, &arch_dummyhandler}
    , {0x69, &arch_dummyhandler}
    , {0x6A, &arch_dummyhandler}
    , {0x6B, &arch_dummyhandler}
    , {0x6C, &arch_dummyhandler}
    , {0x6D, &arch_dummyhandler}
    , {0x6E, &arch_dummyhandler}
    , {0x6F, &arch_dummyhandler}
    , {0x70, &arch_dummyhandler}
    , {0x71, &arch_dummyhandler}
    , {0x72, &arch_dummyhandler}
    , {0x73, &arch_dummyhandler}
    , {0x74, &arch_dummyhandler}
    , {0x75, &arch_dummyhandler}
    , {0x76, &arch_dummyhandler}
    , {0x77, &arch_dummyhandler}
    , {0x78, &arch_dummyhandler}
    , {0x79, &arch_dummyhandler}
    , {0x7A, &arch_dummyhandler}
    , {0x7B, &arch_dummyhandler}
    , {0x7C, &arch_dummyhandler}
    , {0x7D, &arch_dummyhandler}
    , {0x7E, &arch_dummyhandler}
    , {0x7F, &arch_dummyhandler}
    , {0x80, &arch_syscall_handler}
    , {0x81, &arch_dummyhandler}
    , {0x82, &arch_dummyhandler}
    , {0x83, &arch_dummyhandler}
    , {0x84, &arch_dummyhandler}
    , {0x85, &arch_dummyhandler}
    , {0x86, &arch_dummyhandler}
    , {0x87, &arch_dummyhandler}
    , {0x88, &arch_dummyhandler}
    , {0x89, &arch_dummyhandler}
    , {0x8A, &arch_dummyhandler}
    , {0x8B, &arch_dummyhandler}
    , {0x8C, &arch_dummyhandler}
    , {0x8D, &arch_dummyhandler}
    , {0x8E, &arch_dummyhandler}
    , {0x8F, &arch_dummyhandler}
    , {0x90, &arch_dummyhandler}
    , {0x91, &arch_dummyhandler}
    , {0x92, &arch_dummyhandler}
    , {0x93, &arch_dummyhandler}
    , {0x94, &arch_dummyhandler}
    , {0x95, &arch_dummyhandler}
    , {0x96, &arch_dummyhandler}
    , {0x97, &arch_dummyhandler}
    , {0x98, &arch_dummyhandler}
    , {0x99, &arch_dummyhandler}
    , {0x9A, &arch_dummyhandler}
    , {0x9B, &arch_dummyhandler}
    , {0x9C, &arch_dummyhandler}
    , {0x9D, &arch_dummyhandler}
    , {0x9E, &arch_dummyhandler}
    , {0x9F, &arch_dummyhandler}
    , {0xA0, &arch_dummyhandler}
    , {0xA1, &arch_dummyhandler}
    , {0xA2, &arch_dummyhandler}
    , {0xA3, &arch_dummyhandler}
    , {0xA4, &arch_dummyhandler}
    , {0xA5, &arch_dummyhandler}
    , {0xA6, &arch_dummyhandler}
    , {0xA7, &arch_dummyhandler}
    , {0xA8, &arch_dummyhandler}
    , {0xA9, &arch_dummyhandler}
    , {0xAA, &arch_dummyhandler}
    , {0xAB, &arch_dummyhandler}
    , {0xAC, &arch_dummyhandler}
    , {0xAD, &arch_dummyhandler}
    , {0xAE, &arch_dummyhandler}
    , {0xAF, &arch_dummyhandler}
    , {0xB0, &arch_dummyhandler}
    , {0xB1, &arch_dummyhandler}
    , {0xB2, &arch_dummyhandler}
    , {0xB3, &arch_dummyhandler}
    , {0xB4, &arch_dummyhandler}
    , {0xB5, &arch_dummyhandler}
    , {0xB6, &arch_dummyhandler}
    , {0xB7, &arch_dummyhandler}
    , {0xB8, &arch_dummyhandler}
    , {0xB9, &arch_dummyhandler}
    , {0xBA, &arch_dummyhandler}
    , {0xBB, &arch_dummyhandler}
    , {0xBC, &arch_dummyhandler}
    , {0xBD, &arch_dummyhandler}
    , {0xBE, &arch_dummyhandler}
    , {0xBF, &arch_dummyhandler}
    , {0xC0, &arch_dummyhandler}
    , {0xC1, &arch_dummyhandler}
    , {0xC2, &arch_dummyhandler}
    , {0xC3, &arch_dummyhandler}
    , {0xC4, &arch_dummyhandler}
    , {0xC5, &arch_dummyhandler}
    , {0xC6, &arch_dummyhandler}
    , {0xC7, &arch_dummyhandler}
    , {0xC8, &arch_dummyhandler}
    , {0xC9, &arch_dummyhandler}
    , {0xCA, &arch_dummyhandler}
    , {0xCB, &arch_dummyhandler}
    , {0xCC, &arch_dummyhandler}
    , {0xCD, &arch_dummyhandler}
    , {0xCE, &arch_dummyhandler}
    , {0xCF, &arch_dummyhandler}
    , {0xD0, &arch_dummyhandler}
    , {0xD1, &arch_dummyhandler}
    , {0xD2, &arch_dummyhandler}
    , {0xD3, &arch_dummyhandler}
    , {0xD4, &arch_dummyhandler}
    , {0xD5, &arch_dummyhandler}
    , {0xD6, &arch_dummyhandler}
    , {0xD7, &arch_dummyhandler}
    , {0xD8, &arch_dummyhandler}
    , {0xD9, &arch_dummyhandler}
    , {0xDA, &arch_dummyhandler}
    , {0xDB, &arch_dummyhandler}
    , {0xDC, &arch_dummyhandler}
    , {0xDD, &arch_dummyhandler}
    , {0xDE, &arch_dummyhandler}
    , {0xDF, &arch_dummyhandler}
    , {0xE0, &arch_dummyhandler}
    , {0xE1, &arch_dummyhandler}
    , {0xE2, &arch_dummyhandler}
    , {0xE3, &arch_dummyhandler}
    , {0xE4, &arch_dummyhandler}
    , {0xE5, &arch_dummyhandler}
    , {0xE6, &arch_dummyhandler}
    , {0xE7, &arch_dummyhandler}
    , {0xE8, &arch_dummyhandler}
    , {0xE9, &arch_dummyhandler}
    , {0xEA, &arch_dummyhandler}
    , {0xEB, &arch_dummyhandler}
    , {0xEC, &arch_dummyhandler}
    , {0xED, &arch_dummyhandler}
    , {0xEE, &arch_dummyhandler}
    , {0xEF, &arch_dummyhandler}
    , {0xF0, &arch_dummyhandler}
    , {0xF1, &arch_dummyhandler}
    , {0xF2, &arch_dummyhandler}
    , {0xF3, &arch_dummyhandler}
    , {0xF4, &arch_dummyhandler}
    , {0xF5, &arch_dummyhandler}
    , {0xF6, &arch_dummyhandler}
    , {0xF7, &arch_dummyhandler}
    , {0xF8, &arch_dummyhandler}
    , {0xF9, &arch_dummyhandler}
    , {0xFA, &arch_dummyhandler}
    , {0xFB, &arch_dummyhandler}
    , {0xFC, &arch_dummyhandler}
    , {0xFD, &arch_dummyhandler}
    , {0xFE, &arch_dummyhandler}
    , {0xFF, &arch_dummyhandler}
};
