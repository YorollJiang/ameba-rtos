/**
 * @file      rtk_bt_vendor.h
 * @author
 * @brief     Bluetooth Common function definition
 * @copyright Copyright (c) 2022. Realtek Semiconductor Corporation. All rights reserved.
 */

#ifndef __RTK_BT_VENDOR_H__
#define __RTK_BT_VENDOR_H__

#include <rtk_bt_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @typedef   rtk_bt_vendor_tx_power_param_t
 * @brief     Set ADV or connect TX power parameter.
 */
typedef struct {
	uint8_t tx_power_type;                  /*!< 0: Set ADV TX power, 1: Set connect TX power */
	union {
		struct adv_tx_power_param {
			uint8_t type;                   /*!< ADV TX power type */
		} adv_tx_power;

		struct conn_tx_power_param {
			uint16_t conn_handle;           /*!< Connect handle */
			uint8_t is_reset;               /*!< 0: User mode, 1: Reset to original */
		} conn_tx_power;
	};
	uint8_t tx_gain;                        /*!< Ref gain index */
} rtk_bt_vendor_tx_power_param_t;

/**
 * @typedef   rtk_bt_vendor_tx_power_subcmd_type_t
 * @brief     Set TX power subcmd type.
 */
typedef enum {
	SUB_CMD_SET_ADV_TX_POWER  = 0x00,
	SUB_CMD_SET_CONN_TX_POWER = 0x0c,
} rtk_bt_vendor_tx_power_subcmd_type_t;

/**
 * @typedef   rtk_bt_vendor_adv_tx_power_type_t
 * @brief     Set ADV TX power type.
 */
typedef enum {
	ADV_TX_POW_SET_1M,
	ADV_TX_POW_SET_2M,
	ADV_TX_POW_SET_1M_DEFAULT,
	ADV_TX_POW_SET_2M_DEFAULT,
} rtk_bt_vendor_adv_tx_power_type_t;

/**
 * @typedef   rtk_bt_vendor_conn_tx_power_is_reset_t
 * @brief     Set connect TX power type.
 */
typedef enum {
	CONN_TX_POW_USER_MODE,
	CONN_TX_POW_RESET_TO_ORIGINAL,
} rtk_bt_vendor_conn_tx_power_is_reset_t;

/**
 * @defgroup  bt_vendor BT Vendor APIs
 * @brief     BT vendor function APIs
 * @ingroup   BT_APIs
 * @{
 */

/**
 * @brief     BT controller power on.
 * @return    None
 */
void rtk_bt_controller_power_on(void);

/**
 * @brief     BT controller power off.
 * @return    None
 */
void rtk_bt_controller_power_off(void);

/**
 * @brief     Set BT TX power gain index.
 * @param[in] index: TX power gain index.
 * @return    None
 */
void rtk_bt_set_bt_tx_power_gain_index(uint32_t index);

/**
 * @brief     Set BT antenna. This API is only useable for AmebaSmart Platform.
 * @param[in] ant: BT antenna, 0 for ANT_S0, 1 for ANT_S1.
 * @return    None
 */
void rtk_bt_set_bt_antenna(uint8_t ant);

/**
 * @brief     Enable HCI debug.
 * @return    None
 */
void rtk_bt_hci_debug_enable(void);

/**
 * @brief     Enable BT Debug Port.
 * @param[in] bt_sel: BT Debug Port selection, 0 for BT vendor, 1 for BT on.
 * @param[in] bt_bdg_mask: BT debug mask, each bit set 1 means enable BT Debug Port[0-31].
 * @return    None
 */
void rtk_bt_debug_port_mask_enable(uint8_t bt_sel, uint32_t bt_bdg_mask);

/**
 * @brief     Enable BT Debug Port.
 * @param[in] bt_sel: BT Debug Port selection, 0 for BT vendor, 1 for BT on.
 * @param[in] bt_dbg_port: BT debug port bit, range 0~31.
 * @param[in] pad: PAD name.
 * @return    None
 */
void rtk_bt_debug_port_pad_enable(uint8_t bt_sel, uint8_t bt_dbg_port, char *pad);

/**
 * @brief     Shift BT Debug Port.
 * @param[in] original: Original BT Debug Port which is shift from.
 * @param[in] mapping: Target BT Debug Port mapping which is shift to.
 * @return    None
 */
void rtk_bt_debug_port_shift(uint8_t original, uint8_t mapping);

/**
 * @brief     Enable BT GPIO.
 * @param[in] bt_gpio: BT GPIO name.
 * @param[in] pad: PAD name.
 * @return    None
 */
void rtk_bt_gpio_enable(uint8_t bt_gpio, char *pad);

/**
 * @brief     Config BT sleep mode.
 * @param[in] mode: BT sleep mode.
 * @return    None
 */
void rtk_bt_sleep_mode(unsigned int mode);

/**
 * @brief     Set BT TX power.
 * @param[in] tx_power: Pointer to TX power parameter.
 * @return
 *            - 0  : Succeed
 *            - Others: Error code
 */
uint16_t rtk_bt_set_tx_power(rtk_bt_vendor_tx_power_param_t *tx_power);

/**
 * @brief     BT LE SOF and EOF interrupt indication enable or disable. This API is only useable for AmebaSmart Platform.
 * @param[in] conn_handle: Connection handle.
 * @param[in] enable: 0 for disable, 1 for enable.
 * @param[in] cb: SOF and EOF interrupt callback.
 * @return
 *            - 0  : Succeed
 *            - Others: Error code
 */
uint16_t rtk_bt_le_sof_eof_ind(uint16_t conn_handle, uint8_t enable, void *cb);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __RTK_BT_VENDOR_H__ */
