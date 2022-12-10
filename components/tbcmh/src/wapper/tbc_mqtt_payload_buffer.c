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

// ThingsBoard MQTT Client low layer API

#include <stdio.h>
#include <string.h>
//#include <time.h>

//#include "freertos/FreeRTOS.h"
//#include "sys/queue.h"
#include "esp_err.h"
//#include "mqtt_client.h"

#include "tbc_mqtt_wapper.h"

#include "tbc_utils.h"

#include "tbc_mqtt_payload_buffer.h"

const static char *TAG = "tbcm_payload_buffer";

void tbcm_payload_buffer_init(tbcm_payload_buffer_t *buffer)
{
    if (!buffer) {
         TBC_LOGE("buffer is NULL!");
         return;
    }

    buffer->topic = NULL;       /*!< Topic associated with this event */
    buffer->payload = NULL;     /*!< Payload/Data associated with this event */
    buffer->topic_len = 0;      /*!< Length of the topic for this event associated with this event */
    //buffer->payload_len = 0;            /*!< Length of the data for this event */
    buffer->total_payload_len = 0;      /*!< Total length of the data (longer data are supplied with multiple events) */
    //buffer->current_payload_offset = 0; /*!< Actual offset for the data associated with this event */

    buffer->received_len = 0;
}
static void _tbcm_payload_buffer_free(tbcm_payload_buffer_t *buffer)
{
    if (!buffer) {
         TBC_LOGE("buffer is NULL!");
         return;
    }

    if (buffer->topic) {
        TBC_FREE(buffer->topic);
        buffer->topic = NULL;       /*!< Topic associated with this event */
    }
    if (buffer->payload) {
        TBC_FREE(buffer->payload);
        buffer->payload = NULL;     /*!< Payload/Data associated with this event */
    }
    buffer->topic_len = 0;              /*!< Length of the topic for this event associated with this event */
    //buffer->payload_len = 0;            /*!< Length of the data for this event */
    buffer->total_payload_len = 0;      /*!< Total length of the data (longer data are supplied with multiple events) */
    //buffer->current_payload_offset = 0; /*!< Actual offset for the data associated with this event */

    buffer->received_len = 0;
}

static void _tbcm_payload_buffer_feed(tbcm_payload_buffer_t *buffer, tbcm_rx_msg_info *rx_msg)
{
    if (!buffer) {
         TBC_LOGE("buffer is NULL!");
         return;
    }
    if (!rx_msg) {
         TBC_LOGE("rx_msg is NULL!");
         return;
    }

    // copy topic
    if (rx_msg->topic && (rx_msg->topic_len>0)) {
        if (buffer->topic) {
            TBC_FREE(buffer->topic);
            buffer->topic = NULL;
            buffer->topic_len = 0;
        }

        buffer->topic = TBC_MALLOC(rx_msg->topic_len);
        if (!buffer->topic) {
            TBC_LOGE("buffer->topic is NULL!");
            return;
        } else {
            memcpy(buffer->topic, rx_msg->topic, rx_msg->topic_len);
            buffer->topic_len = rx_msg->topic_len;
        }
    }

    // copy payload
    if (rx_msg->payload && (rx_msg->payload_len>0) && (rx_msg->total_payload_len>0)) {
        if (!buffer->payload) {
            buffer->total_payload_len = 0;
            buffer->received_len = 0;
            buffer->payload = TBC_MALLOC(rx_msg->total_payload_len);
            if (buffer->payload) {
                buffer->total_payload_len = rx_msg->total_payload_len;
                buffer->received_len += rx_msg->payload_len;
                memcpy(buffer->payload+rx_msg->current_payload_offset, rx_msg->payload, rx_msg->payload_len);
            }
        } else {
            if (buffer->total_payload_len != rx_msg->total_payload_len) {
                TBC_LOGI("buffer->total_payload_len(%d) != rx_msg->total_payload_len(%d)",
                    buffer->total_payload_len, rx_msg->total_payload_len);
                return;
            }

            memcpy(buffer->payload+rx_msg->current_payload_offset, rx_msg->payload, rx_msg->payload_len);
            buffer->received_len += rx_msg->payload_len;
        }
    }
}

