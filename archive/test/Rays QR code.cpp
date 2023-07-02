// Copyright 2019 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
​
// Display the About dialog, along with the QR code.  Note that the QR code URL may be formed by using
// an administrative environment variable, so that when the Airnote data is ultimately routed to a Route
// on the Notehub, this QR code can be updated to point to that same website where the data can be displayed.
​
// Status and Settings-related Menus
​
#include <qrcode.h>
#include "../../airnote.h"
#include "../ui/ui.h"
​
// The number of pixels on a side for QR code.  See the table here
// for the various parameters:  https://github.com/ricmoo/QRCode
#define QRDOUBLE
#ifdef QRDOUBLE
#define QRVERSION           6
#define QRVERSION_PIXELS    41
#define QRECC               ECC_MEDIUM      // 154 chars allowed in URL
#else
#define QRVERSION           14
#define QRVERSION_PIXELS    73
#define QRECC               ECC_HIGH        // 283 chars allowed in URL
#endif
#ifdef QRDOUBLE
#define QRPIXELS            ((((int)(QRVERSION_PIXELS+7)/8)*8)*2)
#else
#define QRPIXELS            (((int)(QRVERSION_PIXELS+7)/8)*8)
#endif
​
// Generate the QR code bitmap.  Arduino bitmaps are formatted as successive scan rows, with bytes
// flowing left-to-right and top-down. However, each byte (which represents 8 horizontal bits) is
// big-endian, meaning that the high order bit is leftmost on the display.
static uint8_t bitmap[QRPIXELS*(QRPIXELS/8)];
​
// Show the About screen
int actionAbout(int buttonState) {
​
    // If we've previously captured input, release it
    switch (buttonState) {
    case BUTTON_START:
        break;
    case BUTTON_RELEASED:
        return MENU_ACTION_CAPTURE;
    default:
        return MENU_ACTION_COMPLETED;
    }
​
    // Clear the display
    displayClear(false);
	displayCenteredBegin(FONT_TINY);
	char device[128], sn[128];
    NoteGetServiceConfigST(NULL, 0, NULL, 0, device, sizeof(device)-1, sn, sizeof(sn)-1);
	displayCentered(sn[0] != '\0' ? sn : device);
	displayCentered(ver());
	displayCenteredBottomEnd();
​
    // Compute position of bitmap
    int wScreen, hText;
    displayGetFontBounds(NULL, &hText);
    displayGetScreenBounds(&wScreen, NULL);
#ifdef QRDOUBLE
    int hBitmap = QRVERSION_PIXELS*2;
#else
    int hBitmap = QRVERSION_PIXELS;
#endif
    int wBitmap = hBitmap;
    int xBitmap = wScreen/2 - wBitmap/2;
    int yBitmap = hText/4;
​
    // URL-encode the device ID while constructing the URL to launch, allowing the notehub admin to
    // customize this link.
    char url[256];
    char chBuf[2] = {0};
    NoteGetEnv("product_url", PRODUCT_URL, url, sizeof(url));
    for (int i=0; i<strlen(device); i++) {
        uint8_t ch = device[i];
        if (ch >= 0x7f
            || (ch <= 0x1F)
            || (ch >= 0x20 && ch <= 0x2F)
            || (ch >= 0x3A && ch <= 0x40)
            || (ch >= 0x5B && ch <= 0x60)
            || (ch >= 0x7B && ch <= 0x7E)) {
            char chHex[5];
            snprintf(chHex, sizeof(chHex), "%%%02x", ch);
            strlcat(url, chHex, sizeof(url));
        } else {
            chBuf[0] = ch;
            strlcat(url, chBuf, sizeof(url));
        }
    }
    debugf("%s\n", url);
​
    // Compute the QR Code
    uint32_t qrcodeBytesLen = qrcode_getBufferSize(QRVERSION);
    uint8_t *qrcodeBytes = (uint8_t *) _malloc(qrcodeBytesLen);
    memset(bitmap, 0, sizeof(bitmap));
    if (qrcodeBytes != NULL) {
        QRCode qrcode;
        qrcode_initText(&qrcode, qrcodeBytes, QRVERSION, QRECC, url);
        uint8_t *p = bitmap;
        if (qrcode.size != QRVERSION_PIXELS)
            debugf("*** WRONG NUMBER OF PIXELS ***\n");
        else {
​
            // Analyze each "module" in the QR code (each pixel), and convert it to an
            // Arduino-compatible bitmap.  Note that if we are QRDOUBLE'ing its size for
            // readability, we need to set two actual pixels horizontally for each pixel
            // in the source, and we need to duplicate each line.
            for (uint8_t y = 0; y < qrcode.size; y++) {
#ifdef QRDOUBLE
                uint16_t px = 0;
#else
                uint8_t px = 0;
#endif
                uint8_t *q = p;
                for (uint8_t x = 0; x < qrcode.size; x++) {
                    if ((x & 0x07) == 0)
                        px = 0;
#ifdef QRDOUBLE
                    if (qrcode_getModule(&qrcode, x, y))
                        px |= (3 << (2*(7 - (x & 0x07))));
                    *q = (uint8_t) (px >> 8);
                    *(q+1) = (uint8_t) (px & 0xff);
#else
                    if (qrcode_getModule(&qrcode, x, y))
                        px |= (1 << (7 - (x & 0x07)));
                    *q = px;
#endif
                    if ((x & 0x07) == 7) {
                        q++;
#ifdef QRDOUBLE
                        q++;
#endif
                    }
                }
#ifdef QRDOUBLE
                memcpy(&p[QRPIXELS/8], p, QRPIXELS/8);
                p += QRPIXELS/8;
#endif
                p += QRPIXELS/8;
            }
        }
        _free(qrcodeBytes);
    }
​
    // Display the QR code
    rect bitmapRect;
    bitmapRect.x = xBitmap;
    bitmapRect.y = yBitmap;
    bitmapRect.w = QRPIXELS;
    bitmapRect.h = QRPIXELS;
    displayBitmap(&bitmapRect, bitmap);
​
    // Update the display
    displayUpdate();
​
    // Capture button input
    return MENU_ACTION_CAPTURE;
​
