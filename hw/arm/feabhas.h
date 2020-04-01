#include "stm32f4xx.h"
#include <qemu/typedefs.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  FEABHAS_BUTTON_ID_NONE = -1,
  FEABHAS_BUTTON_ID_BACK = 0,
  FEABHAS_BUTTON_ID_UP = 1,
  FEABHAS_BUTTON_ID_SELECT = 2,
  FEABHAS_BUTTON_ID_DOWN = 3,
  FEABHAS_NUM_BUTTONS = 4
} FeabhasButtonID;

typedef struct {
  int gpio;
  int pin;
  bool active_high;
} FeabhasButtonMap;
typedef struct {
  int dbgserial_uart_index;
  int feabhas_control_uart_index;

  FeabhasButtonMap button_map[FEABHAS_NUM_BUTTONS];
  uint32_t gpio_idr_masks[STM32F4XX_GPIO_COUNT];

  // memory sizes
  uint32_t flash_size;
  uint32_t ram_size;
} FeabhasBoardConfig;
