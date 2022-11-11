/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
//#include "nvs_flash.h"
//#include "esp_event.h"
#include "esp_log.h"

#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#include "tbc_mqtt_helper.h"
//#include "protocol_examples_common.h"

static const char *TAG = "MY_FWUPDATE";

#define FW_DESCRIPTION              "FW_DESCRIPTION"
//#define CURRENT_FW_TITLE            "MY_ESP32_FW"
//#define CURRENT_FW_VERSION          "7.8.9"

//#define SW_DESCRIPTION              "SW_DESCRIPTION"
//#define CURRENT_SW_TITLE            "MY_ESP32_SW"
//#define CURRENT_SW_VERSION          "2.3.4"

typedef struct my_fwupdate{
    const esp_partition_t *configured_partition;  // = esp_ota_get_boot_partition()

    const esp_partition_t *running_partition;     // = esp_ota_get_running_partition()
    ////bool runnning_app_was_first_boot;             // defautl false

    const esp_partition_t *update_partition;      // = esp_ota_get_next_update_partition(NULL)
    esp_ota_handle_t update_handle;

    int size;                                     // new f/w size
    bool image_header_was_checked;                // new f/w  header was checked
} my_fwupdate_t;

static my_fwupdate_t _my_fwupdate = {
                .configured_partition = NULL,  // = esp_ota_get_boot_partition()
                .running_partition = NULL,     // = esp_ota_get_running_partition()
                .update_partition = NULL,      // = esp_ota_get_next_update_partition(NULL)

                .update_handle = 0,
                .image_header_was_checked = false
            };

static volatile bool _my_fwupdate_request_reboot = false;

static void ____fwupdate_reset(void)
{
    //_my_fwupdate.configured_partition = esp_ota_get_boot_partition();
    //_my_fwupdate.running_partition = esp_ota_get_running_partition();
    //_my_fwupdate.update_partition = esp_ota_get_next_update_partition(NULL);
    _my_fwupdate.update_handle = 0;
    _my_fwupdate.size = 0;
    _my_fwupdate.image_header_was_checked = false;
}

static bool ____fwupdate_diagnostic(void)
{
    return true;
    /*
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    return diagnostic_is_ok;
    */
}

//Don't call TBMCH API in the callback!
static const char* _my_fwupdate_on_get_current_title(tbmch_handle_t client, void *context)
{
    // TODO: division F/W or S/W !!!!

    static char title[64] = {0};

    //get titile from running_partition
    if (_my_fwupdate.running_partition) {
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(_my_fwupdate.running_partition, &running_app_info) == ESP_OK) { //esp_ota_get_app_description()
            ESP_LOGI(TAG, "Current running F/W title: %s", running_app_info.project_name);
            strncpy(title, running_app_info.project_name, sizeof(title)-1); //version
            return title;
        }
    }

    return NULL; //CURRENT_FW_TITLE;
}
//Don't call TBMCH API in the callback!
static const char* _my_fwupdate_on_get_current_version(tbmch_handle_t client, void *context)
{
    // TODO: division F/W or S/W !!!!

    static char version[64] = {0};

    //get version from running_partition
    if (_my_fwupdate.running_partition) {
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(_my_fwupdate.running_partition, &running_app_info) == ESP_OK) {
            ESP_LOGI(TAG, "Current running F/W version: %s", running_app_info.version);
            strncpy(version, running_app_info.version, sizeof(version)-1);
            return version;
        }
    }

    return NULL; //CURRENT_FW_VERSION;
}

//Don't call TBMCH API in the callback!
//return 1 on negotiate successful(next to F/W OTA), -1/ESP_FAIL on negotiate failure, 0/ESP_OK on already updated!
static tbmch_err_t _my_fwupdate_on_negotiate(tbmch_handle_t client, void *context,
        const char *fw_title, const char *fw_version, int fw_size,
        const char *fw_checksum, const char *fw_checksum_algorithm,
        char *fw_error, int error_size)
{
    // TODO: division F/W or S/W !!!!
    ESP_LOGI(TAG, "Receving F/W shared attributes: fw_title=%s, fw_version=%s, fw_size=%d, fw_checksum=%s, fw_checksum_algorithm=%s",
        fw_title, fw_version, fw_size, fw_checksum, fw_checksum_algorithm);

    //check fw_title != current_fw_title
    const char * current_fw_title = _my_fwupdate_on_get_current_title(client, context);
    if (strcmp(fw_title, current_fw_title)!=0) {
    
        ESP_LOGI(TAG, "New F/W titile(%s) is not equal to current F/W title(%s)", fw_title, current_fw_title);
        snprintf(fw_error, error_size, "New F/W titile(%s) is not equal to current F/W title(%s)", fw_title, current_fw_title);
        return ESP_FAIL;
    }
    
    //check fw_version == current_fw_version
    const char* current_fw_version = _my_fwupdate_on_get_current_version(client, context);
    if (strcmp(fw_version, current_fw_version)==0) {
        ESP_LOGI(TAG, "New F/W version(%s) is equal to current F/W version(%s)", fw_version, current_fw_version);
        snprintf(fw_error, error_size, "New F/W version(%s) is equal to current F/W version(%s)", fw_version, current_fw_version);
        return ESP_OK; // It MUST return 0/ESP_OK on F/W already updated!
    }

    // check fw_size > update.size
    if (_my_fwupdate.update_partition) {
        if (fw_size > _my_fwupdate.update_partition->size) {
            ESP_LOGI(TAG, "New F/W size(%d) is bigger than update partition size(%d)",
                        fw_size, _my_fwupdate.update_partition->size);
            snprintf(fw_error, error_size, "New F/W size(%d) is bigger than update partition size(%d)", 
                        fw_size, _my_fwupdate.update_partition->size);
            return ESP_FAIL;
        }
    }

    return 1; //return 1 on negotiate successful(next to F/W OTA)
}

