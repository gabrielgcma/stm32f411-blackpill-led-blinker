#include <stdint.h>
#include <stdlib.h>

// Peripherals base addresses
#define GPIOC_BASE_ADDRESS 0x40020800
#define RCC_BASE_ADDRESS 0x40023800

// Register offset values
#define RCC_AHB1ENR_OFFSET 0x30
#define GPIOC_MODER_OFFSET 0x00
#define GPIOC_OTYPER_OFFSET 0x04
#define GPIOC_PUPDR_OFFSET 0x0C
#define GPIOC_BSRR_OFFSET 0X18

// Register addresses
#define RCC_AHB1ENR (RCC_BASE_ADDRESS + RCC_AHB1ENR_OFFSET)
#define GPIOC_MODER (GPIOC_BASE_ADDRESS + GPIOC_MODER_OFFSET)
#define GPIOC_OTYPER (GPIOC_BASE_ADDRESS + GPIOC_OTYPER_OFFSET)
#define GPIOC_PUPDR (GPIOC_BASE_ADDRESS + GPIOC_PUPDR_OFFSET)
#define GPIOC_BSRR (GPIOC_BASE_ADDRESS + GPIOC_BSRR_OFFSET)

// GPIO port modes:
#define GPIO_INPUT_MODE (0)  // Input (reset state)
#define GPIO_OUTPUT_MODE (1) // General Purpose Output Mode
#define GPIO_ALT_MODE (2)    // Alternate function mode
#define GPIO_ANALOG_MODE (3) // Analog mode

// GPIO output types:
#define GPIO_OT_PP (0) // Push-pull output
#define GPIO_OT_OD (1) // Open-drain output

// GPIO pull-up/pull-down modes:
#define GPIO_PUPDR_NONE (0)
#define GPIO_PUPDR_PULLUP (1)
#define GPIO_PUPDR_PULLDOWN (2)

#define GPIO_MODER_SHIFT_AMOUNT(pin_number) (pin_number * 2)
#define GPIO_OT_SHIFT_AMOUNT(pin_number) (pin_number)
#define GPIO_PUPDR_SHIFT_AMOUNT(pin_number) (pin_number * 2)

// Masks:
/*
    Usa-se o número binário 11 (decimal 3) porque temos um par
de bits de configuração para setar no MODER referente ao pino desejado, e após o
shift aqui definido, vamos negar a máscara para que os bits
setados como 11 sejam resetados para 00 e assim possamos aplicar um OU lógico e
assim configurar o par de bits da forma desejada, de acordo com o modo de saída
do pino desejado.
*/
#define GPIO_MODER_MASK(pin_number) (3 << GPIO_MODER_SHIFT_AMOUNT(pin_number))
#define GPIO_OTYPER_MASK(pin_number) (1 << pin_number)
#define GPIO_PUPDR_MASK(pin_number) (3 << GPIO_PUPDR_SHIFT_AMOUNT(pin_number))

// Port enable
#define RCC_AHB1ENR_GPIOCEN (1 << 2)

#define LED_DELAY (500000)

#define GPIO_BSRR_SET(pin_number) (1 << (pin_number))
#define GPIO_BSRR_RESET(pin_number) (1 << (pin_number + 16))

