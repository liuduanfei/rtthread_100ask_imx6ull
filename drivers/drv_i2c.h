
#ifndef __DRV_I2C_H__

#define __DRV_I2C_H__

/* i2cr */
#define IEN    (0x1 << 7)   /* 2C enable */
#define IIEN   (0x1 << 6)   /* I2C interrupt enable */
#define MSTA   (0x1 << 5)   /* Master/Slave mode select bit */
#define MTX    (0x1 << 4)   /* Transmit/Receive mode select bit */
#define TXAK   (0x1 << 3)   /* Transmit acknowledge enable */
#define RSTA   (0x1 << 2)   /* Repeat start */

/* i2sr */
#define ICF    (0x1 << 7)   /* Data transferring bit */
#define IAAS   (0x1 << 6)   /* I2C addressed as a slave bi */
#define IBB    (0x1 << 5)   /* I2C bus busy bit */
#define IAL    (0x1 << 4)   /* Arbitration lost */
#define SRW    (0x1 << 2)   /* Slave read/write */
#define IIF    (0x1 << 1)   /* I2C interrupt */
#define RXAK   (0x1 << 0)   /* Received acknowledge */

#endif /* __DRV_I2C_H__ */
