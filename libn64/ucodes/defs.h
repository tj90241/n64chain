; RSP CP0 registers
.set DMA_CACHE,             $0
.set DMA_DRAM,              $1
.set DMA_READ_LENGTH,       $2
.set DMA_WRITE_LENGTH,      $3
.set SP_STATUS,             $4
.set DMA_FULL,              $5
.set DMA_BUSY,              $6
.set SP_RESERVED,           $7
.set CMD_START,             $8
.set CMD_END,               $9
.set CMD_CURRENT,           $10
.set CMD_STATUS,            $11
.set CMD_CLOCK,             $12
.set CMD_BUSY,              $13
.set CMD_PIPE_BUSY,         $14
.set CMD_TMEM_BUSY,         $15

; SP_STATUS write flags
.set CLEAR_HALT,            0x1
.set SET_HALT,              0x2
.set CLEAR_BROKE,           0x4
.set CLEAR_RSP_INTERRUPT,   0x8
.set SET_RSP_INTERRUPT,     0x10
.set CLEAR_SINGLE_STEP,     0x20
.set SET_SINGLE_STEP,       0x40
.set CLEAR_INTR_ON_BREAK,   0x80
.set SET_INTR_ON_BREAK,     0x100

; CMD_STATUS write flags
.set CLEAR_XBUS_DMEM_DMA,   0x1
.set SET_XBUS_DMEM_DMA,     0x2
.set CLEAR_FREEZE,          0x4
.set SET_FREEZE,            0x8
.set CLEAR_FLUSH,           0x10
.set SET_FLUSH,             0x20
.set CLEAR_TMEM_COUNTER,    0x40
.set CLEAR_PIPE_COUNTER,    0x80
.set CLEAR_CMD_COUNTER,     0x100
.set CLEAR_CLOCK_COUNTER,   0x200

