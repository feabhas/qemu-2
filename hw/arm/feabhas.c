#include "feabhas.h"
#include "hw/boards.h"
#include "hw/ssi.h"

const static FeabhasBoardConfig s_board_config_silk_bb = {
    // .dbgserial_uart_index = 0, // USART1
    // .feabhas_control_uart_index =
    //     1, // USART2 -- note this is also used by the DA14681 on real HW
    // .button_map =
    //     {
    //         {STM32_GPIOC_INDEX, 13, true},
    //         {STM32_GPIOD_INDEX, 2, true},
    //         {STM32_GPIOH_INDEX, 0, true},
    //         {STM32_GPIOH_INDEX, 1, true},
    //     },
    .gpio_idr_masks =
        {
            // [STM32_GPIOC_INDEX] = 1 << 13,
            [STM32_GPIOD_INDEX] = 0x7f,
            // [STM32_GPIOH_INDEX] = (1 << 1) | (1 << 0),
        },
    .flash_size =
        4096,        /* Kbytes - larger to aid in development and debugging */
    .ram_size = 256, /* Kbytes */
};

static const char *LED_ID[] = {"D6", "D5", "D4", "D3"};
static void led_irq_handler(void *opaque, int n, int level) {
  switch (level) {
  case 0:
    printf("%d:%s Off\n", n, LED_ID[n]);
    break;
  case 1:
    printf("%d:%s On\n", n, LED_ID[n]);
    break;
  }
}

static void feabhas_32f412_init(MachineState *machine,
                                const FeabhasBoardConfig *board_config) {
  Stm32Gpio *gpio[STM32F4XX_GPIO_COUNT];
  Stm32Uart *uart[STM32F4XX_UART_COUNT];
  Stm32Timer *timer[STM32F4XX_TIM_COUNT];
  DeviceState *rtc_dev;
  struct stm32f4xx stm;
  ARMCPU *cpu;

  // Note: allow for bigger flash images (4MByte) to aid in development and
  // debugging
  stm32f4xx_init(board_config->flash_size, board_config->ram_size,
                 machine->kernel_filename, gpio, board_config->gpio_idr_masks,
                 uart, timer, &rtc_dev, 8000000 /*osc_freq*/,
                 32768 /*osc2_freq*/, &stm, &cpu);

  // Set the feabhas specific QEMU settings on the target
  //   pebble_set_qemu_settings(rtc_dev);

  /* Connect LED to GPIO D pin 8 & 9 */
  qemu_irq *led_irq = qemu_allocate_irqs(led_irq_handler, NULL, 4);
  qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 8, led_irq[0]);
  qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 9, led_irq[1]);
  qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 10, led_irq[2]);
  qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 11, led_irq[3]);
}

static void feabhas_init(MachineState *machine) {
  feabhas_32f412_init(machine, &s_board_config_silk_bb);
}

static void feabhas_machine_init(MachineClass *mc) {
  mc->desc = "Feabhas Training Board";
  mc->init = feabhas_init;
}

DEFINE_MACHINE("feabhas", feabhas_machine_init)
