#include "cypd3177.h"
#include <string.h>
#include <stdio.h>

extern I2C_HandleTypeDef hi2c3;
extern UART_HandleTypeDef huart2;


/*Swap 16-bit endian-ness*/
static uint16_t swap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}


/*Write to CYPD3177 over I2C*/
HAL_StatusTypeDef CYPD3177_Write(uint16_t reg, uint8_t *data, uint16_t size) {
    uint16_t reg_swapped = swap16(reg);
    return HAL_I2C_Mem_Write(&hi2c3,
                             (CYPD3177_I2C_ADDR),
                             reg_swapped,
                             I2C_MEMADD_SIZE_16BIT,
                             data,
                             size,
                             HAL_MAX_DELAY);
}


/*Read from CYPD3177 over I2C*/
HAL_StatusTypeDef CYPD3177_Read(uint16_t reg, uint8_t *data, uint16_t size) {
    uint16_t reg_swapped = swap16(reg);
    return HAL_I2C_Mem_Read(&hi2c3,
                            (CYPD3177_I2C_ADDR),
                            reg_swapped,
                            I2C_MEMADD_SIZE_16BIT,
                            data,
                            size,
                            HAL_MAX_DELAY);
}


/*Check if CYPD3177 device is responsive*/
HAL_StatusTypeDef CYPD3177_Online(bool *is_active)
{
    uint8_t data = 0;
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_DEVICE_MODE_REG, &data, 1);
    if (res == HAL_OK) {
        *is_active = (data == CYPD_DEVICE_ACTIVE);
    }
    return res;
}


/*Read the CYPD3177 ID register*/
HAL_StatusTypeDef CYPD3177_ID(uint16_t *id) {
    uint8_t buf[2];
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_SILICON_ID_REG, buf, 2);
    if (res == HAL_OK) {
        *id = (buf[1] << 8) | buf[0];   // little endian -> host uint16
    }
    return res;
}

/*Read live VBUS voltage from CYPD3177*/
HAL_StatusTypeDef CYPD3177_VBUS_mV(uint16_t *voltage) {
    uint8_t data = 0;
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_BUS_VOLTAGE_REG, &data, 1);
    if (res == HAL_OK) {
        *voltage = data * 100; // LSB=100mV
    }
    return res;
}


/*Read the interrupt source and status*/
HAL_StatusTypeDef CYPD3177_Int_Read(cypd3177_int_t *status) {
    uint8_t data = 0;
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_INTERRUPT_REG, &data, 1);
    if (res == HAL_OK) {
        status->device_int  = (data & CYPD_DEVICE_INT);
        status->pd_port_int = (data & CYPD_PD_PORT_INT) >> 1;
    }
    return res;
}


/*Read the Type-C Status Register*/
HAL_StatusTypeDef CYPD3177_TypeC_Status_Read(cypd3177_type_c_status_t *status) {
    uint8_t data[4] = {0};
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_TYPE_C_STATUS_REG, data, 4);
    if (res == HAL_OK) {
        status->port_partner_conn_status = (data[0] & CYPD_PORT_CONNECTED);
        status->cc_polarity   = (cc_pol)((data[0] & CYPD_CC_POLARITY) >> 1);
        status->att_dev_type  = (att_dev_type)((data[0] & CYPD_ATT_DEV_TYPE) >> 2);
        status->current_level = (curr_level)((data[0] & CYPD_CURR_LEVEL) >> 6);
    }
    return res;
}


/* Read the PD Status Register */
HAL_StatusTypeDef CYPD3177_PD_Status_Read(cypd3177_pd_status_t *status) {
    uint8_t buf[4] = {0};
    HAL_StatusTypeDef res = CYPD3177_Read(CYPD_PD_STATUS_REG, buf, 4);
    if (res != HAL_OK) {
        return res;
    }

    uint32_t raw = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

    status->explicit_contract = (raw & CYPD_CONTRACT_STATE) ? true : false;
    status->SinkTxOk          = (raw & CYPD_SINK_TX) ? true : false;
    status->PE_SNK_Ready      = (raw & CYPD_PE_STATE) ? true : false;

    return HAL_OK;
}



/*Change CYPD3177 PDOs*/
HAL_StatusTypeDef CYPD3177_ChangePDO(uint32_t *pdo) {
    union {
        uint8_t  data8[12];
        uint32_t data32[3];
    } pdo_data;

    pdo_data.data32[0] = 0x534E4B50; // "SNKP"
    pdo_data.data32[1] = pdo[0];
    pdo_data.data32[2] = pdo[1];

    if (CYPD3177_Write(CYPD_WRITE_DATA_MEM_REG, pdo_data.data8, sizeof(pdo_data.data8)) != HAL_OK)
        return HAL_ERROR;

    uint8_t mask = 0x03; // Enables, PDO0 amd PD1. Higher voltage => higher priority.
    if (CYPD3177_Write(CYPD_SELECT_SINK_PDO_CMD, &mask, 1) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}

