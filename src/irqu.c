#include <seven_irq.h>
#include <string.h>

#define N_IRQ 14

typedef void IrqHandlerFunc();

struct IRQ
{
    u32 data;
    IrqHandlerFunc *func;
};

// Invariants:
// Valid slots must have LOHWORD(data) != 0
// Slots must be sorted by ascending priority
// Slots must have a valid function pointer
extern struct IRQ IRQ_TABLE[N_IRQ];

static u16 IRQS_IN_TABLE = 0;

extern void irqAddHandler(u16 irqs, IrqHandlerFunc *func, u16 priority)
{
    u32 old = irqDisableIME();

    struct IRQ *i = IRQ_TABLE;
    struct IRQ  t;

    irqs &= 0x3FFF;

    if (!irqs || !func)
    {
        goto out;
    }

    if (IRQS_IN_TABLE & irqs)
    {
        goto out;
    }

    // Search logic
    //
    // A slot is one of: Unused, Eligible, or Used
    //
    // A slot is unused, if:
    //   its IRQ flags are 0
    //
    // A slot is eligible, if:
    //   it has the same priority and handler function
    //
    //   this means:
    //       irqAddHandler(IRQ_DMA_0, dmaHandler, 0);
    //       irqAddHandler(IRQ_DMA_1, dmaHandler, 0);
    //
    //   form one IRQ entry with
    //   priority: 0, irqs: DMA_0, DMA_1, func: dmaHandler
    //
    // A slot is used, if:
    //   its IRQ flags match ones present in `irqs`.
    while (true)
    {
        t = *i;

        // Slot eligible for merging
        if (t.data >> 16 == priority && t.func == func)
        {
            IRQS_IN_TABLE |= irqs;
            t.data |= irqs;

            *i = t;
            break;
        }

        // Slot can be inserted here
        //
        // (The only circumstance under which we find an empty slot is
        //  if we didn't find any inhabited slot with a greater priority,
        //  meaning said empty slot works as the insertion spot)
        if ((t.data & 0xFFFF) == 0 || t.data >> 16 > priority)
        {
            IRQS_IN_TABLE |= irqs;
            t.data = irqs | (priority << 16);
            t.func = func;

            // This math is a little funky.
            //
            // We need to shift everything from the current slot
            // to the end of the table back by 1 slot
            //
            // Because we don't wanna shift things past the end of the table
            // we need to subtract 1 slot from the byte count
            //
            // This is fine because once the table is full we won't do any more
            // shifts, so we can't clobber anything.
            u32 bytes = sizeof(IRQ_TABLE) - sizeof(struct IRQ) - ((u32)i - (u32)IRQ_TABLE);

            // TODO: Implement memmove but better for this purpose
            memmove(i + 1, i, bytes);
            *i = t;

            break;
        }

        i++;
    }

out:
    REG_IME = old;
}

static struct IRQ *irqFindEntry(u16 irqs)
{
    for (struct IRQ *i = IRQ_TABLE; i->data; i++)
    {
        if (i->data & irqs)
        {
            return i;
        }
    }

    return NULL;
}

extern void irqRemoveHandler(u16 irqs)
{
    u32 old = irqDisableIME();

    struct IRQ *i = irqFindEntry(irqs);
    struct IRQ  t;

    if (i)
    {
        t = *i;

        t.data &= ~irqs;

        // if slot is now empty, throw it out
        if (!(t.data & 0xFFFF))
        {
            u32 bytes = sizeof(IRQ_TABLE) - ((u32)i - (u32)IRQ_TABLE)

            memmove(i, i+1, bytes);
        }
    }

    REG_IME = old;
}
