#ifndef __UART0_CONFIG_H__
#define __UART0_CONFIG_H__

#define GPIO_PORTA_AFSEL_R  (reg_t(0x40004420)) // GPIOA Alternate Function Select Register
#define GPIO_PORTA_DEN_R    (reg_t(0x4000451C)) // GPIOA Digital Enable Register
#define UART0_DR_R          (reg_t(0x4000C000)) // UART0 Data Register
#define UART0_IBRD_R        (reg_t(0x4000C024)) // UART0 Integer Baud-Rate Divisor Register
#define UART0_FBRD_R        (reg_t(0x4000C028)) // UART0 Fractional Baud-Rate Divisor Register
#define UART0_LCRH_R        (reg_t(0x4000C02C)) // UART0 Line Control Register
#define UART0_CTL_R         (reg_t(0x4000C030)) // UART0 Control Register
#define UART0_IM_R          (reg_t(0x4000C038)) // UART0 Interrupt Mask Register
#define UART0_MIS_R         (reg_t(0x4000C040)) // UART0 Masked Interrupt Status Register
#define UART0_ICR_R         (reg_t(0x4000C044)) // UART0 Interrupt Clear Register

#define UART_LCRH_WLEN_8    0x00000060  // 8 bit word length
#define UART_CTL_UARTEN     0x00000301  // UART RX/TX Enable
#define UART_INT_TX         0x020       // Transmit Interrupt Mask
#define UART_INT_RX         0x010       // Receive Interrupt Mask
#define UART_INT_RT         0x040       // Receive Timeout Interrupt Mask
#define EN_RX_PA0           0x00000001  // Enable Receive Function on PA0
#define EN_TX_PA1           0x00000002  // Enable Transmit Function on PA1
#define EN_DIG_PA0          0x00000001  // Enable Digital I/O on PA0
#define EN_DIG_PA1          0x00000002  // Enable Digital I/O on PA1

// Clock Gating Registers
#define SYSCTL_RCGC1_R      (reg_t(0x400FE104))
#define SYSCTL_RCGC2_R      (reg_t(0x400FE108))

#define SYSCTL_RCGC1_UART0  0x00000001  // UART0 Clock Gating Control
#define SYSCTL_RCGC2_GPIOA  0x00000001  // Port A Clock Gating Control

#define NVIC_EN0_R          (reg_t(0xE000E100))	// Interrupt 0-31 Set Enable Register

#define UART0_INTVECT      5

#endif
