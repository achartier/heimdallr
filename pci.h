#ifndef PCI_H_
#define PCI_H_

typedef struct pci_device_field
{
    char *reg;
    char *size;
    char *mask;
    struct pci_device_field *next;
} pci_device_field;

typedef struct pci_device_quirk
{
    char *name;
    char *device;
    char *vendor;
    char *subvendor;
    char *subdevice;
    struct pci_device_field *fields;
    struct pci_device_quirk *next;
} pci_device_quirk;

typedef struct pci_dev_infos
{
    unsigned int domain;
    unsigned int bus;
    unsigned int dev;
    int func;
    struct pci_dev_infos *next;
} pci_dev_infos;


pci_device_field *pci_device_field_add(pci_device_field *head,
                                       char *reg, char *size, char *mask);
pci_device_quirk *pci_device_quirk_add(pci_device_quirk *head, char *name,
                                       char *device, char *vendor,
                                       char *subvendor, char *subdevice,
                                       pci_device_field *fields);
pci_dev_infos *pci_dev_infos_add(pci_dev_infos *head, unsigned int domain,
                                 unsigned int bus, unsigned int dev, int func);

void pci_device_quirk_free(pci_device_quirk *head);
void pci_dev_infos_free(pci_dev_infos *head);

#endif /* PCI_H_ */
