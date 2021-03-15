/*
 * Copyright (C) 2021 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_temperature IPSO Temperature LwM2M object
 * @brief       IPSO Temperature object implementation for LwM2M client using Wakaama
 *
 * This implements the LwM2M IPSO Temperature object as specified in the LwM2M registry.
 *
 * This IPSO object should be used with a temperature sensor to report a temperature measurement.
 *
 * It also provides resources for minimum/maximum measured values and the minimum/maximum range that can be
 * measured by the temperature sensor. An example measurement unit is degrees Celsius.
 *
 * ## Resources
 *
 * For an XML description of the object see
 * https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/3303-1_0.xml
 *
 * | Name                              | ID   | Mandatory | Type   | Range | Units |
 * | --------------------------------- | :--: | :-------: | :----: | :---: | :---: |
 * | Sensor Value                      | 5700 |    Yes    |  Float |   -   |   -   |
 * | Min Measured Value                | 5601 |     No    |  Float |   -   |   -   |
 * | Max Measured Value                | 5602 |     No    |  Float |   -   |   -   |
 * | Min Range Value                   | 5603 |     No    |  Float |   -   |   -   |
 * | Max Range Value                   | 5604 |     No    |  Float |   -   |   -   |
 * | Sensor Units                      | 5701 |     No    | String |   -   |   -   |
 * | Reset Min and Max Measured Values | 5605 |     No    |    -   |   -   |   -   |
 *
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef OBJECTS_TEMPERATURE_H
#define OBJECTS_TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"

/**
 * @brief IPSO Temperature object ID.
 */
#define LWM2M_TEMPERATURE_OBJECT_ID     3303

/**
 * @name IPSO Temperature object resource's IDs.
 * @{
 */

/**
 * @brief Sensor value resource ID.
 */
#define LWM2M_TEMPERATURE_VALUE_ID          5700

/**
 * @brief Minimum measurement resource ID.
 */
#define LWM2M_TEMPERATURE_MIN_MEAS_ID       5601

/**
 * @brief Maximum measurement resource ID.
 */
#define LWM2M_TEMPERATURE_MAX_MEAS_ID       5602

/**
 * @brief Minumum range value resource ID.
 */
#define LWM2M_TEMPERATURE_MIN_RANGE_ID      5603

/**
 * @brief Maximum range value resource ID.
 */
#define LWM2M_TEMPERATURE_MAX_RANGE_ID      5604

/**
 * @brief Sensor units resource ID.
 */
#define LWM2M_TEMPERATURE_UNITS_ID          5701

/**
 * @brief Reset of the minimum and maximum measurements resrouce ID.
 */
#define LWM2M_TEMPERATURE_RESET_MIN_MAX_ID  5605
/** @} */

/**
 * @brief   Get the IPSO Temperature object handle
 *
 * @return Pointer to the global handle of the IPSO temperature object.
 */
lwm2m_object_t *lwm2m_object_temperature_get(void);

/**
 * @brief   Create a new IPSO Temperature object instance and add it to the @p object list.
 * 
 * @param[in, out] object           Temperature object handle.
 * @param[in]      instance_id      ID for the new instance.
 * @param[in]      min_range        Minimum value in the sensor range.
 * @param[in]      max_range        Maximum value in the sensor range.
 * @param[in]      units            String representing the sensor units. The string is copied.
 * @param[in]      units_len        Length of @p units.
 *
 * @return 0 on success
 * @return <0 otherwise
 */
int lwm2m_object_temperature_instance_create(lwm2m_object_t *object, uint16_t instance_id,
                                             double min_range, double max_range, const char *units,
                                             size_t units_len);

/**
 * @brief 
 * 
 */
int lwm2m_object_temperature_value_update(lwm2m_client_data_t *client_data, uint16_t instance_id,
                                          double val);
#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_TEMPERATURE_H */
/** @} */
