#define _GNU_SOURCE 1

#include <stdio.h>
#include <json.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "pci.h"

static inline char*
strdup_or_null(const char *default_value)
{
    return default_value ? strdup(default_value) : NULL;
}

static char*
json_parse_string_field(json_object *jvalue,
                        const char *field_name,
                        const char *field_default_value)
{
    json_object *jfield = json_object_object_get(jvalue, field_name);

    if (NULL == jfield || is_error(jfield))
        return strdup_or_null(field_default_value);

    if (!json_object_is_type(jfield, json_type_string))
        return strdup_or_null(field_default_value);

    return strdup(json_object_get_string(jfield));
}


static pci_device_field*
json_parse_config_space_field(json_object *jvalue,
                              pci_device_field *fields_list)
{
    char *reg = json_parse_string_field(jvalue, "register", NULL);
    char *size = json_parse_string_field(jvalue, "size", NULL);
    char *mask = json_parse_string_field(jvalue, "mask", "00000000");

    if (NULL == reg || NULL == size) // those fields are mandatory
        return NULL;

    if (strlen(reg) != 8)
    {
        printf("reg %s not good\n", reg);
        return NULL;
    }

    if (strlen(size) != 1 ||
        (size[0] != '1' && size[0] != '2' && size[0] != '4'))
    {
        printf("size %s not good\n", size);
        return NULL;
    }

    if (mask != NULL && strlen(mask) != 8)
    {
        printf("mask %s not good\n", mask);
        return NULL;
    }

    return pci_device_field_add(fields_list, reg, size, mask);
}


static pci_device_field*
json_parse_quirk_config_space_fields(json_object *jquirk)
{
    json_object *jarray = json_object_object_get(jquirk, "config_space_fields");
    pci_device_field *result = NULL;

    if (NULL == jarray)
        return NULL;

    if (!json_object_is_type(jarray, json_type_array))
        return NULL;

    int len = json_object_array_length(jarray);
    printf("found %d potential config space addresses to allow\n", len);

    json_object *jvalue;
    json_type type;
    for (int i = 0; i < len; ++i)
    {
        jvalue = json_object_array_get_idx(jarray, i);
        type = json_object_get_type(jvalue);

        if (type != json_type_object)
        {
            printf("field #%d is not an object, skipping it...\n", i);
            break;
        }

        result = json_parse_config_space_field(jvalue, result);

    }

    return result;
}

static pci_device_quirk*
json_parse_quirk(json_object *jquirk, pci_device_quirk *quirks_list)
{
    char *name = json_parse_string_field(jquirk, "name", "");
    char *vendor = json_parse_string_field(jquirk, "vendor", "ffff");
    char *device = json_parse_string_field(jquirk, "device", "ffff");
    char *subvendor = json_parse_string_field(jquirk, "subvendor", "ffff");
    char *subdevice = json_parse_string_field(jquirk, "subdevice", "ffff");

    // FIXME : help the user to figure out what his error is
    // He probably doesn't deserve it, but let's not be rude
    if (strlen(vendor) != 4 || strlen(device) != 4 ||
        strlen(subvendor) != 4 || strlen(subdevice) != 4)
        return NULL;

    pci_device_field *config_space_fields = json_parse_quirk_config_space_fields(jquirk);

    if (NULL == config_space_fields)
    {
        printf("no space fields found...not adding that quirk\n");
        return quirks_list;
    }

    return pci_device_quirk_add(quirks_list, name, device, vendor, subvendor,
                                subdevice, config_space_fields);
}

static pci_device_quirk*
json_parse_quirks_array(json_object *quirks_array)
{
    int len = json_object_array_length(quirks_array);
    pci_device_quirk *quirks = NULL;

    printf("found %d potential quirks to add...\n", len);

    json_object *jvalue;
    enum json_type type;
    for (int i = 0; i < len; ++i)
    {
        jvalue = json_object_array_get_idx(quirks_array, i);
        type = json_object_get_type(jvalue);

        if (type != json_type_object)
        {
            printf("quirk #%d is not an object, skipping it...\n", i);
            break;
        }

        quirks = json_parse_quirk(jvalue, quirks);
    }

    return quirks;
}

pci_device_quirk*
parse_json_file(const char *json_file)
{
    struct stat st;
    int res;
    char *json_str;
    json_object *jobj;
    enum json_type type;
    FILE *f;

    res = stat(json_file, &st);
    if (res == -1)
    {
        printf("bad file, bad !\n");
        return NULL;
    }

    json_str = malloc(st.st_size * sizeof (char));
    f = fopen(json_file, "r");
    fread(json_str, st.st_size, 1, f);
    fclose(f);
    jobj = json_tokener_parse(json_str);

    if (is_error(jobj))
    {
        printf("bad json bad !\n");
        return NULL;
    }

    type = json_object_get_type(jobj);
    if (type != json_type_array)
    {
        printf("root node should be an array\n");
        return NULL;
    }

    pci_device_quirk *result = json_parse_quirks_array(jobj);

    json_object_put(jobj);
    free(json_str);

    return result;
}
