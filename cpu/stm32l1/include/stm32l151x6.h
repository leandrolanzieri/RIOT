#define USART2_RX_A_3_TX_A_2 \
{   .dev      = USART2, \
    .rcc_mask = RCC_APB1ENR_USART1EN,    \
    .bus      = APB1,    \
    .irqn     = USART2_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_A, 2),  \
    .rx_pin   = GPIO_PIN(PORT_A, 3),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART1_RX_A_10_TX_A_9 \
{   .dev      = USART1, \
    .rcc_mask = RCC_APB2ENR_USART1EN,    \
    .bus      = APB2,    \
    .irqn     = USART1_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_A, 9),  \
    .rx_pin   = GPIO_PIN(PORT_A, 10),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART1_RX_B_7_TX_B_6 \
{   .dev      = USART1, \
    .rcc_mask = RCC_APB2ENR_USART1EN,    \
    .bus      = APB2,    \
    .irqn     = USART1_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_B, 6),  \
    .rx_pin   = GPIO_PIN(PORT_B, 7),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART3_RX_B_11_TX_B_10 \
{   .dev      = USART3, \
    .rcc_mask = RCC_APB1ENR_USART1EN,    \
    .bus      = APB1,    \
    .irqn     = USART3_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_B, 10),  \
    .rx_pin   = GPIO_PIN(PORT_B, 11),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART3_RX_C_11_TX_C_10 \
{   .dev      = USART3, \
    .rcc_mask = RCC_APB1ENR_USART1EN,    \
    .bus      = APB1,    \
    .irqn     = USART3_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_C, 10),  \
    .rx_pin   = GPIO_PIN(PORT_C, 11),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART2_RX_D_6_TX_D_5 \
{   .dev      = USART2, \
    .rcc_mask = RCC_APB1ENR_USART1EN,    \
    .bus      = APB1,    \
    .irqn     = USART2_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_D, 5),  \
    .rx_pin   = GPIO_PIN(PORT_D, 6),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define USART3_RX_D_9_TX_D_8 \
{   .dev      = USART3, \
    .rcc_mask = RCC_APB1ENR_USART1EN,    \
    .bus      = APB1,    \
    .irqn     = USART3_IRQn,   \
    .tx_pin   = GPIO_PIN(PORT_D, 8),  \
    .rx_pin   = GPIO_PIN(PORT_D, 9),  \
    .tx_af    = GPIO_AF7, \
    .rx_af    = GPIO_AF7 }

#define SPI1_MOSI_A_7_MISO_A_6(CS_PIN) \
{   .dev       = SPI1, \
    .rccmask   = RCC_APB2ENR_SPI1EN,    \
    .apbbus    = APB2,    \
    .mosi_pin  = GPIO_PIN(PORT_A, 7),  \
    .miso_pin  = GPIO_PIN(PORT_A, 6),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define SPI1_MOSI_A_12_MISO_A_11(CS_PIN) \
{   .dev       = SPI1, \
    .rccmask   = RCC_APB2ENR_SPI1EN,    \
    .apbbus    = APB2,    \
    .mosi_pin  = GPIO_PIN(PORT_A, 12),  \
    .miso_pin  = GPIO_PIN(PORT_A, 11),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define SPI1_MOSI_B_5_MISO_B_4(CS_PIN) \
{   .dev       = SPI1, \
    .rccmask   = RCC_APB2ENR_SPI1EN,    \
    .apbbus    = APB2,    \
    .mosi_pin  = GPIO_PIN(PORT_B, 5),  \
    .miso_pin  = GPIO_PIN(PORT_B, 4),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define SPI1_MOSI_E_15_MISO_E_14(CS_PIN) \
{   .dev       = SPI1, \
    .rccmask   = RCC_APB2ENR_SPI1EN,    \
    .apbbus    = APB2,    \
    .mosi_pin  = GPIO_PIN(PORT_E, 15),  \
    .miso_pin  = GPIO_PIN(PORT_E, 14),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define SPI2_MOSI_B_15_MISO_B_14(CS_PIN) \
{   .dev       = SPI2, \
    .rccmask   = RCC_APB1ENR_SPI2EN,    \
    .apbbus    = APB1,    \
    .mosi_pin  = GPIO_PIN(PORT_B, 15),  \
    .miso_pin  = GPIO_PIN(PORT_B, 14),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define SPI2_MOSI_D_3_MISO_D_4(CS_PIN) \
{   .dev       = SPI2, \
    .rccmask   = RCC_APB1ENR_SPI2EN,    \
    .apbbus    = APB1,    \
    .mosi_pin  = GPIO_PIN(PORT_D, 3),  \
    .miso_pin  = GPIO_PIN(PORT_D, 4),  \
    .cs_pin    = CS_PIN,                \
    .af        = GPIO_AF5 }

#define I2C1_SCL_B_6_SDA_B_7(SPEED) \
{   .dev        = I2C1,  \
    .speed      = SPEED,                   \
    .scl_pin    = GPIO_PIN(PORT_B, 6),  \
    .sda_pin    = GPIO_PIN(PORT_B, 7),  \
    .scl_af     = GPIO_AF4,         \
    .sda_af     = GPIO_AF4,         \
    .bus        = APB1,     \
    .rcc_mask   = RCC_APB1ENR_I2C1EN,     \
    .clk        = CLOCK_APB1,      \
    .irqn       = I2C1_EV_IRQn }

#define I2C1_SCL_B_8_SDA_B_9(SPEED) \
{   .dev        = I2C1,  \
    .speed      = SPEED,                   \
    .scl_pin    = GPIO_PIN(PORT_B, 8),  \
    .sda_pin    = GPIO_PIN(PORT_B, 9),  \
    .scl_af     = GPIO_AF4,         \
    .sda_af     = GPIO_AF4,         \
    .bus        = APB1,     \
    .rcc_mask   = RCC_APB1ENR_I2C1EN,     \
    .clk        = CLOCK_APB1,      \
    .irqn       = I2C1_EV_IRQn }

#define I2C2_SCL_B_10_SDA_B_11(SPEED) \
{   .dev        = I2C2,  \
    .speed      = SPEED,                   \
    .scl_pin    = GPIO_PIN(PORT_B, 10),  \
    .sda_pin    = GPIO_PIN(PORT_B, 11),  \
    .scl_af     = GPIO_AF4,         \
    .sda_af     = GPIO_AF4,         \
    .bus        = APB1,     \
    .rcc_mask   = RCC_APB1ENR_I2C2EN,     \
    .clk        = CLOCK_APB1,      \
    .irqn       = I2C2_EV_IRQn }


