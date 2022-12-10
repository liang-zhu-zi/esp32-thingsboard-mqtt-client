// Copyright 2022 liangzhuzhi2020@gmail.com, https://github.com/liang-zhu-zi/esp32-thingsboard-mqtt-client
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

// This file is called by ota_upadate.c/.h.

#ifndef _OTA_FWUPDATE_H_
#define _OTA_FWUPDATE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void*     ota_fwupdate_init(void);
tbc_err_t ota_fwupdate_negotiate(void *context_xw,
                                const char *current_fw_title, const char *current_fw_version,
                                const char *fw_title, const char *fw_version, uint32_t fw_size,
                                const char *fw_checksum, const char *fw_checksum_algorithm,
                                char *fw_error, int error_size);
tbc_err_t ota_fwupdate_write(void *context_xw,
                            const void *fw_data, uint32_t data_read,
                            char *fw_error, int error_size);
tbc_err_t ota_fwupdate_end(void *context_xw,
                           char *fw_error, int error_size);
void      ota_fwupdate_abort(void *context_xw);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
