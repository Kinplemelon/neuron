/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2021 EMQ Technologies Co., Ltd All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include "connection/mqtt_client_intf.h"
#include "mqtt_c_client.h"

neu_err_code_e neu_mqtt_client_open(neu_mqtt_client_t *      p_client,
                                    const neu_mqtt_option_t *option,
                                    void *                   context)
{
    return mqtt_c_client_open((mqtt_c_client_t **) p_client, option, context);
}

neu_err_code_e neu_mqtt_client_is_connected(neu_mqtt_client_t client)
{
    return mqtt_c_client_is_connected(client);
}

neu_err_code_e neu_mqtt_client_subscribe(neu_mqtt_client_t client,
                                         const char *topic, const int qos,
                                         neu_subscribe_handle handle)
{
    return mqtt_c_client_subscribe(client, topic, qos, handle);
}

neu_err_code_e neu_mqtt_client_unsubscribe(neu_mqtt_client_t client,
                                           const char *      topic)
{
    return mqtt_c_client_unsubscribe(client, topic);
}

neu_err_code_e neu_mqtt_client_publish(neu_mqtt_client_t client,
                                       const char *topic, int qos,
                                       unsigned char *payload, size_t len)
{
    return mqtt_c_client_publish(client, topic, qos, payload, len);
}

neu_err_code_e neu_mqtt_client_suspend(neu_mqtt_client_t client)
{
    return mqtt_c_client_suspend(client);
}

neu_err_code_e neu_mqtt_client_continue(neu_mqtt_client_t client)
{
    return mqtt_c_client_continue(client);
}

neu_err_code_e neu_mqtt_client_close(neu_mqtt_client_t client)
{
    return mqtt_c_client_close(client);
}
