#include <main.h>
#include <driver/usb/usb_driver.h>

#include <driver/usb/usb_hub.h>

#include <driver/usb/usb_keyboard.h>

// Array of USB drivers.
const usb_driver_t UsbDrivers[] = {
    // Hubs.
    { "Hub", usb_hub_init },

    // HID devices.
    { "HID keyboard", usb_keyboard_init },

    // End driver.
    { "", NULL }
};