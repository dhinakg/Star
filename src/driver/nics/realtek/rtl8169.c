/*
 * File: rtl8139.c
 * 
 * Copyright (c) 2017-2018 Sydney Erickson, John Davis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <main.h>
#include <io.h>
#include <kprint.h>
#include <string.h>

#include <kernel/memory/kheap.h>

#include <driver/pci.h>
#include <kernel/networking/networking.h>

bool rtl8169_init(pci_device_t *pciDevice) {
    // Is this actually a NIC?
    if (!(pciDevice->Class == PCI_CLASS_NETWORK && 
        pciDevice->Subclass == PCI_SUBCLASS_NETWORK_ETHERNET)) {
        return false;
    }

    // Is the PCI device an RTL8139?
        // Official RTL8169
    if (!(pciDevice->VendorId == 0x10EC && pciDevice->DeviceId == 0x8169) &&
        // Linksys E61032 Ver. 3
        !(pciDevice->VendorId == 0x1737 && pciDevice->DeviceId == 0x1032)) {
        return false;
    }

    int BaseAddress = pciDevice->BaseAddresses[0].BaseAddress;

    outb(BaseAddress + 0x37, 0x10);
    kprintf("\e[35mRTL8169: Sent reset bit to RTL8169.\n");
    while(inb(BaseAddress + 0x37) & 0x10){}
    kprintf("RTL8169: Reset completed.\n");

    kprintf("RTL8169: MAC address: ");
    for (int i = 0; i < 6; i++) {
        kprintf("%2X", inb(BaseAddress+i));
        if (i != 5) {
            kprintf(":");
        }
    }
    kprintf("\n");

    // Create network device.
    net_device_t *netDevice = (net_device_t*)kheap_alloc(sizeof(net_device_t));
    memset(netDevice, 0, sizeof(net_device_t));
    netDevice->Name = "RTL8169";

    // Register network device.
   // net_device_register(netDevice);

    // Return true, we have handled the PCI device passed to us
    return true;
}