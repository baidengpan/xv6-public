#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
// end 在 kernel.ld 中通过 PROVIDE(end = .) 定义，它是内核加载后的第一个地址。
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  /**
   * 物理页分配器初始化
   */
  // 初始化物理页分配器， end 是内核加载后的第一个地址， P2V(4*1024*1024) 是将物理地址 4MB 转换为虚拟地址，用于指定物理页分配的起始和结束范围。
  kinit1(end, P2V(4*1024*1024)); // phys page allocator

  /**
   * 内核页表初始化
   */
  // 初始化内核页表，为内核代码和数据分配虚拟地址空间。
  kvmalloc();      // kernel page table

  /**
   * 多处理器检测
   */
  // 检测系统中的其他处理器，为后续的多处理器启动做准备。
  mpinit();        // detect other processors

  /**
   * 中断控制器初始化
   */
  // 初始化本地 APIC（Advanced Programmable Interrupt Controller），用于处理本地处理器的中断。
  lapicinit();     // interrupt controller
  // 初始化段描述符，设置处理器的段寄存器。
  seginit();       // segment descriptors
  // 禁用旧的可编程中断控制器（PIC），因为现代系统通常使用更先进的 APIC 或 IOAPIC。
  picinit();       // disable pic
  // 初始化 IOAPIC（I/O Advanced Programmable Interrupt Controller），用于处理外部设备的中断。
  ioapicinit();    // another interrupt controller

  /**
   * 硬件设备初始化
   */
  // 初始化控制台硬件，用于系统的输入输出。
  consoleinit();   // console hardware
  // 初始化串口，用于调试和通信。
  uartinit();      // serial port

  /**
   * 进程表和陷阱向量初始化
   */
  // 初始化进程表，用于管理系统中的进程。
  pinit();         // process table
  // 初始化陷阱向量，设置中断和异常处理程序的入口地址。
  tvinit();        // trap vectors

  /**
   * 其他系统组件初始化
   */
  // 初始化缓冲区缓存，用于磁盘 I/O 的缓存管理。
  binit();         // buffer cache
  // 初始化文件表，用于管理文件系统中的文件。
  fileinit();      // file table
  // 初始化磁盘设备，为后续的磁盘 I/O 操作做准备。
  ideinit();       // disk 

  /**
   * 启动其他处理器
   */
  // 启动系统中的其他处理器，让它们开始执行 mpenter 函数。
  startothers();   // start other processors

  /**
   * 物理页分配器二次初始化
   */
  // 在其他处理器启动后，对物理页分配器进行二次初始化，扩大物理页分配的范围。
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()

  /**
   * 创建第一个用户进程
   */
  // 创建系统中的第一个用户进程，标志着系统开始进入用户态运行。
  userinit();      // first user process

  /**
   * 完成当前处理器的设置
   */
  // 完成当前处理器的设置，包括加载中断描述符表（IDT），启动调度器开始运行进程。
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

