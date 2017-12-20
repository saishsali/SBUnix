#ifndef _PAGE_FAULT_H
#define _PAGE_FAULT_H

#define PF_P 1

void page_fault_exception(stack_registers *registers);

#endif
