#define SDL_MAIN_HANDLED

#define CYCLES_PER_FRAME 12

#define WINDOW_SCALE 10
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

#include <io.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

SDL_AudioSpec wav_spec;
Uint32 wav_length;
Uint8 *wav_buffer;
SDL_AudioDeviceID device_id;

unsigned char memory[4096]; // Main memory (4KB)
unsigned short I;           // Index register
unsigned short pc;          // Program counter
unsigned char V[16];        // General purpose registers V0–VF

unsigned short stack[16]; // Stack
unsigned short sp;        // Stack pointer

unsigned char delay_timer;
unsigned char sound_timer;

unsigned char gfx[64 * 32]; // Display (64x32 pixels)
unsigned short drawFlag;

unsigned char key[16]; // Key states
bool waiting_for_keypress;
uint8_t keypress_target_register;

unsigned short opcode; // Current opcode
int runCycles = 0;
uint32_t last_timer_tick = 0;

unsigned char chip8_fontset[80] =
    {

        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F

};

int map_sdl_key_to_chip8(SDL_Keycode keycode)
{
    switch (keycode)
    {
    case SDLK_1:
        return 0x1;
    case SDLK_2:
        return 0x2;
    case SDLK_3:
        return 0x3;
    case SDLK_4:
        return 0xC;
    case SDLK_q:
        return 0x4;
    case SDLK_w:
        return 0x5;
    case SDLK_e:
        return 0x6;
    case SDLK_r:
        return 0xD;
    case SDLK_a:
        return 0x7;
    case SDLK_s:
        return 0x8;
    case SDLK_d:
        return 0x9;
    case SDLK_f:
        return 0xE;
    case SDLK_z:
        return 0xA;
    case SDLK_x:
        return 0x0;
    case SDLK_c:
        return 0xB;
    case SDLK_v:
        return 0xF;
    default:
        return -1;
    }
}

void initialize()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (int i = 0; i < 80; ++i)
    {
        memory[0x50 + i] = chip8_fontset[i];
    }

    printf("The fontset is loaded into the memory!\n");

    delay_timer = 0;
    sound_timer = 0;

    // sound initialization
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("Failed to initialize SDL audio: %s\n", SDL_GetError());
        exit(1);
    }

    if (SDL_LoadWAV("beep.wav", &wav_spec, &wav_buffer, &wav_length) == NULL)
    {
        printf("Failed to load WAV: %s\n", SDL_GetError());
        exit(1);
    }

    device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
    if (device_id == 0)
    {
        printf("Failed to open audio device: %s\n", SDL_GetError());
        exit(1);
    }
}

void loadGame(const char *filename)
{

    FILE *rom;

    rom = fopen(filename, "rb");

    if (rom == NULL)
    {
        printf("The file is not opened\n");
    }
    else
        printf("File opened\n");

    // Reading how many bytes of ROM
    fseek(rom, 0, SEEK_END);
    printf("ROM read\n");

    long rom_size = ftell(rom);
    printf("ROM Size is %ld Bytes\n", rom_size);
    runCycles = rom_size / 2;
    rewind(rom);

    // Copying contents of ROM into memory
    fread(&memory[0x200], 1, rom_size, rom);
    printf("ROM contents copied to memory successfully!\n");
    fclose(rom);
}

