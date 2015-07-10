
#include "RCOutput_LDR.h"

using namespace Ldr;

void LdrRCOutput::init(void* machtnichts) {}

void LdrRCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {}

uint16_t LdrRCOutput::get_freq(uint8_t ch) {
    return 50;
}

void LdrRCOutput::enable_ch(uint8_t ch)
{}

void LdrRCOutput::disable_ch(uint8_t ch)
{}

void LdrRCOutput::write(uint8_t ch, uint16_t period_us)
{}

void LdrRCOutput::write(uint8_t ch, uint16_t* period_us, uint8_t len)
{}

uint16_t LdrRCOutput::read(uint8_t ch) {
    return 900;
}

void LdrRCOutput::read(uint16_t* period_us, uint8_t len)
{}

