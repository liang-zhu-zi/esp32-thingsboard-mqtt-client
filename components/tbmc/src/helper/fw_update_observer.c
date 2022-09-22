// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/thingsboard-mqttclient-basedon-espmqtt
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file is called by tb_mqtt_client_helper.c/.h.

#include <string.h>

#include "esp_err.h"

#include "fw_update_observer.h"
#include "tb_mqtt_client_helper_log.h"

const static char *TAG = "fw_update";

/*!< Initialize tbmch_fwupdate_t */
tbmch_fwupdate_t *_tbmch_fwupdate_init(tbmch_handle_t client, const char *fw_title,
                                       void *context,
                                       tbmch_fwupdate_on_sharedattributes_t on_fw_attributes,
                                       tbmch_fwupdate_on_response_t on_fw_response,
                                       tbmch_fwupdate_on_success_t on_fw_success,
                                       tbmch_fwupdate_on_timeout_t on_fw_timeout)
{
    if (!fw_title) {
        TBMCH_LOGE("method is NULL");
        return NULL;
    }
    if (!on_fw_attributes ) {
        TBMCH_LOGE("on_fw_attributes is NULL");
        return NULL;
    }
    if (!on_fw_response ) {
        TBMCH_LOGE("on_fw_response is NULL");
        return NULL;
    }
    if (!on_fw_success ) {
        TBMCH_LOGE("on_fw_success is NULL");
        return NULL;
    }
    if (!on_fw_timeout ) {
        TBMCH_LOGE("on_fw_timeout is NULL");
        return NULL;
    }
    
    tbmch_fwupdate_t *fwupdate = TBMCH_MALLOC(sizeof(tbmch_fwupdate_t));
    if (!fwupdate) {
        TBMCH_LOGE("Unable to malloc memeory!");
        return NULL;
    }

    memset(fwupdate, 0x00, sizeof(tbmch_fwupdate_t));
    fwupdate->client = client;
    fwupdate->fw_title = TBMCH_MALLOC(strlen(fw_title)+1);
    if (fwupdate->fw_title) {
        strcpy(fwupdate->fw_title, fw_title);
    }
    fwupdate->context = context;
    fwupdate->on_fw_attributes = on_fw_attributes;
    fwupdate->on_fw_response = on_fw_response;
    fwupdate->on_fw_success = on_fw_success;
    fwupdate->on_fw_timeout = on_fw_timeout;
    fwupdate->fw_version = NULL;
    fwupdate->fw_checksum = NULL;
    fwupdate->fw_checksum_algorithm = NULL;
    fwupdate->request_id = -1;
    fwupdate->chunk = 0;
    fwupdate->checksum = 0; 
    return fwupdate;
}
/*!< Destroys the tbmch_fwupdate_t */
tbmch_err_t _tbmch_fwupdate_destroy(tbmch_fwupdate_t *fwupdate)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return ESP_FAIL;
    }

    TBMCH_FREE(fwupdate->fw_title);
    TBMCH_FREE(fwupdate->fw_version);
    TBMCH_FREE(fwupdate->fw_checksum);
    TBMCH_FREE(fwupdate->fw_checksum_algorithm);
    TBMCH_FREE(fwupdate);
    return ESP_OK;
}

static void __tbmch_fwupdate_reset(tbmch_fwupdate_t *fwupdate)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return;
    }

    
    if (fwupdate->fw_version) {
        TBMCH_FREE(fwupdate->fw_version);
        fwupdate->fw_version = NULL;
    }
    if (fwupdate->fw_checksum) {
        TBMCH_FREE(fwupdate->fw_checksum);
        fwupdate->fw_checksum = NULL;
    }
    if (fwupdate->fw_checksum_algorithm) {
        TBMCH_FREE(fwupdate->fw_checksum_algorithm);
        fwupdate->fw_checksum_algorithm = NULL;
    }

    fwupdate->request_id = -1;
    fwupdate->chunk = 0;
    fwupdate->checksum = 0;
}

