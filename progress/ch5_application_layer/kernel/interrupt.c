/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#include "interrupt.h"
#include "lib.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"

#define SAVE_ALL                    \
    "cld;           \n\t"           \
    "pushq  %rax;   \n\t"           \
    "pushq  %rax;   \n\t"           \
    "movq   %es,    %rax;   \n\t"   \
    "pushq  %rax;   \n\t"           \
    "movq   %ds,    %rax;   \n\t"   \
    "pushq  %rax;   \n\t"           \
    "xorq   %rax,   %rax;   \n\t"   \
    "pushq  %rbp;   \n\t"           \
    "pushq  %rdi;   \n\t"           \
    "pushq  %rsi;   \n\t"           \
    "pushq  %rdx;   \n\t"           \
    "pushq  %rcx;   \n\t"           \
    "pushq  %rbx;   \n\t"           \
    "pushq  %r8;    \n\t"           \
    "pushq  %r9;    \n\t"           \
    "pushq  %r10;   \n\t"           \
    "pushq  %r11;   \n\t"           \
    "pushq  %r12;   \n\t"           \
    "pushq  %r13;   \n\t"           \
    "pushq  %r14;   \n\t"           \
    "pushq  %r15;   \n\t"           \
    "movq   $0x10,  %rdx;   \n\t"   \
    "movq   %rdx,   %ds;    \n\t"   \
    "movq   %rdx,   %es;    \n\t"

/* 在內嵌組合語言表達式中，如果輸入輸出與損壞部分可以省略
 * 另外這裡不使用__asm__關鍵字是因為他是巨集，他會在其他部分被調用比如
 * __asm__ __volatile__ (
 *  SAVE_ALL
 * );
 */

#define IRQ_NAME2(nr) nr##_interrupt(void)
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)

#define Build_IRQ(nr)   \
void IRQ_NAME(nr);      \
__asm__ (	SYMBOL_NAME_STR(IRQ)#nr"_interrupt:     \n\t"   \
            "pushq  $0x00                           \n\t"   \
            SAVE_ALL                                        \
            "movq   %rsp,   %rdi                    \n\t"   \
            "leaq   ret_from_intr(%rip),	%rax    \n\t"   \
            "pushq  %rax                            \n\t"   \
            "movq   $"#nr", %rsi                    \n\t"   \
            "jmp    do_IRQ  \n\t");
// SYMBOL_NAME_STR此巨集在linkage.h中定義。

Build_IRQ(0x20)
Build_IRQ(0x21)
Build_IRQ(0x22)
Build_IRQ(0x23)
Build_IRQ(0x24)
Build_IRQ(0x25)
Build_IRQ(0x26)
Build_IRQ(0x27)
Build_IRQ(0x28)
Build_IRQ(0x29)
Build_IRQ(0x2a)
Build_IRQ(0x2b)
Build_IRQ(0x2c)
Build_IRQ(0x2d)
Build_IRQ(0x2e)
Build_IRQ(0x2f)
Build_IRQ(0x30)
Build_IRQ(0x31)
Build_IRQ(0x32)
Build_IRQ(0x33)
Build_IRQ(0x34)
Build_IRQ(0x35)
Build_IRQ(0x36)
Build_IRQ(0x37)

void (* interrupt[24])(void)=
{
    IRQ0x20_interrupt,
    IRQ0x21_interrupt,
    IRQ0x22_interrupt,
    IRQ0x23_interrupt,
    IRQ0x24_interrupt,
    IRQ0x25_interrupt,
    IRQ0x26_interrupt,
    IRQ0x27_interrupt,
    IRQ0x28_interrupt,
    IRQ0x29_interrupt,
    IRQ0x2a_interrupt,
    IRQ0x2b_interrupt,
    IRQ0x2c_interrupt,
    IRQ0x2d_interrupt,
    IRQ0x2e_interrupt,
    IRQ0x2f_interrupt,
    IRQ0x30_interrupt,
    IRQ0x31_interrupt,
    IRQ0x32_interrupt,
    IRQ0x33_interrupt,
    IRQ0x34_interrupt,
    IRQ0x35_interrupt,
    IRQ0x36_interrupt,
    IRQ0x37_interrupt,
};

void init_interrupt()
{
    int i;
    // 32 - 55對應0x20-0x0x37。
    for (i = 32;i < 56; ++i) {
        set_intr_gate(i, 2, interrupt[i - 32]);
        // i表示是第i號idt描述符，2表示使用tss的isr2，interrupt[i - 32]為處理函式的地址。
    }

    color_printk(RED, BLACK, "8259A init \n");
    //8259A-master	ICW1-4
    io_out8(0x20, 0x11); // 0x11表示啟用ICW4 (BIT4為ICW，BIT0為啟用ICW4)
    io_out8(0x21, 0x20); // 表示中斷向量從0x20開始(BIT0-BIT2必須為0)
    io_out8(0x21, 0x04); // 表示IRQ2聯集從8259A晶片
    io_out8(0x21, 0x01); // 表示AEOI模式會自動復位ISR，不須透過CPU發出EOI指令。

    //8259A-slave	ICW1-4
    io_out8(0xa0, 0x11);
    io_out8(0xa1, 0x28);
    io_out8(0xa1, 0x02); // 表示銜接到IRQ2的引腳
    io_out8(0xa1, 0x01);

    //8259A-M/S	OCW1
    io_out8(0x21,0xfd); // 把IRQ1以外的中段屏蔽
    io_out8(0xa1,0xff);
    sti();
}

void do_IRQ(unsigned long regs, unsigned long nr)	//regs:rsp,nr
{
    unsigned char x;
    color_printk(RED, BLACK, "do_IRQ:%#08x\t", nr);
    x = io_in8(0x60);
    color_printk(RED, BLACK, "key code:%#08x\n", x);
    io_out8(0x20, 0x20);
}
