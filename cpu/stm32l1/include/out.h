#define USART2_RX_A_3_TX_A_2 \
{    .dev = USART2, \
     .rcc_mask = RCC_APB1ENR_USART1EN, \
     .bus = APB1, \
     .irqn = USART2_IRQn, \
     .tx_pin = GPIO_PIN(PORT_A,2), \
     .rx_pin = GPIO_PIN(PORT_A,3), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART1_RX_A_10_TX_A_9 \
{    .dev = USART1, \
     .rcc_mask = RCC_APB2ENR_USART1EN, \
     .bus = APB2, \
     .irqn = USART1_IRQn, \
     .tx_pin = GPIO_PIN(PORT_A,9), \
     .rx_pin = GPIO_PIN(PORT_A,10), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART1_RX_B_7_TX_B_6 \
{    .dev = USART1, \
     .rcc_mask = RCC_APB2ENR_USART1EN, \
     .bus = APB2, \
     .irqn = USART1_IRQn, \
     .tx_pin = GPIO_PIN(PORT_B,6), \
     .rx_pin = GPIO_PIN(PORT_B,7), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART3_RX_B_11_TX_B_10 \
{    .dev = USART3, \
     .rcc_mask = RCC_APB1ENR_USART1EN, \
     .bus = APB1, \
     .irqn = USART3_IRQn, \
     .tx_pin = GPIO_PIN(PORT_B,10), \
     .rx_pin = GPIO_PIN(PORT_B,11), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART3_RX_C_11_TX_C_10 \
{    .dev = USART3, \
     .rcc_mask = RCC_APB1ENR_USART1EN, \
     .bus = APB1, \
     .irqn = USART3_IRQn, \
     .tx_pin = GPIO_PIN(PORT_C,10), \
     .rx_pin = GPIO_PIN(PORT_C,11), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART2_RX_D_6_TX_D_5 \
{    .dev = USART2, \
     .rcc_mask = RCC_APB1ENR_USART1EN, \
     .bus = APB1, \
     .irqn = USART2_IRQn, \
     .tx_pin = GPIO_PIN(PORT_D,5), \
     .rx_pin = GPIO_PIN(PORT_D,6), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

#define USART3_RX_D_9_TX_D_8 \
{    .dev = USART3, \
     .rcc_mask = RCC_APB1ENR_USART1EN, \
     .bus = APB1, \
     .irqn = USART3_IRQn, \
     .tx_pin = GPIO_PIN(PORT_D,8), \
     .rx_pin = GPIO_PIN(PORT_D,9), \
     .tx_af = GPIO_AF7, \
     .rx_af = GPIO_AF7 }