void emulateCycle()
{

    opcode = (memory[pc] << 8 | memory[pc + 1]);
    // printf("Memory1: %02X, Memory2: %02X, OPCode: %04X\n ", memory[pc], memory[pc + 1], opcode);

    switch (opcode & 0xF000) // To group according to the first digit of the 2-Byte Hex eg. 0 & F = 0 -> It'll be grouped in 0x0...
    {

    case 0x0000:
        switch (opcode & 0x00FF) // To group according to the first 2 digits of the 2-Byte Hex eg. 00 & FF = 0 -> It'll be grouped in 0x00..
        {

        case 0x00E0: // 0x00E0: Clears the screen
            for (int i = 0; i < 2048; i++)
            {
                gfx[i] = 0;
            }
            printf("Screen cleared!\n");
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x00EE: // 0x00EE: Returns from a subroutine.

            pc = stack[sp];
            sp--;

            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            break;

        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);

            break;
        }
        break;

    case 0x1000: // 0x1NNN: Jumps to address NNN.

        pc = opcode & 0x0FFF;
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode); // 124E ->

        break;

    case 0x2000: // 0x2NNN: Call subroutine at NNN.

        sp++;
        stack[sp] = pc + 2;
        pc = opcode & 0x0FFF;
        printf("case 0x2000  PC: %X, Opcode: %X\n", pc, opcode);
        break;

    case 0x3000: // 0x3xkk: Skip next instruction if Vx == kk.

        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 2;
        }
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);

        pc += 2;
        break;

    case 0x4000: // 0x4xkk: Skip next instruction if Vx != kk.

        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 2;
        }
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0x5000: // 0x5xy0: Skip next instruction if Vx == Vy.

        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0x6000: // 0x6xkk: Sets register VX to the value kk.
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0x7000: // 0x7xkk: Set Vx = Vx + kk.

        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] + (opcode & 0x00FF);
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0x8000: //

        switch (opcode & 0x000F)
        {

        case 0x0000: // 0x8xy1 : Set Vx = Vx OR Vy.

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0001: // 0x8xy1 : Set Vx = Vx OR Vy.

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0002: // 0x8xy2 : Set Vx = Vx AND Vy.

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0003: // 0x8xy3 : Set Vx = Vx XOR Vy.

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0004: // 0x8xy4 :  Set Vx = Vx + Vy, set VF = carry.

            unsigned short add = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];

            if (add > 255)
            {
                V[0xF] = 1;
            }
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] = add & 0xFF;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0005: // 0x8xy5 : Set Vx = Vx - Vy, set VF = NOT borrow.

            unsigned short sub = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4];

            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
            {
                V[0xF] = 1;
            }
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] = sub & 0xFF;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0006: // 0x8xy6 : Set Vx = Vx SHR 1.

            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;

            V[(opcode & 0x0F00) >> 8] >>= 1;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0007: // 0x8xy7 : Set Vx = Vy - Vx, set VF = NOT borrow. (if Vy > Vx)

            if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
            {
                V[0xF] = 1;
            }
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x000E: // 0x8xyE : Set Vx = Vx SHL 1.

            V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;

            V[(opcode & 0x0F00) >> 8] <<= 1;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        default:
            printf("Unhandled 0x8XY* opcode: %X\n", opcode);
            pc += 2; // To prevent infinite loop
            break;
        }

        break;

    case 0x9000: // 0x9xy0 : Skip next instruction if Vx != Vy.

        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0xA000: // 0xAnnn : Set I = nnn.

        I = (opcode & 0x0FFF);
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0xB000: // 0xBnnn : Jump to location nnn + V0.

        pc = (opcode & 0x0FFF) + V[0];
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);

        break;

    case 0xC000: // 0xCxkk : Set Vx = random byte AND kk.

        V[(opcode & 0x0F00) >> 8] = ((clock() & 0xFF) & (opcode & 0x00FF));
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        pc += 2;
        break;

    case 0xD000: // 0xDxyn : Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

        /*

        NOTES:
        1. Variables (x, y, height, pixel)
        2. reset collision flag
        3. main for loop
        4. pixel pointer
        5. inner for loop ( for drawing )
        6. if statement for collision detection
        7. At final, XOR the pixel with


        */

        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;

        for (int row = 0; row < height; row++)
        {
            pixel = memory[I + row];
            for (int col = 0; col < 8; col++)
            {
                if ((pixel & (0x80 >> col)) != 0)
                {
                    int x_coord = (x + col) % 64;
                    int y_coord = (y + row) % 32;
                    int index = y_coord * 64 + x_coord;

                    if (gfx[index] == 1)
                        V[0xF] = 1;

                    gfx[index] ^= 1;
                }
            }
        }

        drawFlag = 1;
        pc += 2;
        printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
        break;

    case 0xE000:

        switch (opcode & 0x00F0)
        {
        case 0x0090: // 0xEx9E : Skip next instruction if key with the value of Vx is pressed.
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);

            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
            {

                pc += 4;
            }
            else
                pc += 2;
            break;

        case 0x00A0: // ExA1 : Skip next instruction if key with the value of Vx is not pressed.

            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
            {

                pc += 4;
            }
            else
                pc += 2;
            break;

        default:
            break;
        }

        break;

    case 0xF000:
        switch (opcode & 0x00FF)
        {

        case 0x07: // Fx07 : Set Vx = delay timer value.

            V[(opcode & 0x0F00) >> 8] = delay_timer;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x0A:
            // Fx0A : Wait for a key press, store the value of the key in Vx.
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);

            bool key_pressed = false;
            for (int i = 0; i < 16; ++i)
            {
                if (key[i])
                { // key[i] is set by SDL key input
                    V[(opcode & 0x0F00) >> 8] = i;
                    key_pressed = true;
                    break;
                }
            }

            if (!key_pressed)
            {
                // Don't increment PC, retry this instruction next cycle
                return;
            }

            // Key was pressed, continue
            pc += 2;
            break;

        case 0x15: // Fx15 : Set delay timer = Vx.

            delay_timer = V[(opcode & 0x0F00) >> 8];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x18: // Fx18 : Set sound timer = Vx.

            sound_timer = V[(opcode & 0x0F00) >> 8];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x1E: // Fx1E : Set I = I + Vx.

            I = I + V[(opcode & 0x0F00) >> 8];
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x29: // Fx29 : Set I = location of sprite for digit Vx.

            I = 0x50 + V[(opcode & 0x0F00) >> 8] * 5;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x33: // Fx33 : Store BCD representation of Vx in memory locations I, I+1, and I+2.

            int Vx = V[(opcode & 0x0F00) >> 8];

            memory[I] = Vx / 100;           // Hundreds digit
            memory[I + 1] = (Vx / 10) % 10; // Tens digit
            memory[I + 2] = Vx % 10;        // Ones digit
                                            // pc += 2;
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x55: // Fx55 : Store registers V0 through Vx in memory starting at location I.

            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
            {
                memory[I + i] = V[i];
            }
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        case 0x65: // Fx65 : Read registers V0 through Vx from memory starting at location I.

            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
            {
                V[i] = memory[I + i];
            }
            printf("  |  PC: %X  |  Opcode: %X  |  \n", pc, opcode);
            pc += 2;
            break;

        default:
            break;
        }
        break;

    default:

        break;
    }
    printf("\n");
}

