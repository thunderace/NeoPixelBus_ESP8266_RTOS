/*-------------------------------------------------------------------------
NeoPixel library 

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/
#pragma once

#include <Arduino.h>

// some platforms do not define this standard progmem type for some reason
//
#ifndef PGM_VOID_P
#define PGM_VOID_P const void *
#endif

// '_state' flags for internal state
#define NEO_DIRTY   0x80 // a change was made to pixel data that requires a show

#include "internal/NeoHueBlend.h"

#include "internal/RgbColor.h"
#include "internal/HslColor.h"
#include "internal/HsbColor.h"
#include "internal/HtmlColor.h"
#include "internal/RgbwColor.h"

#include "internal/NeoColorFeatures.h"
#include "internal/DotStarColorFeatures.h"

#include "internal/Layouts.h"
#include "internal/NeoTopology.h"
#include "internal/NeoRingTopology.h"
#include "internal/NeoTiles.h"
#include "internal/NeoMosaic.h"

#include "internal/NeoBufferContext.h"
#include "internal/NeoBufferMethods.h"
#include "internal/NeoBuffer.h"
#include "internal/NeoSpriteSheet.h"
#include "internal/NeoBitmapFile.h"
#include "internal/NeoDib.h"
#include "internal/NeoEase.h"
#include "internal/NeoGamma.h"

#if defined(ARDUINO_ARCH_ESP8266)

#include "internal/NeoEsp8266DmaMethod.h"
#include "internal/NeoEsp8266UartMethod.h"
#include "internal/NeoEspBitBangMethod.h"
#include "internal/DotStarGenericMethod.h"

#elif defined(ARDUINO_ARCH_ESP32)

#include "internal/NeoEsp32I2sMethod.h"
#include "internal/NeoEspBitBangMethod.h"
#include "internal/DotStarGenericMethod.h"

#elif defined(__arm__) // must be before ARDUINO_ARCH_AVR due to Teensy incorrectly having it set

#include "internal/NeoArmMethod.h"
#include "internal/DotStarGenericMethod.h"

#elif defined(ARDUINO_ARCH_AVR)

#include "internal/NeoAvrMethod.h"
#include "internal/DotStarAvrMethod.h"

#elif defined(CONFIG_TARGET_PLATFORM_ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)

#include "internal/NeoEsp8266RtosDmaMethod.h"

#else
#error "Platform Currently Not Supported, please add an Issue at Github/Makuna/NeoPixelBus"
#endif

#if !defined(__AVR_ATtiny85__)
#include "internal/DotStarSpiMethod.h"
#endif

static const uint8_t _NeoPixelGammaTable[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
    3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
    7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
   13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
   20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
   30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
   42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
   58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
   76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
   97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
  122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
  150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
  182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
  218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255};



template<typename T_COLOR_FEATURE, typename T_METHOD> class NeoPixelBus
{
public:
    // Constructor: number of LEDs, pin number
    // NOTE:  Pin Number maybe ignored due to hardware limitations of the method.
   
    NeoPixelBus(uint16_t countPixels, uint8_t pin) :
        _countPixels(countPixels),
        _state(0),
        _method(pin, countPixels, T_COLOR_FEATURE::PixelSize)
    {
    }

    NeoPixelBus(uint16_t countPixels, uint8_t pinClock, uint8_t pinData) :
        _countPixels(countPixels),
        _state(0),
        _method(pinClock, pinData, countPixels, T_COLOR_FEATURE::PixelSize)
    {
    }

    NeoPixelBus(uint16_t countPixels) :
        _countPixels(countPixels),
        _state(0),
        _method(countPixels, T_COLOR_FEATURE::PixelSize)
    {
    }

    ~NeoPixelBus()
    {
    }

    operator NeoBufferContext<T_COLOR_FEATURE>()
    {
        Dirty(); // we assume you are playing with bits
        return NeoBufferContext<T_COLOR_FEATURE>(_method.getPixels(), _method.getPixelsSize());
    }

    void Begin()
    {
        _method.Initialize();
        Dirty();
    }

    void Show()
    {
        if (!IsDirty())
        {
            return;
        }

        _method.Update();

        ResetDirty();
    }

    inline bool CanShow() const
    { 
        return _method.IsReadyToUpdate();
    };

    bool IsDirty() const
    {
        return  (_state & NEO_DIRTY);
    };

    void Dirty()
    {
        _state |= NEO_DIRTY;
    };

    void ResetDirty()
    {
        _state &= ~NEO_DIRTY;
    };

    uint8_t* Pixels() 
    {
        return _method.getPixels();
    };

    size_t PixelsSize() const
    {
        return _method.getPixelsSize();
    };

    size_t PixelSize() const
    {
        return T_COLOR_FEATURE::PixelSize;
    };

    uint16_t PixelCount() const
    {
        return _countPixels;
    };

    void SetPixelColor(uint16_t indexPixel, typename T_COLOR_FEATURE::ColorObject color)
    {
        if (indexPixel < _countPixels)
        {
            T_COLOR_FEATURE::applyPixelColor(_method.getPixels(), indexPixel, color);
            Dirty();
        }
    };

    typename T_COLOR_FEATURE::ColorObject GetPixelColor(uint16_t indexPixel) const
    {
        if (indexPixel < _countPixels)
        {
            return T_COLOR_FEATURE::retrievePixelColor(_method.getPixels(), indexPixel);
        }
        else
        {
            // Pixel # is out of bounds, this will get converted to a 
            // color object type initialized to 0 (black)
            return 0;
        }
    };

    void ClearTo(typename T_COLOR_FEATURE::ColorObject color)
    {
        uint8_t temp[T_COLOR_FEATURE::PixelSize]; 
        uint8_t* pixels = _method.getPixels();

        T_COLOR_FEATURE::applyPixelColor(temp, 0, color);

        T_COLOR_FEATURE::replicatePixel(pixels, temp, _countPixels);

        Dirty();
    };

    void ClearTo(typename T_COLOR_FEATURE::ColorObject color, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first <= last)
        {
            uint8_t temp[T_COLOR_FEATURE::PixelSize];
            uint8_t* pixels = _method.getPixels();
            uint8_t* pFront = T_COLOR_FEATURE::getPixelAddress(pixels, first);

            T_COLOR_FEATURE::applyPixelColor(temp, 0, color);

            T_COLOR_FEATURE::replicatePixel(pFront, temp, last - first + 1);

            Dirty();
        }
    }

    void RotateLeft(uint16_t rotationCount)
    {
        if ((_countPixels - 1) >= rotationCount)
        {
            _rotateLeft(rotationCount, 0, _countPixels - 1);
        }
    }

    void RotateLeft(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= rotationCount)
        {
            _rotateLeft(rotationCount, first, last);
        }
    }

    void ShiftLeft(uint16_t shiftCount)
    {
        if ((_countPixels - 1) >= shiftCount)
        {
            _shiftLeft(shiftCount, 0, _countPixels - 1);
            Dirty();
        }
    }

    void ShiftLeft(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels && 
            last < _countPixels && 
            first < last &&
            (last - first) >= shiftCount)
        {
            _shiftLeft(shiftCount, first, last);
            Dirty();
        }
    }

    void RotateRight(uint16_t rotationCount)
    {
        if ((_countPixels - 1) >= rotationCount)
        {
            _rotateRight(rotationCount, 0, _countPixels - 1);
        }
    }

    void RotateRight(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= rotationCount)
        {
            _rotateRight(rotationCount, first, last);
        }
    }

    void ShiftRight(uint16_t shiftCount)
    {
        if ((_countPixels - 1) >= shiftCount)
        {
            _shiftRight(shiftCount, 0, _countPixels - 1);
            Dirty();
        }
    }

    void ShiftRight(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        if (first < _countPixels &&
            last < _countPixels &&
            first < last &&
            (last - first) >= shiftCount)
        {
            _shiftRight(shiftCount, first, last);
            Dirty();
        }
    }
    
    void SwapPixelColor(uint16_t indexPixelOne, uint16_t indexPixelTwo)
    {
        auto colorOne = GetPixelColor(indexPixelOne);
        auto colorTwo = GetPixelColor(indexPixelTwo);

        SetPixelColor(indexPixelOne, colorTwo);
        SetPixelColor(indexPixelTwo, colorOne);
    };

    static uint32_t   Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
        return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }
    
    static uint32_t   ColorHSV(uint16_t hue, uint8_t sat=255, uint8_t val=255) {
        uint8_t r, g, b;
        // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
        // 0 is not the start of pure red, but the midpoint...a few values above
        // zero and a few below 65536 all yield pure red (similarly, 32768 is the
        // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
        // each for red, green, blue) really only allows for 1530 distinct hues
        // (not 1536, more on that below), but the full unsigned 16-bit type was
        // chosen for hue so that one's code can easily handle a contiguous color
        // wheel by allowing hue to roll over in either direction.
        hue = (hue * 1530L + 32768) / 65536;
        // Because red is centered on the rollover point (the +32768 above,
        // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
        // where 0 and 1530 would yield the same thing. Rather than apply a
        // costly modulo operator, 1530 is handled as a special case below.
        
        // So you'd think that the color "hexcone" (the thing that ramps from
        // pure red, to pure yellow, to pure green and so forth back to red,
        // yielding six slices), and with each color component having 256
        // possible values (0-255), might have 1536 possible items (6*256),
        // but in reality there's 1530. This is because the last element in
        // each 256-element slice is equal to the first element of the next
        // slice, and keeping those in there this would create small
        // discontinuities in the color wheel. So the last element of each
        // slice is dropped...we regard only elements 0-254, with item 255
        // being picked up as element 0 of the next slice. Like this:
        // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
        // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
        // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
        // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
        // the constants below are not the multiples of 256 you might expect.
        
        // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
        if(hue < 510) {         // Red to Green-1
        b = 0;
        if(hue < 255) {       //   Red to Yellow-1
          r = 255;
          g = hue;            //     g = 0 to 254
        } else {              //   Yellow to Green-1
          r = 510 - hue;      //     r = 255 to 1
          g = 255;
        }
        } else if(hue < 1020) { // Green to Blue-1
        r = 0;
        if(hue <  765) {      //   Green to Cyan-1
          g = 255;
          b = hue - 510;      //     b = 0 to 254
        } else {              //   Cyan to Blue-1
          g = 1020 - hue;     //     g = 255 to 1
          b = 255;
        }
        } else if(hue < 1530) { // Blue to Red-1
        g = 0;
        if(hue < 1275) {      //   Blue to Magenta-1
          r = hue - 1020;     //     r = 0 to 254
          b = 255;
        } else {              //   Magenta to Red-1
          r = 255;
          b = 1530 - hue;     //     b = 255 to 1
        }
        } else {                // Last 0.5 Red (quicker than % operator)
        r = 255;
        g = b = 0;
        }
        
        // Apply saturation and value to R,G,B, pack into 32-bit result:
        uint32_t v1 =   1 + val; // 1 to 256; allows >>8 instead of /255
        uint16_t s1 =   1 + sat; // 1 to 256; same reason
        uint8_t  s2 = 255 - sat; // 255 to 0
        return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
              (((((g * s1) >> 8) + s2) * v1) & 0xff00)       |
             ( ((((b * s1) >> 8) + s2) * v1)           >> 8);
        
    }
      static uint8_t    gamma8(uint8_t x) {
        return _NeoPixelGammaTable[x]; // 0-255 in, 0-255 out
      }
    
    static uint32_t   gamma32(uint32_t x) {
      uint8_t *y = (uint8_t *)&x;
      // All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
      // to avoid a bunch of shifting and masking that would be necessary for
      // properly handling different endianisms (and each byte is a fairly
      // trivial operation, so it might not even be wasting cycles vs a check
      // and branch for the RGB case). In theory this might cause trouble *if*
      // someone's storing information in the unused most significant byte
      // of an RGB value, but this seems exceedingly rare and if it's
      // encountered in reality they can mask values going in or coming out.
      for(uint8_t i=0; i<4; i++) y[i] = gamma8(y[i]);
      return x; // Packed 32-bit return
        
    }

 
protected:
    const uint16_t _countPixels; // Number of RGB LEDs in strip

    uint8_t _state;     // internal state
    T_METHOD _method;

    void _rotateLeft(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        // store in temp
        uint8_t temp[rotationCount * T_COLOR_FEATURE::PixelSize];
        uint8_t* pixels = _method.getPixels();

        uint8_t* pFront = T_COLOR_FEATURE::getPixelAddress(pixels, first);

        T_COLOR_FEATURE::movePixelsInc(temp, pFront, rotationCount);

        // shift data
        _shiftLeft(rotationCount, first, last);

        // move temp back
        pFront = T_COLOR_FEATURE::getPixelAddress(pixels, last - (rotationCount - 1));
        T_COLOR_FEATURE::movePixelsInc(pFront, temp, rotationCount);

        Dirty();
    }

    void _shiftLeft(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        uint16_t front = first + shiftCount;
        uint16_t count = last - front + 1;

        uint8_t* pixels = _method.getPixels();
        uint8_t* pFirst = T_COLOR_FEATURE::getPixelAddress(pixels, first);
        uint8_t* pFront = T_COLOR_FEATURE::getPixelAddress(pixels, front);

        T_COLOR_FEATURE::movePixelsInc(pFirst, pFront, count);

        // intentional no dirty
    }

    void _rotateRight(uint16_t rotationCount, uint16_t first, uint16_t last)
    {
        // store in temp
        uint8_t temp[rotationCount * T_COLOR_FEATURE::PixelSize];
        uint8_t* pixels = _method.getPixels();

        uint8_t* pFront = T_COLOR_FEATURE::getPixelAddress(pixels, last - (rotationCount - 1));

        T_COLOR_FEATURE::movePixelsDec(temp, pFront, rotationCount);

        // shift data
        _shiftRight(rotationCount, first, last);

        // move temp back
        pFront = T_COLOR_FEATURE::getPixelAddress(pixels, first);
        T_COLOR_FEATURE::movePixelsDec(pFront, temp, rotationCount);

        Dirty();
    }

    void _shiftRight(uint16_t shiftCount, uint16_t first, uint16_t last)
    {
        uint16_t front = first + shiftCount;
        uint16_t count = last - front + 1;

        uint8_t* pixels = _method.getPixels();
        uint8_t* pFirst = T_COLOR_FEATURE::getPixelAddress(pixels, first);
        uint8_t* pFront = T_COLOR_FEATURE::getPixelAddress(pixels, front);

        T_COLOR_FEATURE::movePixelsDec(pFront, pFirst, count);
        // intentional no dirty
    }
};


