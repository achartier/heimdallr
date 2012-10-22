#include <stdio.h>
#include <stdlib.h>
#include <pci/pci.h>
#include <assert.h>

#include "json.h"
#include "pci.h"

#if 0
# define PCIBACK_SLOTS_FILE  "/sys/bus/pci/drivers/pciback/slots"
# define PCIBACK_QUIRKS_FILE "/sys/bus/pci/drivers/pciback/quirks"
#else
# define PCIBACK_SLOTS_FILE  "/tmp/slots"
# define PCIBACK_QUIRKS_FILE "/tmp/quirks"
#endif

pci_dev_infos*
get_pciback_devices(void)
{
    FILE *file = fopen(PCIBACK_SLOTS_FILE, "r");
    if (NULL == file)
    {
        perror (PCIBACK_SLOTS_FILE);
        return NULL;
    }

    char line[128];
    pci_dev_infos *res = NULL;

    while (fgets (line, sizeof (line), file) != NULL)
    {
        unsigned int domain, bus, dev;
        int func, scanned;

        scanned = sscanf(line, "%04x:%02x:%02x.%d\n",
                     &domain, &bus,
                     &dev, &func);
        if (4 != scanned) // the line was not well-formatted
            continue;

        res = pci_dev_infos_add(res, domain, bus, dev, func);
    }

    fclose (file);

    return res;
}

static void
fill_device_with_quirks(FILE *f,
                        struct pci_dev *dev,
                        pci_device_quirk *quirks)
{
    for (; quirks; quirks = quirks->next)
    {
        uint16_t vendor = strtoul(quirks->vendor, NULL, 16);
        if (vendor != 0xffff && vendor != dev->vendor_id)
            continue;

        uint16_t device = strtoul(quirks->device, NULL, 16);
        if (device != 0xffff && vendor != dev->device_id)
            continue;

        /* Check subvendor/subdevice ids as well */
        uint16_t subvendor = strtoul(quirks->subvendor, NULL, 16);
        uint16_t dev_subvendor = pci_read_word(dev, PCI_SUBSYSTEM_VENDOR_ID);
        if (subvendor != 0xffff && subvendor != dev_subvendor)
            continue;

        uint16_t subdevice = strtoul(quirks->subdevice, NULL, 16);
        uint16_t dev_subdevice = pci_read_word(dev, PCI_SUBSYSTEM_ID);
        if (subdevice != 0xffff && subdevice != dev_subdevice)
            continue;

        printf("match\n");
        struct pci_device_field *field = quirks->fields;
        for (; field; field = field->next)
        {
            uint32_t reg = strtoul(field->reg, NULL, 16);
            uint32_t size = field->size[0] - '0'; // we already tested that this field is correct
            uint32_t mask = strtoul(field->mask, NULL, 16);
            printf("gonna write stuff\n");
            fprintf(f, "%04x:%02x:%02x:%01x-%08x:%d:%08x\n",
                    dev->domain, dev->bus, dev->dev, dev->func,
                    reg, size, mask);
        }
    }
}

void
scan_all_pci_devices(pci_device_quirk *quirks,
                     pci_dev_infos    *pciback_devices)
{
    FILE *f = fopen(PCIBACK_QUIRKS_FILE, "w");
    if (!f)
    {
        perror(PCIBACK_QUIRKS_FILE);
        return;
    }


    struct pci_access *pacc = pci_alloc();
    pci_init(pacc);

    while (pciback_devices)
    {
        struct pci_dev *dev = pci_get_dev(pacc, pciback_devices->domain,
                                          pciback_devices->bus,
                                          pciback_devices->dev,
                                          pciback_devices->func);

        fill_device_with_quirks(f, dev, quirks);
        pci_free_dev(dev);
        pciback_devices = pciback_devices->next;
    }

    pci_cleanup(pacc);
    fclose(f);
}


void
usage(void)
{
    /* FIXME */
    printf("wrong usage\n");
}

int
main(int argc, char **argv)
{
    if (argc != 2)
    {
        usage();
        return 1;
    }

    pci_device_quirk *quirks = parse_json_file(argv[1]);

    if (NULL == quirks)
    {
        fprintf(stderr, "No quirks found.\n");
        return 2;
    }

    pci_dev_infos *pciback_devices = get_pciback_devices();

    if (NULL == pciback_devices)
    {
        fprintf(stderr, "No pciback device found.\n");
    }

    scan_all_pci_devices(quirks, pciback_devices);

    pci_device_quirk_free(quirks);
    pci_dev_infos_free(pciback_devices);
    return 0;
}
