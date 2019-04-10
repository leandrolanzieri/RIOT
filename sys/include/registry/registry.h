/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry RIOT Registry
 * @ingroup     sys
 * @brief       RIOT Registry module for handling runtime configurations
 *
 * ## About
 *
 * The RIOT Registry is a module for interacting with persistent key-value
 * configurations. It forms part of the Runtime Configuration System for RIOT
 * nodes.
 *
 * ## Architecture
 *
 * The Registry interacts with other RIOT modules via
 * @ref registry_config_group_t "Registry Handlers", and with non-volatile devices
 * via @ref sys_registry_store "Storage Facilities". This way the functionality
 * of the RIOT Registry is independent of the functionality of the module and
 * storage devices.
 *
 * ![RIOT Registry architecture](riot-registry-architecture.svg)
 *
 * ### Registry Handlers
 *
 * @ref registry_config_group_t "Registry Handlers" (RH) represent Configuration
 * Groups in the RIOT Registry. A RIOT module is required to implement and
 * register a RH in order to expose its configurations in the Registry.
 *
 * A RH is defined by a @ref registry_config_group_t::name "name" and a series of
 * handlers for interacting with the configuration parametes of the
 * configuration group. The handlers are:
 *
 * - @ref registry_config_group_t::hndlr_get "get"
 * - @ref registry_config_group_t::hndlr_set "set"
 * - @ref registry_config_group_t::hndlr_commit "commit"
 * - @ref registry_config_group_t::hndlr_export "export"
 *
 * It is responsibility of the module to implement these handlers and perform
 * all necessary checks before applying values to the configuration parameters.
 *
 * ### Storage Facilities
 *
 * @ref sys_registry_store "Storage Facilities" (SF) implement the
 * @ref registry_store_itf_t "storage interface" to allow the RIOT Registry to
 * load, search and store configuration parameters.
 *
 * ## RIOT Registry usage flow
 * - 1 The RIOT registry is initialized with @ref registry_init().
 * - 2 Modules declare and register RHs for configuration groups by calling
 *     @ref registry_register().
 * - 3 SFs are registered as sources and/or destinations of configurations by
 *     calling registry_<storage-name>_src() and registry_<storage-name>_dst().
 *
 * @{
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifndef REGISTRY_REGISTRY_H
#define REGISTRY_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "clist.h"
#include "cn-cbor/cn-cbor.h"

/**
 * @brief Maximum amount of levels of hierarchy in configurations names.
 */
#define REGISTRY_MAX_DIR_DEPTH     8

/**
 * @brief Maximum amount of characters per level in configurations names.
 */
#define REGISTRY_MAX_DIR_NAME_LEN  64

/**
 * @brief Maximum length of a value when converted to string
 */
#define REGISTRY_MAX_VAL_LEN       64

/**
 * @brief Maximum length of a configuration name.
 * @{
 */
#define REGISTRY_MAX_NAME_LEN      ((REGISTRY_MAX_DIR_NAME_LEN * \
                                    REGISTRY_MAX_DIR_DEPTH) + \
                                    (REGISTRY_MAX_DIR_DEPTH - 1))
/** @} */

/**
 * @brief Prototype of a callback function for the load action of a store
 * interface
 */
typedef void (*load_cb_t)(char *name, char *val, void *cb_arg);

/**
 * @brief Store facility descriptor
 */
typedef struct {
    clist_node_t node;                    /**< linked list node */
    const struct registry_store_itf *itf; /**< interface for the facility */
} registry_store_t;

/**
 * @brief Storage facility interface.
 * All store facilities should, at least, implement the load and save actions.
 */
typedef struct registry_store_itf {
    /**
     * @brief Loads all stored configuration
     *
     * @param[in]   store    storage facility descriptor
     * @param[out]  buf      buffer to save the stored configuration
     * @param[in]   buf_len  length of @p buf
     * @return 0 on success, non-zero on failure
     */
    int (*load)(registry_store_t *store, uint8_t *buf, size_t buf_len);

    /**
     * @brief If implemented, it is used for any preparation the storage may
     * need before starting a saving process.
     *
     * @param[in] store Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_start)(registry_store_t *store);

    /**
     * @brief Saves a parameter into storage.
     *
     * @param[in] store   Storage facility descriptor
     * @param[in] buf     CBOR encoded configuration
     */
    int (*save)(registry_store_t *store, uint8_t *buf, size_t buf_len);

    /**
     * @brief If implemented, it is used for any tear-down the storage may need
     * after a saving process.
     *
     * @param[in] store Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_end)(registry_store_t *store);
} registry_store_itf_t;

/**
 * @brief Configuration group descriptor. Each configuration group should
 * be registered using the @ref registry_register() function.
 * A handler provides the pointer to get, set and commit configuration
 * parameters.
 */
typedef struct {
    clist_node_t node; /**< Linked list node */
    char *name; /**< String representing the configuration group */
    cn_cbor *(*get)(cn_cbor *cb, cn_cbor_context *cb_ctx, void *ctx);
    int (*set)(cn_cbor *cb, void *ctx);
    int (*commit)(void *ctx);
    void *ctx; /**< Optional context used by the handlers */
} registry_config_group_t;

/**
 * @brief List of registered configuration groups
 */
extern clist_node_t registry_config_groups;

/**
 * @brief Initializes the RIOT Registry and the store modules.
 */
void registry_init(void);

/**
 * @brief Initializes the store module.
 */
void registry_store_init(void);

/**
 * @brief Registers a new configuration group.
 *
 * @param[in] group Pointer to the configuration group descriptor.
 */
void registry_register_config_group(registry_config_group_t *group);

/**
 * @brief Registers a new storage as a source of configurations. Multiple
 *        storages can be configured as sources at the same time. Configurations
 *        will be loaded from all of them. This is commonly called by the
 *        storage facilities who implement their own registry_<storage-name>_src
 *        function.
 *
 * @param[in] src Pointer to the storage to register as source.
 */
void registry_register_src(registry_store_t *src);

/**
 * @brief Registers a new storage as a destination for saving configurations.
 *        Only one storage can be registered as destination at a time. If a
 *        previous storage had been registered before it will be replaced by the
 *        new one. This is commonly called by the storage facilities who
 *        implement their own registry_<storage-name>_dst function.
 *
 * @param[in] dst Pointer to the storage to register
 */
void registry_register_dst(registry_store_t *dst);

/**
 * @brief Sets the configuration of a group. The CBOR element should be a
 *        map with one string key and a value. The key should match the name
 *        of the configuration group. The value will be used by the module.
 *
 * @param[in] name String of the name of the parameter to be set
 * @param[in] val_str New value for the parameter
 * @return -EINVAL if handlers could not be found, otherwise returns the
 *             value of the set handler function.
 */
int registry_set_config(cn_cbor *cb);

cn_cbor *registry_get_config(cn_cbor *cb, cn_cbor_context *cb_ctx);

int registry_commit(char *name);

int registry_load(cn_cbor_context *cb_ctx);

int registry_save(cn_cbor_context *cb_ctx);

int registry_export(int (*export_func)(cn_cbor *cb), char *name);

#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_REGISTRY_H */
