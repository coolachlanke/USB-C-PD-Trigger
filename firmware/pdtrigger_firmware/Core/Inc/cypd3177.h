#ifndef CYPD3177_H_
#define CYPD3177_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// I2C 7-bit address (datasheet says 0x08)
#define CYPD3177_I2C_ADDR   (0x08 << 1)  // HAL wants 8-bit shifted

// Register Addresses
#define CYPD_DEVICE_MODE_REG		0x0000
#define CYPD_SILICON_ID_REG			0x0002
#define CYPD_INTERRUPT_REG			0x0006
#define CYPD_PD_STATUS_REG			0x1008
#define CYPD_TYPE_C_STATUS_REG		0x100C
#define CYPD_BUS_VOLTAGE_REG		0x100D
#define CYPD_CURRENT_PDO_REG		0x1010
#define CYPD_CURRENT_RDO_REG		0x1014
#define CYPD_SWAP_RESPONSE_REG		0x1028
#define CYPD_EVENT_STATUS_REG		0x1044
#define CYPD_READ_GPIO_LEVEL_REG	0x0082
#define CYPD_SAMPLE_GPIO_REG		0x0083
#define CYPD_WRITE_DATA_MEM_REG		0x1800

#define CYPD_RESET_CMD				0x0008
#define CYPD_EVENT_MASK_CMD			0x1024
#define CYPD_DM_CONTROL_CMD			0x1000
#define CYPD_SELECT_SINK_PDO_CMD	0x1005
#define CYPD_PD_CONTROL_CMD			0x1006
#define CYPD_REQUEST_CMD			0x1050
#define CYPD_SET_GPIO_MODE_CMD		0x0080
#define CYPD_SET_GPIO_LEVEL_CMD		0x0081

#define CYPD_DEV_RESPONSE_CMD		0x007E
#define CYPD_PD_RESPONSE_CMD		0x1400

#define CYPD_DEVICE_ACTIVE			0x95
#define CYPD_DEVICE_INT				0x01
#define CYPD_PD_PORT_INT			0x02
#define CYPD_PORT_CONNECTED			0x01
#define CYPD_CC_POLARITY			0x02
#define CYPD_ATT_DEV_TYPE			0x1C
#define CYPD_CURR_LEVEL				0xC0
#define CYPD_CONTRACT_STATE			0x400
#define CYPD_SINK_TX				0x4000
#define CYPD_PE_STATE				0x8000

// Structs/enums
typedef enum {
	NO_ATT = 0x00,
	SOURCE_ATT = 0x02,
	DEBUG_ACC_ATT = 0x03
} att_dev_type;

typedef enum {
	LEVEL_900,
	LEVEL_1500,
	LEVEL_3000,
	LEVEL_RES
} curr_level;

typedef enum {
	POL_CC1,
	POL_CC2
} cc_pol;

typedef struct {
	bool device_int;
	bool pd_port_int;
} cypd3177_int_t;

typedef struct {
	bool port_partner_conn_status;
	cc_pol cc_polarity;
	att_dev_type att_dev_type;
	curr_level current_level;

} cypd3177_type_c_status_t;

typedef struct {
	bool explicit_contract;
	bool SinkTxOk;
	bool PE_SNK_Ready;

} cypd3177_pd_status_t;

/*Exported functions*/
HAL_StatusTypeDef CYPD3177_Write(uint16_t reg, uint8_t *data, uint16_t size);
HAL_StatusTypeDef CYPD3177_Read(uint16_t reg, uint8_t *data, uint16_t size);
HAL_StatusTypeDef CYPD3177_Online(bool *is_active);
HAL_StatusTypeDef CYPD3177_ID(uint16_t *id);
HAL_StatusTypeDef CYPD3177_VBUS_mV(uint16_t *voltage);
HAL_StatusTypeDef CYPD3177_Int_Read(cypd3177_int_t *status);
HAL_StatusTypeDef CYPD3177_TypeC_Status_Read(cypd3177_type_c_status_t *status);
HAL_StatusTypeDef CYPD3177_PD_Status_Read(cypd3177_pd_status_t *status);
HAL_StatusTypeDef CYPD3177_ChangePDO(uint32_t *pdo);

#endif
