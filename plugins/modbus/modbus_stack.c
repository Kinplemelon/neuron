#include <assert.h>

#include <neuron.h>

#include "modbus_stack.h"

struct modbus_stack {
    void *                  ctx;
    modbus_stack_send       send_fn;
    modbus_stack_value      value_fn;
    modbus_stack_write_resp write_resp;

    uint16_t seq;
};

static __thread void *write_req = NULL;

modbus_stack_t *modbus_stack_create(void *ctx, modbus_stack_send send_fn,
                                    modbus_stack_value      value_fn,
                                    modbus_stack_write_resp write_resp)
{
    modbus_stack_t *stack = calloc(1, sizeof(modbus_stack_t));

    stack->ctx        = ctx;
    stack->send_fn    = send_fn;
    stack->value_fn   = value_fn;
    stack->write_resp = write_resp;

    return stack;
}

void modbus_stack_destroy(modbus_stack_t *stack)
{
    free(stack);
}

int modbus_stack_recv(modbus_stack_t *stack, neu_protocol_unpack_buf_t *buf)
{
    struct modbus_header header = { 0 };
    struct modbus_code   code   = { 0 };
    int                  ret    = 0;

    ret = modbus_header_unwrap(buf, &header);
    if (ret <= 0) {
        return ret;
    }

    ret = modbus_code_unwrap(buf, &code);
    if (ret <= 0) {
        return ret;
    }

    switch (code.function) {
    case MODBUS_READ_COIL:
    case MODBUS_READ_INPUT:
    case MODBUS_READ_HOLD_REG:
    case MODBUS_READ_INPUT_REG: {
        struct modbus_data data  = { 0 };
        uint8_t *          bytes = NULL;
        ret                      = modbus_data_unwrap(buf, &data);
        if (ret <= 0) {
            return ret;
        }

        bytes = neu_protocol_unpack_buf(buf, data.n_byte);
        stack->value_fn(stack->ctx, code.slave_id, data.n_byte, bytes);

        break;
    }
    case MODBUS_WRITE_M_HOLD_REG:
    case MODBUS_WRITE_M_COIL: {
        struct modbus_address address = { 0 };
        ret                           = modbus_address_unwrap(buf, &address);
        if (ret <= 0) {
            stack->write_resp(stack->ctx, write_req,
                              NEU_ERR_PLUGIN_WRITE_FAILURE);
            return ret;
        }

        stack->write_resp(stack->ctx, write_req, 0);
        break;
    }
    case MODBUS_WRITE_S_COIL:
    case MODBUS_WRITE_S_HOLD_REG:
    default:
        break;
    }

    return neu_protocol_unpack_buf_used_size(buf);
}

int modbus_stack_read(modbus_stack_t *stack, uint8_t slave_id,
                      enum modbus_area area, uint16_t start_address,
                      uint16_t n_reg, uint16_t *response_size)
{
    static __thread uint8_t                 buf[32] = { 0 };
    static __thread neu_protocol_pack_buf_t pbuf    = { 0 };

    neu_protocol_pack_buf_init(&pbuf, buf, sizeof(buf));

    modbus_address_wrap(&pbuf, start_address, n_reg);

    switch (area) {
    case MODBUS_AREA_COIL:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_READ_COIL);
        *response_size += n_reg / 8 + ((n_reg % 8) > 0 ? 1 : 0);
        break;
    case MODBUS_AREA_INPUT:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_READ_INPUT);
        *response_size += n_reg / 8 + ((n_reg % 8) > 0 ? 1 : 0);
        break;
    case MODBUS_AREA_INPUT_REGISTER:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_READ_INPUT_REG);
        *response_size += n_reg * 2;
        break;
    case MODBUS_AREA_HOLD_REGISTER:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_READ_HOLD_REG);
        *response_size += n_reg * 2;
        break;
    }

    *response_size += sizeof(struct modbus_code);
    *response_size += sizeof(struct modbus_data);

    modbus_header_wrap(&pbuf, stack->seq++);
    *response_size += sizeof(struct modbus_header);

    return stack->send_fn(stack->ctx, neu_protocol_pack_buf_used_size(&pbuf),
                          neu_protocol_pack_buf_get(&pbuf));
}

int modbus_stack_write(modbus_stack_t *stack, void *req, uint8_t slave_id,
                       enum modbus_area area, uint16_t start_address,
                       uint16_t n_reg, uint8_t *bytes, uint8_t n_byte,
                       uint16_t *response_size)
{
    static __thread uint8_t                 buf[256] = { 0 };
    static __thread neu_protocol_pack_buf_t pbuf     = { 0 };

    neu_protocol_pack_buf_init(&pbuf, buf, sizeof(buf));

    modbus_data_wrap(&pbuf, n_byte, bytes);

    modbus_address_wrap(&pbuf, start_address, n_reg);

    switch (area) {
    case MODBUS_AREA_COIL:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_WRITE_M_COIL);
        break;
    case MODBUS_AREA_HOLD_REGISTER:
        modbus_code_wrap(&pbuf, slave_id, MODBUS_WRITE_M_HOLD_REG);
        break;
    default:
        assert(false);
        break;
    }
    *response_size += sizeof(struct modbus_code);
    *response_size += sizeof(struct modbus_address);

    modbus_header_wrap(&pbuf, stack->seq++);
    *response_size += sizeof(struct modbus_header);

    write_req = req;
    return stack->send_fn(stack->ctx, neu_protocol_pack_buf_used_size(&pbuf),
                          neu_protocol_pack_buf_get(&pbuf));
}