#include <stdlib.h>

#include "pci.h"

pci_device_field*
pci_device_field_add(pci_device_field *head,
                     char *reg, char *size, char *mask)
{
    pci_device_field *new = malloc (sizeof(pci_device_field));
    pci_device_field *runner = head;

    new->reg = reg;
    new->size = size;
    new->mask = mask;
    new->next = NULL;

    if (NULL == head)
        return new;

    while (runner->next != NULL)
        runner = runner->next;

    runner->next = new;
    return head;
}

pci_device_quirk*
pci_device_quirk_add(pci_device_quirk *head, char *name,
                     char *device, char *vendor,
                     char *subvendor, char *subdevice,
                     pci_device_field *fields)
{
    pci_device_quirk *new = malloc(sizeof(pci_device_quirk));
    pci_device_quirk *runner = head;

    new->name = name;
    new->device = device;
    new->vendor = vendor;
    new->subvendor = subvendor;
    new->subdevice = subdevice;
    new->fields = fields;
    new->next = NULL;

    if (NULL == head)
        return new;

    while (runner->next != NULL)
        runner = runner->next;

    runner->next = new;
    return head;
}

pci_dev_infos*
pci_dev_infos_add(pci_dev_infos *head, unsigned int domain,
                  unsigned int bus, unsigned int dev, int func)
{
    pci_dev_infos *new = malloc(sizeof(pci_dev_infos));
    pci_dev_infos *runner = head;

    new->domain = domain;
    new->bus = bus;
    new->dev = dev;
    new->func = func;

    if (NULL == head)
        return new;

    while (runner->next != NULL)
        runner = runner->next;

    runner->next = new;
    return head;
}

static void
pci_device_field_free(pci_device_field *head)
{
    pci_device_field *runner = NULL;

    while (head)
    {
        runner = head->next;

        free(head->reg);
        free(head->size);
        free(head->mask);

        free(head);
        head = runner;
    }
}

void
pci_device_quirk_free(pci_device_quirk *head)
{
    pci_device_quirk *runner = NULL;

    while (head)
    {
        runner = head->next;

        free(head->name);
        free(head->device);
        free(head->vendor);
        free(head->subvendor);
        free(head->subdevice);
        pci_device_field_free(head->fields);

        free(head);
        head = runner;
    }
}


void
pci_dev_infos_free(pci_dev_infos *head)
{
    pci_dev_infos *runner = NULL;

    while (head)
    {
        runner = head->next;
        free(head);
        head = runner;
    }
}
