#include <memory.h>

#include <neuron.h>

#include "modbus_point.h"

struct modbus_sort_ctx {
    uint16_t start;
    uint16_t end;
};

static __thread uint16_t modbus_read_max_byte = 255;

static int  tag_cmp(neu_tag_sort_elem_t *tag1, neu_tag_sort_elem_t *tag2);
static bool tag_sort(neu_tag_sort_t *sort, void *tag, void *tag_to_be_sorted);

int modbus_tag_to_point(neu_datatag_t *tag, modbus_point_t *point)
{
    int ret = NEU_ERR_SUCCESS;
    if ((tag->attribute & NEU_ATTRIBUTE_SUBSCRIBE) == NEU_ATTRIBUTE_SUBSCRIBE) {
        return NEU_ERR_TAG_ATTRIBUTE_NOT_SUPPORT;
    }

    ret = neu_datatag_parse_addr_option(tag, &point->option);
    if (ret != 0) {
        return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
    }

    char area = 0;
    int  n    = sscanf(tag->addr_str, "%hhd!%c%hd", &point->slave_id, &area,
                   &point->start_address);
    if (n != 3) {
        return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
    }

    point->start_address -= 1;
    point->type = tag->type;

    switch (area) {
    case '0':
        point->area = MODBUS_AREA_COIL;
        break;
    case '1':
        point->area = MODBUS_AREA_INPUT;
        break;
    case '3':
        point->area = MODBUS_AREA_INPUT_REGISTER;
        break;
    case '4':
        point->area = MODBUS_AREA_HOLD_REGISTER;
        break;
    default:
        return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
    }

    if (point->area == MODBUS_AREA_INPUT ||
        point->area == MODBUS_AREA_INPUT_REGISTER) {
        if ((tag->attribute & NEU_ATTRIBUTE_WRITE) == NEU_ATTRIBUTE_WRITE) {
            return NEU_ERR_TAG_ATTRIBUTE_NOT_SUPPORT;
        }
    }

    switch (point->area) {
    case MODBUS_AREA_INPUT:
    case MODBUS_AREA_COIL:
        if (point->type != NEU_DTYPE_BIT) {
            return NEU_ERR_TAG_TYPE_NOT_SUPPORT;
        }
        if (point->option.bit.bit > 7) {
            return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
        }
        break;
    case MODBUS_AREA_INPUT_REGISTER:
    case MODBUS_AREA_HOLD_REGISTER:
        if (point->type == NEU_DTYPE_CSTR && point->option.string.length <= 0) {
            return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
        }
        if (point->type == NEU_DTYPE_BIT && point->option.bit.bit > 15) {
            return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
        }
        if (point->type == NEU_DTYPE_BIT &&
            (tag->attribute & NEU_ATTRIBUTE_WRITE) == NEU_ATTRIBUTE_WRITE) {
            return NEU_ERR_TAG_ATTRIBUTE_NOT_SUPPORT;
        }
        break;
    }

    switch (point->type) {
    case NEU_DTYPE_BIT:
        point->n_register = 1;
        break;
    case NEU_DTYPE_UINT16:
    case NEU_DTYPE_INT16:
        if (point->area == MODBUS_AREA_COIL ||
            point->area == MODBUS_AREA_INPUT) {
            ret = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
        } else {
            point->n_register = 1;
        }
        break;
    case NEU_DTYPE_UINT32:
    case NEU_DTYPE_INT32:
    case NEU_DTYPE_FLOAT:
        if (point->area == MODBUS_AREA_COIL ||
            point->area == MODBUS_AREA_INPUT) {
            ret = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
        } else {
            point->n_register = 2;
        }
        break;
    case NEU_DTYPE_CSTR:
        if (point->area == MODBUS_AREA_COIL ||
            point->area == MODBUS_AREA_INPUT) {
            ret = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
        } else {
            if (point->option.string.length > 127) {
                return NEU_ERR_TAG_ADDRESS_FORMAT_INVALID;
            }
            switch (point->option.string.type) {
            case NEU_DATATAG_STRING_TYPE_H:
            case NEU_DATATAG_STRING_TYPE_L:
                point->n_register = point->option.string.length / 2 +
                    point->option.string.length % 2;
                break;
            case NEU_DATATAG_STRING_TYPE_D:
            case NEU_DATATAG_STRING_TYPE_E:
                point->n_register = point->option.string.length;
                break;
            }
        }
        break;
    default:
        return NEU_ERR_TAG_TYPE_NOT_SUPPORT;
    }

    strncpy(point->name, tag->name, sizeof(point->name));
    return 0;
}

