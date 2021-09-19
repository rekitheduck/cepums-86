#include "cepumspch.h"

#include "Core.h"
#include "IOManager.h"
#include "Log.h"
#include "MemoryManager.h"
#include "Processor/Processor.h"

#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL.h>

SDL_Texture* g_charBitmaps[256];

bool loadFontTextures(SDL_Renderer* renderer)
{
    // Load font file
    std::ifstream font_stream("default-font.bin", std::ios::in | std::ios::binary);
    if (!font_stream)
    {
        DC_CORE_ERROR("Font loader failed to open 'default-font.bin' fot file");
        return false;
    }

    // Read the font file
    uint8_t font_raw[16 * 256];
    font_stream.read((char *)&font_raw, 4096);
    font_stream.close();

    SDL_Surface* surf = nullptr;

    // For every character
    for (auto i = 0; i < 256; i++)
    {
        uint8_t data[8 * 16]{ 0x00 };

        // For every byte of raw font data
        for (auto j = 0; j < 16; j++)
        {
            // 1 byte (j) is a line in data

            // For every bit in a byte
            for (auto k = 0; k < 8; k++)
            {
                if (font_raw[i * 16 + j] & (1 << k))
                    data[j * 8 + k] = 0x01;
            }
        }

        surf = SDL_CreateRGBSurfaceFrom(&data, 8, 16, 8, 8, 0x00, 0x00, 0x00, 0x00);
        if (surf == nullptr)
        {
            DC_CORE_ERROR("Texture Error: {0}", SDL_GetError());
        };

        // Setup the palettes
        SDL_Palette* palette = SDL_AllocPalette(2);
        SDL_Color color;
        color.r = 0;
        color.g = 0;
        color.b = 0;
        color.a = 0;
        SDL_SetPaletteColors(palette, &color, 0, 1);
        color.r = 255;
        color.g = 255;
        color.b = 255;
        color.a = 255;
        SDL_SetPaletteColors(palette, &color, 1, 1);
        
        SDL_SetSurfacePalette(surf, palette);
        SDL_FreePalette(palette);

        g_charBitmaps[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_SetTextureBlendMode(g_charBitmaps[i], SDL_BLENDMODE_BLEND);

        SDL_FreeSurface(surf);
    }

    return true;
}

void deleteFontTextures()
{
    for (auto i = 0; i < 256; i++)
    {
        SDL_DestroyTexture(g_charBitmaps[i]);
    }
}

int main(int argc, char** argv)
{
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    // Initialize the basics
    Cepums::Log::init();
    DC_CORE_INFO("Cepums-86 starting up ...");

    // Create a window (regular MDA is 720x350)
    SDL_Window* window = SDL_CreateWindow("Cepums-86", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 720, 400, 0);
    
    // Handle errors
    if (!window)
    {
        DC_CORE_CRITICAL("SDL_CreateWindow: {0}", SDL_GetError());
        return 1;
    }

    // Create the renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load the font
    loadFontTextures(renderer);

    // Make a processor
    Cepums::Processor processor;

    Cepums::MemoryManager memoryManager;
    Cepums::IOManager ioManager;

    SDL_Rect font_rect;
    font_rect.x = 0;
    font_rect.y = 0;
    font_rect.w = 8;
    font_rect.h = 16;

    SDL_Rect bg_rect;
    bg_rect.x = 0;
    bg_rect.y = 0;
    bg_rect.w = 9;
    bg_rect.h = 16;

    SDL_RendererFlip flip_font = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL);

    bool shouldExecute = true;

    // Create the Processor loop thread
    std::thread processing([&] {
        uint64_t thrLastTime = 0;
        uint64_t thrCurrentTime = 0;
        double processorTimerAccumulator = 0;
        double PITTimerAccumulator = 0;

        while (shouldExecute)
        {
            thrLastTime = thrCurrentTime;
            thrCurrentTime = SDL_GetPerformanceCounter();
            double delta = (double)(thrCurrentTime - thrLastTime) * 1000 * 1000 / SDL_GetPerformanceFrequency(); // This is in microseconds
            processorTimerAccumulator += delta;
            PITTimerAccumulator += delta;

            // Run the CPU at 4.77272666 MHz
            if (processorTimerAccumulator > 0.20952383642268)
            {
                processorTimerAccumulator = 0;
                processor.execute(memoryManager, ioManager);
            }

            // Run the PIT at 1.193182 MHz
            if (PITTimerAccumulator > 0.83809511038551)
            {
                PITTimerAccumulator = 0;
                ioManager.runPIT();
            }
        }
    });

    uint8_t colorRegularR = 0xCC;
    uint8_t colorRegularG = 0x99;
    uint8_t colorRegularB = 0x00;

    uint8_t colorIntenseR = 0xFF;
    uint8_t colorIntenseG = 0xCF;
    uint8_t colorIntenseB = 0x00;

    unsigned int lastTime = 0;
    unsigned int currentTime = 0;

    unsigned int blinkTime = 0;

    while (shouldExecute)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                ioManager.onKeyPress(event.key.keysym.scancode);
                break;
            case SDL_KEYUP:
                ioManager.onKeyRelease(event.key.keysym.scancode);
                break;
            case SDL_QUIT:
                shouldExecute = false;
                break;
            default:
                break;
            }
        }

        lastTime = currentTime;
        currentTime = SDL_GetTicks();

        // Increment the blink timer
        blinkTime += currentTime - lastTime;

        // And reset if it's 0
        if (blinkTime > 1000)
            blinkTime = 0;

        // Get the MDA RAM
        auto mda = memoryManager.getMDA();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Reset rectangles before running
        font_rect.x = 0;
        font_rect.y = 0;
        bg_rect.x = 0;
        bg_rect.y = 0;
        for (auto y = 0; y < 25; y++)
        {
            for (auto  x = 0; x < 80; x++)
            {
                bool blink = false;
                bool high_intensity = false;
                bool invert = false;
                bool blank = false;
                bool underline = false;
                uint8_t character = mda.at((x * 2 + 160 * y));
                uint8_t attribute = mda.at((x * 2 + 160 * y) + 1);

                // Inverting and blank are special cases
                if (attribute == 0x70 || attribute == 0x78 || attribute == 0xF0 || attribute == 0xF8)
                    invert = true;
                else if (attribute == 0x00 || attribute == 0x08 || attribute == 0x80 || attribute == 0x88)
                    blank = true;

                // Underline checking requires some fancy stuff
                if (IS_BIT_SET(attribute, 0) && IS_BIT_NOT_SET(attribute, 1) && IS_BIT_NOT_SET(attribute, 2))
                    underline = true;

                // Bit 7 is blink
                if (IS_BIT_SET(attribute, 7))
                    blink = true;

                // Bit 3 is high intensity
                if (IS_BIT_SET(attribute, 3))
                    high_intensity = true;

                // Skip if blank :)
                if (blank)
                {
                    font_rect.x += 9;
                    bg_rect.x += 9;
                    continue;
                }

                // Draw background only if it's inverted
                if (invert)
                {
                    // High or regular intensity
                    if (high_intensity)
                        SDL_SetRenderDrawColor(renderer, colorIntenseR, colorIntenseG, colorIntenseB, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, colorRegularR, colorRegularG, colorRegularB, 255);
                    SDL_RenderFillRect(renderer, &bg_rect);

                    // Set character color to black in this case
                    SDL_SetTextureColorMod(g_charBitmaps[character], 0, 0, 0);
                }
                else
                {
                    if (high_intensity)
                    {
                        SDL_SetRenderDrawColor(renderer, colorIntenseR, colorIntenseG, colorIntenseB, 255);
                        SDL_SetTextureColorMod(g_charBitmaps[character], colorIntenseR, colorIntenseG, colorIntenseB);
                    }
                    else
                    {
                        SDL_SetRenderDrawColor(renderer, colorRegularR, colorRegularG, colorRegularB, 255);
                        SDL_SetTextureColorMod(g_charBitmaps[character], colorRegularR, colorRegularG, colorRegularB);
                    }
                }

                if (!blink || (blink && blinkTime > 500))
                {
                    if (underline)
                        SDL_RenderDrawLine(renderer, bg_rect.x, bg_rect.y + 14, bg_rect.x + 8, bg_rect.y + 14);
                    SDL_RenderCopyEx(renderer, g_charBitmaps[character], nullptr, &font_rect, 0, nullptr, flip_font);
                }

                font_rect.x += 9;
                bg_rect.x += 9;
            }

            font_rect.x = 0;
            font_rect.y += 16;
            bg_rect.x = 0;
            bg_rect.y += 16;
        }

        // Swaps buffers I think
        SDL_RenderPresent(renderer);
    }

    // Quit
    shouldExecute = false;
    processing.join();

    // Clean up SDL stuff
    deleteFontTextures();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