static bool _tbcm_rx_msg_is_completion(tbcm_rx_msg_info *rx_msg)
{
    if (!rx_msg) {
        return false;
    }
    if (!rx_msg->topic) {
        TBC_LOGD("rx_msg->topic is NULL!");
        return false;
    }
    if (rx_msg->topic_len <= 0) {
        TBC_LOGD("rx_msg->topic_len(%d) is less than 0!", rx_msg->topic_len);
        return false;
    }
    if (rx_msg->payload_len < rx_msg->total_payload_len) {
        TBC_LOGD("rx_msg->payload_len(%d) is less then rx_msg->total_payload_len(%d)!",
            rx_msg->payload_len, rx_msg->total_payload_len);
        return false;
    }

    return true;
}
static bool _tbcm_payload_buffer_is_completion(tbcm_payload_buffer_t *buffer)
{
    if (!buffer) {
        return false;
    }
    if (!buffer->topic) {
        TBC_LOGD("buffer->topic is NULL!");
        return false;
    }
    if (buffer->topic_len <= 0) {
        TBC_LOGD("buffer->topic_len(%d) is less than 0!", buffer->topic_len);
        return false;
    }
    if (!buffer->payload) {
        TBC_LOGD("buffer->payload is NULL!");
        return false;
    }
    if (buffer->received_len < buffer->total_payload_len) {
        TBC_LOGD("buffer->received_len(%d) is less then buffer->total_payload_len(%d)!",
            buffer->received_len, buffer->total_payload_len);
        return false;
    }

    return true;
}

void tbcm_payload_buffer_pocess(tbcm_payload_buffer_t *buffer, esp_mqtt_event_handle_t src_event,
                        void *client, tbcm_payload_buffer_on_process_t on_payload_process)
{
    // 0: if parameter is invalid, then return.
    if (!buffer || !src_event || !on_payload_process) {
        TBC_LOGE("buffer(%p), src_event(%p), on_payload_process(%p) is NULL", 
            buffer, src_event, on_payload_process);
        return;
    }

    tbcm_rx_msg_info rx_msg_ = {0};
    tbcm_rx_msg_info *rx_msg = &rx_msg_;
    rx_msg->topic = src_event->topic;          /*!< Topic associated with this event */
    rx_msg->payload = src_event->data;         /*!< Data associated with this event */
    rx_msg->topic_len = src_event->topic_len;  /*!< Length of the topic for this event associated with this event */
    rx_msg->payload_len = src_event->data_len;                       /*!< Length of the data for this event */
    rx_msg->total_payload_len = src_event->total_data_len;           /*!< Total length of the data (longer data are supplied with multiple events) */
    rx_msg->current_payload_offset = src_event->current_data_offset; /*!< Actual offset for the data associated with this event */
    
    // 1: if new msg(rx_msg)->total_payload_len is too long(128K), then return.
    if (rx_msg->total_payload_len > MAX_TBCM_RX_MSG_LENGTH) {
        TBC_LOGE("rx_msg->total_payload_len(%d) is bigger than MAX_TBCM_RX_MSG_LENGTH(%d)",
            rx_msg->total_payload_len, MAX_TBCM_RX_MSG_LENGTH);
        return;
    }
    // check rx_msg->current_payload_offset
    if (rx_msg->current_payload_offset>0 &&
        rx_msg->current_payload_offset>=rx_msg->total_payload_len) {
        TBC_LOGI("rx_msg->current_payload_offset(%d) >= rx_msg->total_payload_len(%d)",
            rx_msg->current_payload_offset, rx_msg->total_payload_len);
        return;
    }

    // 2: if new msg(rx_msg)->topic is NOT NULL, then clear old un-completion msg.
    if (rx_msg->topic) {
        TBC_LOGD("rx_msg->topic(%.*s) is NOT NULL! free payload buffer!", rx_msg->topic_len, rx_msg->topic);
        _tbcm_payload_buffer_free(buffer);
    }

    // 3: if new msg(rx_msg) is completion, then process it, return.
    if (_tbcm_rx_msg_is_completion(rx_msg)) {
        on_payload_process(client, src_event, rx_msg->topic, rx_msg->topic_len, rx_msg->payload, rx_msg->payload_len);
        return;
    }

    // 4: feed new msg(rx_msg) to buffer.
    _tbcm_payload_buffer_feed(buffer, rx_msg);

    // 5: if buffer is completion, then process it, return.
    if (_tbcm_payload_buffer_is_completion(buffer)) {
        // tbcm_rx_msg_info temp_rx_msg;
        // temp_rx_msg.topic       = buffer->topic;        /*!< Topic associated with this event */
        // temp_rx_msg.topic_len   = buffer->topic_len;    /*!< Length of the topic for this event associated with this event */
        // temp_rx_msg.payload             = buffer->payload;          /*!< Payload/Data associated with this event */
        // temp_rx_msg.payload_len         = buffer->received_len;     /*!< Length of the data for this event */
        // temp_rx_msg.total_payload_len   = buffer->total_payload_len;/*!< Total length of the data (longer data are supplied with multiple events) */
        // temp_rx_msg.current_payload_offset = 0;                     /*!< Actual offset for the data associated with this event */
        on_payload_process(client, src_event, buffer->topic, buffer->topic_len, buffer->payload, buffer->received_len);
        _tbcm_payload_buffer_free(buffer);
        return;
    }
}

void tbcm_payload_buffer_clear(tbcm_payload_buffer_t *buffer)
{
    _tbcm_payload_buffer_free(buffer);
}