modbus_read_cmd_sort_t *modbus_tag_sort(UT_array *tags, uint16_t max_byte)
{
    modbus_read_max_byte          = max_byte;
    neu_tag_sort_result_t *result = neu_tag_sort(tags, tag_sort, tag_cmp);

    modbus_read_cmd_sort_t *sort_result =
        calloc(1, sizeof(modbus_read_cmd_sort_t));
    sort_result->n_cmd = result->n_sort;
    sort_result->cmd   = calloc(result->n_sort, sizeof(modbus_read_cmd_t));

    for (uint16_t i = 0; i < result->n_sort; i++) {
        modbus_point_t *tag =
            *(modbus_point_t **) utarray_front(result->sorts[i].tags);
        struct modbus_sort_ctx *ctx = result->sorts[i].info.context;

        sort_result->cmd[i].tags     = utarray_clone(result->sorts[i].tags);
        sort_result->cmd[i].slave_id = tag->slave_id;
        sort_result->cmd[i].area     = tag->area;
        sort_result->cmd[i].start_address = tag->start_address;
        sort_result->cmd[i].n_register    = ctx->end - ctx->start;

        free(result->sorts[i].info.context);
    }

    neu_tag_sort_free(result);
    return sort_result;
}

void modbus_tag_sort_free(modbus_read_cmd_sort_t *cs)
{
    for (uint16_t i = 0; i < cs->n_cmd; i++) {
        utarray_free(cs->cmd[i].tags);
    }

    free(cs->cmd);
    free(cs);
}

static int tag_cmp(neu_tag_sort_elem_t *tag1, neu_tag_sort_elem_t *tag2)
{
    modbus_point_t *p_t1 = (modbus_point_t *) tag1->tag;
    modbus_point_t *p_t2 = (modbus_point_t *) tag2->tag;

    if (p_t1->slave_id > p_t2->slave_id) {
        return 1;
    } else if (p_t1->slave_id < p_t2->slave_id) {
        return -1;
    }

    if (p_t1->area > p_t2->area) {
        return 1;
    } else if (p_t1->area < p_t2->area) {
        return -1;
    }

    if (p_t1->start_address > p_t2->start_address) {
        return 1;
    } else if (p_t1->start_address < p_t2->start_address) {
        return -1;
    }

    if (p_t1->n_register > p_t2->n_register) {
        return 1;
    } else if (p_t1->n_register < p_t2->n_register) {
        return -1;
    }

    return 0;
}

static bool tag_sort(neu_tag_sort_t *sort, void *tag, void *tag_to_be_sorted)
{
    modbus_point_t *        t1  = (modbus_point_t *) tag;
    modbus_point_t *        t2  = (modbus_point_t *) tag_to_be_sorted;
    struct modbus_sort_ctx *ctx = NULL;

    if (sort->info.context == NULL) {
        sort->info.context = calloc(1, sizeof(struct modbus_sort_ctx));
        ctx                = (struct modbus_sort_ctx *) sort->info.context;
        ctx->start         = t1->start_address;
        ctx->end           = t1->start_address + t1->n_register;
        return true;
    }

    ctx = (struct modbus_sort_ctx *) sort->info.context;

    if (t1->slave_id != t2->slave_id) {
        return false;
    }

    if (t1->area != t2->area) {
        return false;
    }

    if (t2->start_address > ctx->end) {
        return false;
    }

    switch (t1->area) {
    case MODBUS_AREA_COIL:
    case MODBUS_AREA_INPUT:
        if ((ctx->end - ctx->start) / 8 >= modbus_read_max_byte - 1) {
            return false;
        }
        break;
    case MODBUS_AREA_INPUT_REGISTER:
    case MODBUS_AREA_HOLD_REGISTER: {
        uint16_t now_bytes = (ctx->end - ctx->start) * 2;
        uint16_t add_now   = now_bytes + t2->n_register * 2;
        if (add_now >= modbus_read_max_byte) {
            return false;
        }

        break;
    }
    }

    if (t2->start_address + t2->n_register > ctx->end) {
        ctx->end = t2->start_address + t2->n_register;
    }

    return true;
}
