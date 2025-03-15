// See MultiProcessor Specification Version 1.[14]
// 在启动时，系统需要检测是否支持多处理器，并配置各个CPU。`mp`结构体帮助系统找到配置表，从而初始化APIC和处理器信息。
struct mp {             // floating pointer
  // 标识符
  uchar signature[4];           // "_MP_"
  // MP配置表的物理地址（关键指针）
  void *physaddr;               // phys addr of MP config table
  // 浮动指针结构长度（单位：16字节）
  uchar length;                 // 1
  // MP规范版本（1.4版本值为0x04）
  uchar specrev;                // [14]
  // 校验和（所有字节相加为0）
  uchar checksum;               // all bytes must add up to 0
  // 配置类型（0表示默认）
  uchar type;                   // MP system config type
  // IMCR标志（APIC模式控制）
  uchar imcrp;
  // 保留字段
  uchar reserved[3];
};

// mpconf 是 多处理器配置表头 ，用于描述系统硬件拓扑结构。
struct mpconf {         // configuration table header
  // 标识符 "PCMP"
  uchar signature[4];           // "PCMP"
  // 整个配置表的总长度（包括所有条目）
  ushort length;                // total table length
  // MP规范版本（1.4版本值为0x01）
  uchar version;                // [14]
  // 校验和（所有字节相加为0）
  uchar checksum;               // all bytes must add up to 0
  // 产品ID字符串（主板/BIOS信息）
  uchar product[20];            // product id
  // OEM特定配置表的指针
  uint *oemtable;               // OEM table pointer
  // OEM表长度
  ushort oemlength;             // OEM table length
  // 配置表条目总数（CPU/APIC等）
  ushort entry;                 // entry count
  // 本地APIC的默认物理地址（关键字段）
  uint *lapicaddr;              // address of local APIC
  // 扩展表长度
  ushort xlength;               // extended table length
  // 扩展表校验和
  uchar xchecksum;              // extended table checksum
  // 保留字段
  uchar reserved;
};

// mpproc 是 处理器配置条目 ，用于描述系统中每个CPU的核心信息。
struct mpproc {         // processor table entry
  // 条目类型（固定为MPPROC=0x00）
  uchar type;                   // entry type (0)
  // 本地APIC ID（用于中断路由）
  uchar apicid;                 // local APIC id
  // APIC硬件版本号
  uchar version;                // local APIC verison
  // CPU特性标志
  uchar flags;                  // CPU flags
    #define MPBOOT 0x02           // This proc is the bootstrap processor.
  // CPU签名（标识厂商）
  uchar signature[4];           // CPU signature
  // CPU特性（来自CPUID指令）
  uint feature;                 // feature flags from CPUID instruction
  // 保留字段
  uchar reserved[8];
};

// mpioapic 是 I/O APIC配置条目 ，用于描述系统中的I/O高级可编程中断控制器信息。
struct mpioapic {       // I/O APIC table entry
  // 条目类型（固定为MPIOAPIC=0x02）
  uchar type;                   // entry type (2)
  // I/O APIC硬件ID（用于中断路由）
  uchar apicno;                 // I/O APIC id
  // I/O APIC硬件版本号
  uchar version;                // I/O APIC version
  // 状态标志（通常表示是否可用）
  uchar flags;                  // I/O APIC flags
  // I/O APIC的物理地址（关键字段）
  uint *addr;                  // I/O APIC address
};

// Table entry types
#define MPPROC    0x00  // One per processor
#define MPBUS     0x01  // One per bus
#define MPIOAPIC  0x02  // One per I/O APIC
#define MPIOINTR  0x03  // One per bus interrupt source
#define MPLINTR   0x04  // One per system interrupt source

//PAGEBREAK!
// Blank page.