//Don't call TBMCH API in the callback!
//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
static tbmch_err_t _my_fwupdate_on_write(tbmch_handle_t client, void *context,
                int request_id, int chunk_id, const void *fw_data, int data_read,
                char *fw_error, int error_size)
{
    // TODO: division F/W or S/W !!!!
    //esp_ota_verify_chip_id(handle->ota_upgrade_buf)??

    esp_err_t err;

    ESP_LOGI(TAG, "Receving F/W response: request_id=%d, chunk_id=%d, fw_data=%p, data_read=%d",
        request_id, chunk_id, fw_data, data_read);

    if (data_read < 0) {
        ESP_LOGE(TAG, "Error: F/W data read error! data_read(%d)<0. request_id=%d, chunk_id=%d", data_read, request_id, chunk_id);
        snprintf(fw_error, error_size, "Error: F/W data read error! data_read(%d)<0", data_read);
        return ESP_FAIL;
    } else if (data_read == 0) {
        //no code. continue f/w update.
        return ESP_OK;
    } else if (data_read > 0) {
        if (_my_fwupdate.image_header_was_checked == false) {
            esp_app_desc_t new_app_info;
            if (data_read < sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                ESP_LOGE(TAG, "received package is not fit len");
                //esp_ota_abort(_my_fwupdate.update_handle);
                snprintf(fw_error, error_size, "received package is not fit len");
                return ESP_FAIL;
            } else {
                // check current version with downloading
                const char *fw_data_ = fw_data;
                memcpy(&new_app_info, &fw_data_[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                esp_app_desc_t running_app_info;
                if (esp_ota_get_partition_description(_my_fwupdate.running_partition, &running_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                }

                const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                esp_app_desc_t invalid_app_info;
                if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                }

                // check current version with last invalid partition
                if (last_invalid_app != NULL) {
                    if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "New version is the same as invalid version.");
                        ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                        ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                        snprintf(fw_error, error_size, "New version is the same as invalid version.");
                        return ESP_FAIL;
                    }
                }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
                if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                    ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                    snprintf(fw_error, error_size, "Current running version is the same as a new. We will not continue the update.");
                    return ESP_FAIL;
                }
#endif

                _my_fwupdate.image_header_was_checked = true;

                err = esp_ota_begin(_my_fwupdate.update_partition, OTA_WITH_SEQUENTIAL_WRITES, &_my_fwupdate.update_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    //esp_ota_abort(_my_fwupdate.update_handle);
                    snprintf(fw_error, error_size, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    return ESP_FAIL;
                }
                ESP_LOGI(TAG, "esp_ota_begin succeeded");
            } 
        }

        err = esp_ota_write(_my_fwupdate.update_handle, (const void *)fw_data, data_read);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "write F/W data failure!");
            //esp_ota_abort(_my_fwupdate.update_handle);
            snprintf(fw_error, error_size, "write F/W data failure!");
            return ESP_FAIL;
        }
        _my_fwupdate.size += data_read;
        ESP_LOGD(TAG, "Written image length +%d", _my_fwupdate.size);
        return ESP_OK;
    }

    return ESP_OK;
}

