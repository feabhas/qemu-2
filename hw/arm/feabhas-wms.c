#include "feabhas-wms.h"
#include "hw/boards.h"
#include "hw/ssi.h"

#include "sysemu/sysemu.h"

static FeabhasWMSConfig s_board_config_silk_bb = {
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
    .ss_value = 0,
};

static void sevensegment_irq_handler(void *opaque, int n, int level)
{
    FeabhasWMSConfig *s = (FeabhasWMSConfig *)opaque;

    // printf("n:%d l:%d\n", n, level);

    switch (n)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    {
        if (level == 0)
        {
            s->ss_value &= ~(1 << n);
        }
        else
        {
            s->ss_value |= (1 << n);
        }
        if (s->ss_value < 10)
        {
            printf("7-Segment: %u\n", s->ss_value);
        }
        else if (s->ss_value == 15)
        {
            puts("7-Segment: IS OFF");
        }
        else
        {
            ; // do nothing
        }
    }
    break;
    case 4:
        printf("Motor is %s\n", !level ? "OFF" : "ON");
        break;
    case 5:
        printf("Motor is now set to %s\n", !level ? "Clockwise " : "Anti-clockise");
        break;
    case 6:
        printf("Latch is %s\n", !level ? "LOW" : "HIGH");
        // s->latch = (bool)level;
        break;
    case 7:
        printf("Buzzer is %s\n", !level ? "OFF" : "ON");
        break;
    default:
        break;
    }
}

static void feabhas_32f412_init(MachineState *machine,
                                const FeabhasWMSConfig *board_config)
{
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
    qemu_irq *wms_op_irq = qemu_allocate_irqs(sevensegment_irq_handler, (void *)board_config, 8);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 8, wms_op_irq[0]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 9, wms_op_irq[1]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 10, wms_op_irq[2]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 11, wms_op_irq[3]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 12, wms_op_irq[4]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 13, wms_op_irq[5]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 14, wms_op_irq[6]);
    qdev_connect_gpio_out((DeviceState *)gpio[STM32_GPIOD_INDEX], 15, wms_op_irq[7]);

    // connect UART3 to qemu serial port 0
    // this allows only one -serial to connect to this device
    stm32_uart_connect((Stm32Uart *)uart[2], serial_hds[0],
                       STM32_USART3_NO_REMAP);
}

static void feabhas_init(MachineState *machine)
{
    feabhas_32f412_init(machine, &s_board_config_silk_bb);
}

static void feabhas_wms_machine_init(MachineClass *mc)
{
    mc->desc = "Feabhas Washing Machine Simulator";
    mc->init = feabhas_init;
}

DEFINE_MACHINE("feabhas-wms", feabhas_wms_machine_init)
