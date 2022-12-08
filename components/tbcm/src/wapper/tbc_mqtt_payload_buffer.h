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

// ThingsBoard Client MQTT payload buffer API

#ifndef _TBC_MQTT_PAYLOAD_BUFFER_H_
#define _TBC_MQTT_PAYLOAD_BUFFER_H_

//#include <stdint.h>
//#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TBCM_RX_MSG_LENGTH (128*1024)

/**
 * ThingsBoard MQTT Client receiving msg info
 */
typedef struct tbcm_rx_msg_info
{
    char *topic;            /*!< Topic associated with this event */
    char *payload;          /*!< Payload/Data associated with this event */
    int topic_len;          /*!< Length of the topic for this event associated with this event */
    int payload_len;            /*!< Length of the data for this event */
    int total_payload_len;      /*!< Total length of the data (longer data are supplied with multiple events) */
    int current_payload_offset; /*!< Actual offset for the data associated with this event */
} tbcm_rx_msg_info;

/**
 * ThingsBoard MQTT Client payload buffer
 */
typedef struct tbcm_payload_buffer {
    char *topic;            /*!< Topic associated with this event */
    char *payload;          /*!< Payload/Data associated with this event */
    int topic_len;          /*!< Length of the topic for this event associated with this event */
    //int payload_len;            /*!< Length of the data for this event */
    int total_payload_len;      /*!< Total length of the data (longer data are supplied with multiple events) */
    //int current_payload_offset; /*!< Actual offset for the data associated with this event */

    int received_len;           /*!< Alread received payload/data length */
} tbcm_payload_buffer_t;

typedef void (*tbcm_payload_buffer_on_process_t)
                                     (void *client, esp_mqtt_event_handle_t src_event,
                                      char *topic, int topic_len,
                                      char *payload, int payload_len);

void tbcm_payload_buffer_init(tbcm_payload_buffer_t *buffer);
void tbcm_payload_buffer_pocess(tbcm_payload_buffer_t *buffer, esp_mqtt_event_handle_t src_event,
                        void *client, tbcm_payload_buffer_on_process_t on_payload_process);
void tbcm_payload_buffer_clear(tbcm_payload_buffer_t *buffer);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