int main(void) {
  // Para piscar o LED, que está ligado no pino PC13, devemos ativar então o
  // GPIOC.

  /* 1. Habilitar o GPIOC no "RCC AHB1 peripheral clock enable register"
   * (RCC_AHB1ENR).
   *
   * ENR = Enable Register, ou seja, o registrador que faz enable
   * de clock dos pinos do AHB1, por exemplo o PC13 que está na porta C do GPIO
   * (GPIOC). Ativando o clock de uma porta, ela pode ser usada.
   *
   *  O bit 2 desse registrador é o "GPIOCEN", ou seja, GPIOC Enable. Devemos
   * setar ele para 1 para ligar a porta C, de acordo com o manual.
   * */

  uint32_t *rcc_ahb1enr = (uint32_t *)RCC_AHB1ENR;
  *rcc_ahb1enr =
      *rcc_ahb1enr |
      RCC_AHB1ENR_GPIOCEN; // Pegando o 1 em binário e deslocando todos os seus
                           // bits 2 vezes para a esquerda: 0x001 << 2 = 0x100
                           // deixando o bit 3 com o valor de 1 como
                           // queríamos. Dessa maneira, os outros bits do
                           // registrador são mantidos com seus valores
                           // originais, já que X OR 0 = X

  /* 2. Configurar PC13 como modo saída (output mode)
   *
   *    Para configurar o pino como saída, segundo o manual, precisamos acessar
   * o registro GPIOX_MODER, ou seja, o registrador que define o modo de
   * operação da porta GPIOX (X = A...E)
   *
   *    Esse registrador é dividido para comportar a configuração do modo de
   * todos os 15 pinos de cada porta, começando com o pino 15 nos 2 bits mais
   * significativos e decrementando até o pino 0 no 2 bits menos significativos.
   *
   *    Segundo o manual, o modo de saída é setado através do valor 01 nos
   * bits. O pino 13 está nos bits 27 e 26 do registrador. Para acessá-los,
   * portanto, é necessário realizar um cálculo: percebe-se que há uma relação
   * de dobro entre o primeiro bit do par de bits do modo do pino, e o número do
   * pino. Logo, para setar o par de bits da configuração do modo do pino N,
   * precisamos deslocar o par de bits 2N vezes para a esquerda:
   */

  uint32_t *gpioc_moder = (uint32_t *)GPIOC_MODER;

  /*  GPIO_MODER_MASK(13) = (3 << 26) = 00001100000000000000000000000000
   * ~GPIO_MODER_MASK(13)             = 11110011111111111111111111111111
   */

  *gpioc_moder = *gpioc_moder & ~GPIO_MODER_MASK(13);

  /*  gpioc_moder                     = xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   * ~GPIO_MODER_MASK                 = 11110011111111111111111111111111
   *
   *  gpioc_moder & ~GPIO_MODER_MASK  = xxxx00xxxxxxxxxxxxxxxxxxxxxxxxxx
   *
   *  Pois 1 & X = X, 0 & X = 0.
   */

  *gpioc_moder =
      *gpioc_moder | (GPIO_OUTPUT_MODE << GPIO_MODER_SHIFT_AMOUNT(13));

  /*  gpioc_moder                           = xxxx00xxxxxxxxxxxxxxxxxxxxxxxxxxx
   *  GPIO_OUTPUT_MODE << 26                = 000001000000000000000000000000000
   *
   *  gpioc_moder | (GPIO_OUTPUT_MODE<<26)  = xxxx01xxxxxxxxxxxxxxxxxxxxxxxxxxx
   *
   *  Pois 0 OR X = X, 0 OR 1 = 1.
   */

  /* 3. Configurar o PC13 com tipo de saída push-pull (output type) */

  uint32_t *gpioc_otyper = (uint32_t *)GPIOC_OTYPER;

  /* GPIO_OTYPER_MASK = (1 << 13) = 00000000000000000010000000000000
    ~GPIO_OTYPER_MASK             = 11111111111111111101111111111111
  */

  *gpioc_otyper = *gpioc_otyper & ~GPIO_OTYPER_MASK(13);

  /*  gpioc_otyper                        = xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   * ~GPIO_OTYPER_MASK                    = 11111111111111111101111111111111

   *  gpioc_otyper & ~GPIO_OTYPER_MASK    = xxxxxxxxxxxxxxxxxx0xxxxxxxxxxxxx
   *
   *  Pois 1 & X = X, 0 & X = 0.
   */

  *gpioc_otyper = *gpioc_otyper | (GPIO_OT_PP << GPIO_OT_SHIFT_AMOUNT(13));

  /*  gpioc_otyper                        = xxxxxxxxxxxxxxxxxx0xxxxxxxxxxxxx
   *  GPIO_OT_PP << 13                    = 00000000000000000000000000000000
   *
   *  gpioc_otyper | (GPIO_OT_PP << 13)   = xxxxxxxxxxxxxxxxxx0xxxxxxxxxxxxx
   *
   *  Pois 0 OR X = X.
   */

  /* 4. Configurar o PC13 sem pull-up e sem pull-down (PU/PD Register) */

  uint32_t *gpioc_pupdr = (uint32_t *)GPIOC_PUPDR;

  /*  GPIO_PUPDR_MASK(13) = (3 << 26) = 00001100000000000000000000000000
   * ~GPIO_PUPDR_MASK(13)             = 11110011111111111111111111111111
   */

  *gpioc_pupdr = *gpioc_pupdr & ~GPIO_PUPDR_MASK(13);

  /*  gpioc_pupdr                     = xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   * ~GPIO_PUPDR_MASK                 = 11110011111111111111111111111111
   *
   *  gpioc_pupdr & ~GPIO_PUPDR_MASK  = xxxx00xxxxxxxxxxxxxxxxxxxxxxxxxx
   *
   *  Pois 1 & X = X, 0 & X = 0.
   */

  *gpioc_pupdr =
      *gpioc_pupdr | (GPIO_PUPDR_NONE << GPIO_PUPDR_SHIFT_AMOUNT(13));

  /*  gpioc_pupdr                           = xxxx00xxxxxxxxxxxxxxxxxxxxxxxxxxx
   *  GPIO_PUPDR_NONE << 26                 = 000000000000000000000000000000000
   *
   *  gpioc_pupdr | (GPIO_PUPDR_NONE<<26)   = xxxx00xxxxxxxxxxxxxxxxxxxxxxxxxxx
   *
   *  Pois 0 OR X = X, 0 OR 1 = 1.
   */

  /* 5. Ligar LED (liga com LOW na saída, desliga com HIGH na saída) */

  while (1) {
    uint32_t i;
    uint32_t *gpioc_bsrr = (uint32_t *)GPIOC_BSRR;

    // Liga o LED (LOW):
    *gpioc_bsrr =
        GPIO_BSRR_RESET(13); // Pode-se setar diretamente porque o shift deixa
                             // todos os outros bits com valor 0, e o valor 0 no
                             // BSSR é ignorado (nenhuama ação é conduzida)
    for (i = 0; i < LED_DELAY; i++)
      ;

    // Desliga o LED (HIGH):
    *gpioc_bsrr = GPIO_BSRR_SET(13);
    for (i = 0; i < LED_DELAY; i++)
      ;
  }

  /* Nao deveria chegar aqui */

  return EXIT_SUCCESS;
}