//Don't call TBMCH API in the callback!
//return 0/ESP_OK on successful, -1/ESP_FAIL on failure
static tbmch_err_t _my_fwupdate_on_end(tbmch_handle_t client, void *context,
                                int request_id, int chunk_id, char *fw_error, int error_size)
{
    // TODO: division F/W or S/W !!!!
    ESP_LOGI(TAG, "F/W update success & end: request_id=%d, chunk_id=%d", request_id, chunk_id);

    esp_err_t err;
    err = esp_ota_end(_my_fwupdate.update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            snprintf(fw_error, error_size, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
            snprintf(fw_error, error_size, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(_my_fwupdate.update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        snprintf(fw_error, error_size, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return ESP_FAIL;
    }
    ____fwupdate_reset();

    //Prepare to restart system!
    _my_fwupdate_request_reboot = true; // restart esp32 in mqtt_app_start()

    return ESP_OK;
}

//Don't call TBMCH API in the callback!
static void _my_fwupdate_on_abort(tbmch_handle_t client, void *context,
                                int request_id, int chunk_id/*current chunk_id*/)
{
    // TODO: division F/W or S/W !!!!
    ESP_LOGI(TAG, "F/W update failure & abort: request_id=%d, chunk_id=%d", request_id, chunk_id);

    if (_my_fwupdate.update_handle) {
        esp_ota_abort(_my_fwupdate.update_handle);
    }
    ____fwupdate_reset();
}

tbmch_err_t my_fwupdate_init(tbmch_handle_t client_)
{
    // TODO: division F/W or S/W !!!!

    _my_fwupdate.configured_partition = esp_ota_get_boot_partition();
    _my_fwupdate.running_partition = esp_ota_get_running_partition();
    ////_my_fwupdate.runnning_app_was_first_boot = false;
    _my_fwupdate.update_partition = esp_ota_get_next_update_partition(NULL);
    _my_fwupdate.update_handle = 0;
    _my_fwupdate.size = 0;
    _my_fwupdate.image_header_was_checked = false;

    // check F/W OTA
    esp_ota_img_states_t ota_state;
    if (_my_fwupdate.running_partition && 
        //_my_fwupdate.running_partition->type == ESP_PARTITION_TYPE_APP &&
        //_my_fwupdate.running_partition->subtype != ESP_PARTITION_SUBTYPE_APP_FACTORY &&
        (esp_ota_get_state_partition(_my_fwupdate.running_partition, &ota_state) == ESP_OK))
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ////_my_fwupdate.runnning_app_was_first_boot = true;

            // run diagnostic function ...
            bool diagnostic_is_ok = ____fwupdate_diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }

    if (_my_fwupdate.configured_partition != _my_fwupdate.running_partition) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 _my_fwupdate.configured_partition->address, _my_fwupdate.running_partition->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }

    // check current version with last invalid partition
    if (_my_fwupdate.running_partition) {
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(_my_fwupdate.running_partition, &running_app_info) == ESP_OK) {
            ESP_LOGI(TAG, "Current running F/W version: %s", running_app_info.version);
        }
        ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
                _my_fwupdate.running_partition->type, _my_fwupdate.running_partition->subtype, _my_fwupdate.running_partition->address);
    }

    if (_my_fwupdate.update_partition == NULL) {
        ESP_LOGW(TAG, "_my_fwupdate.update_partition is NULL! Don't executive F/W update!");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Update partition type %d subtype %d (offset 0x%08x)",
            _my_fwupdate.update_partition->type, _my_fwupdate.update_partition->subtype, _my_fwupdate.update_partition->address);

    tbmch_otaupdate_config_t otaupdate_config = {
        .ota_type = TBMCH_OTAUPDATE_TYPE_FW, /*!< FW/TBMCH_OTAUPDATE_TYPE_FW or SW/TBMCH_OTAUPDATE_TYPE_SW  */
        .chunk_size = 16*1024,               /*!< chunk_size, eg: 8192. 0 to get all F/W or S/W by request  */
    
        .context = &_my_fwupdate, //NULL,
        .on_get_current_ota_title = _my_fwupdate_on_get_current_title,     /*!< callback of getting current F/W or S/W OTA title */
        .on_get_current_ota_version = _my_fwupdate_on_get_current_version, /*!< callback of getting current F/W or S/W OTA version */
        .on_ota_negotiate = _my_fwupdate_on_negotiate,   /*!< callback of F/W or S/W OTA attributes */
        .on_ota_write = _my_fwupdate_on_write,           /*!< callback of F/W or S/W OTA doing */
        .on_ota_end = _my_fwupdate_on_end,               /*!< callback of F/W or S/W OTA success & end */
        .on_ota_abort = _my_fwupdate_on_abort,           /*!< callback of F/W or S/W OTA failure & abort */

        ////.is_first_boot = _my_fwupdate.runnning_app_was_first_boot
    };
    tbmch_err_t err = tbmch_otaupdate_append(client_, FW_DESCRIPTION, &otaupdate_config);
    return err;
}

bool my_fwupdate_request_reboot(void)
{
    return _my_fwupdate_request_reboot;
}