int main(int argc, char **argv)
{

    initialize();
    if (argc < 2)
    {
        printf("Usage: %s <ROM file>\n", argv[0]);
        return 1;
    }

    loadGame(argv[1]); // Pass the filename as a string

    SDL_Init(SDL_INIT_VIDEO);

    last_timer_tick = SDL_GetTicks();

    SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          0, 0,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             VIDEO_WIDTH, VIDEO_HEIGHT);

    uint32_t pixels[VIDEO_WIDTH * VIDEO_HEIGHT];

    int quit = 0;
    SDL_Event event;

    while (!quit)
    {
        while (SDL_PollEvent(&event)) // SINGLE poll loop
        {
            SDL_Keycode keycode = event.key.keysym.sym;
            int chip8_key;

            switch (event.type)
            {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_KEYDOWN:
                chip8_key = map_sdl_key_to_chip8(keycode);
                if (chip8_key != -1)
                {
                    key[chip8_key] = 1;
                    printf("Key Down: %s (CHIP-8 Key: %X)\n", SDL_GetKeyName(keycode), chip8_key);

                    if (waiting_for_keypress)
                    {
                        V[keypress_target_register] = chip8_key;
                        waiting_for_keypress = false;
                    }
                }
                break;

            case SDL_KEYUP:
                chip8_key = map_sdl_key_to_chip8(keycode);
                if (chip8_key != -1)
                {
                    key[chip8_key] = 0;
                    printf("Key Up: %s (CHIP-8 Key: %X)\n", SDL_GetKeyName(keycode), chip8_key);
                }
                break;
            }
        }

        if (waiting_for_keypress)
        {
            SDL_Delay(2);
            continue;
        }

        for (int i = 0; i < CYCLES_PER_FRAME; i++)
            emulateCycle();

        // 60Hz Timer Tick
        uint32_t now = SDL_GetTicks();
        if (now - last_timer_tick >= 16) // 1000ms / 60Hz ≈ 16ms
        {
            if (delay_timer > 0)
                delay_timer--;

            if (sound_timer > 0)
            {
                sound_timer--;

                SDL_ClearQueuedAudio(device_id);
                SDL_QueueAudio(device_id, wav_buffer, wav_length);
                SDL_PauseAudioDevice(device_id, 0);
            }

            last_timer_tick = now;
        }

        if (drawFlag)
        {
            for (int y = 0; y < VIDEO_HEIGHT; y++)
            {
                for (int x = 0; x < VIDEO_WIDTH; x++)
                {
                    int index = y * VIDEO_WIDTH + x;
                    pixels[index] = gfx[index] ? 0x00FF99FF : 0x001A0FFF;
                }
            }

            int windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            SDL_UpdateTexture(texture, NULL, pixels, VIDEO_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            drawFlag = 0;
        }

        SDL_Delay(16); // Small delay to prevent CPU overuse
    }
    SDL_CloseAudioDevice(device_id);
    SDL_FreeWAV(wav_buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}