bool _tbmch_fwupdate_do_sharedattributes(tbmch_fwupdate_t *fwupdate, const char *fw_title, const char *fw_version,
                                         const char *fw_checksum, const char *fw_checksum_algorithm)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return false;
    }

    __tbmch_fwupdate_reset(fwupdate);
    bool result = fwupdate->on_fw_attributes(fwupdate->client, fwupdate->context,
                                             fw_title, fw_version, fw_checksum, fw_checksum_algorithm);
    if (result) {
        // cache fw_version
        fwupdate->fw_version = TBMCH_MALLOC(strlen(fw_version)+1);
        if (fwupdate->fw_version) {
            strcpy(fwupdate->fw_version, fw_version);
        }
        // cache fw_checksum
        fwupdate->fw_checksum = TBMCH_MALLOC(strlen(fw_checksum)+1);
        if (fwupdate->fw_checksum) {
            strcpy(fwupdate->fw_checksum, fw_checksum);
        }
        // cache fw_checksum_algorithm
        fwupdate->fw_checksum_algorithm = TBMCH_MALLOC(strlen(fw_checksum_algorithm)+1);
        if (fwupdate->fw_checksum_algorithm) {
            strcpy(fwupdate->fw_checksum_algorithm, fw_checksum_algorithm);
        }
    }

    return result;
}

const char *_tbmch_fwupdate_get_title(tbmch_fwupdate_t *fwupdate)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return NULL;
    }

    return fwupdate->fw_title;
}

int _tbmch_fwupdate_get_request_id(tbmch_fwupdate_t *fwupdate)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return -1;
    }

    return fwupdate->request_id;
}

tbmch_err_t _tbmch_fwupdate_set_request_id(tbmch_fwupdate_t *fwupdate, int request_id)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return ESP_FAIL;
    }

    fwupdate->request_id = request_id;
    return ESP_OK;
}

//return -1: end & failure;  0: end & success;  1: go on, get next package
int _tbmch_fwupdate_do_response(tbmch_fwupdate_t *fwupdate, int chunk/*current chunk*/, const void *fw_data, int data_size)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return -1; //end & failure
    }
    
    tbmch_err_t result = true;
    if (fw_data && data_size) {
        result = fwupdate->on_fw_response(fwupdate->client, fwupdate->context,
                                          fwupdate->request_id, chunk/*current chunk*/, fw_data, data_size);
        if (!result) {
            goto fwupdate_fail;
        } else {
            // TODO:  calc checksum: fwupdate->checksum = {fw_checksum_algorithm, fw_data, data_size};
            fwupdate->chunk = chunk+data_size; // TODO:...
            return 1; //go on
        }
    } else {
        // TODO: verify checksum: result = (fwupdate->checksum == fwupdate->fw_checksum{string});
        if (!result) {
            goto fwupdate_fail;
        } else {
            fwupdate->on_fw_success(fwupdate->client, fwupdate->context, fwupdate->request_id, chunk/*total_size*/);
            __tbmch_fwupdate_reset(fwupdate);
            return 0; //end & success
        }
    }

fwupdate_fail:
    // on_timeout() and reset()
    fwupdate->on_fw_timeout(fwupdate->client, fwupdate->context, fwupdate->request_id, chunk /*current chunk*/);
    __tbmch_fwupdate_reset(fwupdate);
    return -1; //end & failure
}

void _tbmch_fwupdate_do_timeout(tbmch_fwupdate_t *fwupdate)
{
    if (!fwupdate) {
        TBMCH_LOGE("fwupdate is NULL");
        return;
    }

    // on_timeout() and reset()
    if (fwupdate->on_fw_timeout && fwupdate->request_id>0) {
        fwupdate->on_fw_timeout(fwupdate->client, fwupdate->context, fwupdate->request_id, fwupdate->chunk/*current chunk*/);
    }
    __tbmch_fwupdate_reset(fwupdate);
}
