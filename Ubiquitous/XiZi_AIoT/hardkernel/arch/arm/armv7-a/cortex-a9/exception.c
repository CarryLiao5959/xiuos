
/**
 * this function will show registers of CPU
 *
 * @param regs the registers point
 */
void PrintStackFrame(struct rt_hw_exp_stack *regs)
{
    rt_kprintf("Execption:\n");
    rt_kprintf("r00:0x%08x r01:0x%08x r02:0x%08x r03:0x%08x\n", regs->r0, regs->r1, regs->r2, regs->r3);
    rt_kprintf("r04:0x%08x r05:0x%08x r06:0x%08x r07:0x%08x\n", regs->r4, regs->r5, regs->r6, regs->r7);
    rt_kprintf("r08:0x%08x r09:0x%08x r10:0x%08x\n", regs->r8, regs->r9, regs->r10);
    rt_kprintf("fp :0x%08x ip :0x%08x\n", regs->fp, regs->ip);
    rt_kprintf("sp :0x%08x lr :0x%08x pc :0x%08x\n", regs->sp, regs->lr, regs->pc);
    rt_kprintf("cpsr:0x%08x\n", regs->cpsr);
}


/**
 * The software interrupt instruction (SWI) is used for entering
 * Supervisor mode, usually to request a particular supervisor
 * function.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void rt_hw_trap_swi(struct rt_hw_exp_stack *regs)
{

}

void rt_hw_trap_irq(void)
{
    void *param;
    rt_isr_handler_t isr_func;
    extern struct rt_irq_desc isr_table[];

    // vectNum = RESERVED[31:13] | CPUID[12:10] | INTERRUPT_ID[9:0]
    // send ack and get ID source
    uint32_t vectNum = gic_read_irq_ack();

    // Check that INT_ID isn't 1023 or 1022 (spurious interrupt)
    if (vectNum & 0x0200)
    {
        gic_write_end_of_irq(vectNum);  // send end of irq
    }
    else
    {
        // copy the local value to the global image of CPUID
        unsigned cpu = (vectNum >> 10) & 0x7;
        unsigned irq = vectNum & 0x1FF;

        /* skip warning */
        cpu = cpu;

        // Call the service routine stored in the handlers array. If there isn't
        // one for this IRQ, then call the default handler.
        /* get interrupt service routine */
        isr_func = isr_table[irq].handler;
#ifdef RT_USING_INTERRUPT_INFO
        isr_table[irq].counter++;
#endif
        if (isr_func)
        {
            /* Interrupt for myself. */
            param = isr_table[irq].param;
            /* turn to interrupt service routine */
            isr_func(irq, param);
        }

        // Signal the end of the irq.
        gic_write_end_of_irq(vectNum);
    }
}
