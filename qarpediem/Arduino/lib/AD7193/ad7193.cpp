#include <Arduino.h>
#include <AD7193.h>
#include <SPI.h>


AD7193::AD7193(uint32_t cs_pin, uint32_t speed){
    SPI.begin();

    config = default_config();
    config.cs_pin = cs_pin;
    config.cread = false;
    config.spi = SPISettings(speed, MSBFIRST, SPI_MODE3);

    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);

    reset();
}


bool AD7193::is_alive(){
    return (get_id() & CHIP_MASKID) == CHIP_ID;
}


uint32_t AD7193::get_id(){
    uint32_t id;
    begin_transaction();
    id = get_register(REG_ID, REGSIZE_ID);
    end_transaction();
    return id;
}


bool AD7193::full_scale_calibration(){
    bool cal_success = false;
    mode_t mode = config.md_mode;

    begin_transaction();
    config.md_mode = MODE_INTFULLCAL;
    set_register(REG_MODE,
                REGSIZE_MODE,
                gen_mode_register(&config));
    if(wait_rdy_low()) cal_success = true;
    end_transaction();

    config.md_mode = mode;
    set_config(&config);

    return cal_success;
}


bool AD7193::calibrate_channel(AD7193::channel_t channel){
    bool cal_success = false;
    int channels = config.cr_channels;
    mode_t mode = config.md_mode;

    setChannelInConfig(&config, channel);
    set_config(&config);

    begin_transaction();
    config.md_mode = MODE_INTZEROCAL;
    set_register(REG_MODE,
                REGSIZE_MODE,
                gen_mode_register(&config));
    if(wait_rdy_low()) cal_success = true;
    end_transaction();

    config.md_mode = mode;
    config.cr_channels = channels;
    set_config(&config);

    return cal_success;
}


bool AD7193::wait_data_available(uint32_t break_time){
    bool status;
    begin_transaction();
    status = wait_rdy_low(break_time);
    end_transaction();
    return status;
}


bool AD7193::wait_rdy_low(uint32_t break_time){
    bool is_low = false;
    uint32_t start = millis();

    while((millis() - start < break_time) && is_low == false){
        is_low = (digitalRead(PIN_MISO) == LOW);
    }

    return is_low;
}

double AD7193::raw_to_millivolts(uint32_t data){
    double max_value = reference_voltage;
    switch(config.cr_gain){
        case GAIN1:     break; // Do nothing
        case GAIN8:		max_value /= 8; break;
        case GAIN16:	max_value /= 16; break;
        case GAIN32:	max_value /= 32; break;
        case GAIN64:	max_value /= 64; break;
        case GAIN128:	max_value /= 128; break;
    }

    if(data != (uint32_t) AD7193_NODATA){
        if(!config.cr_unipolar){
            return max_value * ((double) data * 2 / 0xffffff - 1);
        } else {
            return ((double) data) / 0xffffff * max_value;
        }
    } else {
        return AD7193_NODATA;
    }
}

uint32_t AD7193::reading_raw(AD7193::channel_t* from_channel){
    uint32_t data;
    uint32_t status;

    if(wait_data_available()){
        begin_transaction();

        if(!config.md_dat_sta && from_channel != NULL){
            status = get_register(REG_STATUS, REGSIZE_STATUS);
        }

        write_com_register(COM_READ, REG_DATA);

        data = read_incoming_register(REGSIZE_DATA);
        if(config.md_dat_sta){
            status = read_incoming_register(REGSIZE_STATUS);
        }

        if(from_channel != NULL){
            *from_channel = (AD7193::channel_t) (status & 0xf);
        }

        end_transaction();
    } else {
        data = AD7193_NODATA;
    }


    return data;
}


uint32_t AD7193::reading_from_channel_raw(AD7193::channel_t channel){
    uint32_t data;
    AD7193::channel_t from_channel;

    if(config.cr_channels & (1 << channel)){
        do {
            data = reading_raw(&from_channel);
        } while(data != AD7193_NODATA && from_channel != channel);
    } else {
        data = AD7193_NODATA;
    }

    return data;
}

double AD7193::reading(void){
	return raw_to_millivolts(reading_raw(NULL));
}


double AD7193::reading(AD7193::channel_t* from_channel){
    return raw_to_millivolts(reading_raw(from_channel));
}

double AD7193::reading_from_channel(AD7193::channel_t channel){
    return raw_to_millivolts(reading_from_channel_raw(channel));
}


void AD7193::sync_config(){
    uint32_t confreg;
    uint32_t modereg;

    begin_transaction();
    confreg = get_register(REG_CONFIG, REGSIZE_CONFIG);
    modereg = get_register(REG_MODE, REGSIZE_MODE);
    end_transaction();

    update_with_conf_reg(&config, confreg);
    update_with_mode_reg(&config, modereg);
}


AD7193::config_t AD7193::get_config(){
    return config;
}


void AD7193::set_config(AD7193::config_t* newconfig){
    begin_transaction();
    push_config(newconfig);
    end_transaction();
}


void AD7193::push_config(AD7193::config_t* newconfig){
    uint32_t confreg = gen_config_register(newconfig);
    uint32_t modereg = gen_mode_register(newconfig);

    //write_com_register(AD7193_COM_READ, REG_DATA);
    //read_incoming_register(REGSIZE_DATA);
    set_register(REG_CONFIG, REGSIZE_CONFIG, confreg);
    set_register(REG_MODE, REGSIZE_MODE, modereg);

    config = *newconfig;
}


void AD7193::reset(){
    begin_transaction();
    for(int i = 0; i < 7; i++){
        SPI.transfer(0xff);
    }
    end_transaction();
}


void AD7193::begin_transaction(){
    SPI.beginTransaction(config.spi);
    digitalWrite(config.cs_pin, LOW);
}


void AD7193::end_transaction(){
    digitalWrite(config.cs_pin, HIGH);
    SPI.endTransaction();
}


void AD7193::write_com_register(uint32_t op_mode, ad7193_reg_t reg){
    uint8_t data = 0;
    data |= op_mode;
    data |= AD7193_COM_ADDR(reg);
    data |= config.cread ? COM_CREAD : 0;
    write_register(REGSIZE_COM, (uint32_t) data);
}


uint32_t AD7193::read_incoming_register(size_t size){
    uint8_t buffer[size];
    memset(buffer, 0, size);
    SPI.transfer(buffer, size);
    return buffer_to_register(size, buffer);
}


void AD7193::write_register(size_t size, uint32_t dat){
    for(int i = size - 1; i >= 0; i--){
        SPI.transfer((dat >> (i * 8)) & 0xff);
    }
}


void AD7193::set_register(ad7193_reg_t reg, size_t size, uint32_t dat){
    write_com_register(COM_WRITE, reg);
    write_register(size, dat);
}


uint32_t AD7193::get_register(ad7193_reg_t reg, size_t size){
    write_com_register(COM_READ, reg);
    return read_incoming_register(size);
}


void AD7193::enable_channel(channel_t channel){
    uint32_t channel_flag = 1 << channel;
    if((channel_flag & config.cr_channels) == 0){
        config.cr_channels |= channel_flag;
        set_config(&config);
    }
}

void AD7193::disable_channel(channel_t channel){
	uint32_t channel_flag = 1 << channel;
	if((channel_flag & config.cr_channels) != 0){
		config.cr_channels &= ~channel_flag;
        set_config(&config);
	}
}

void AD7193::disable_all_channels(){
	if(config.cr_channels != 0){
		config.cr_channels = 0;
		set_config(&config);
	}
}

void AD7193::set_mode(AD7193::mode_t mode){
	if(config.md_mode != mode){
		config.md_mode = mode;
		set_config(&config);
	}
}

void AD7193::set_polarity(AD7193::polarity_t polarity){
    if(polarity == UNIPOLAR && !config.cr_unipolar){
        config.cr_unipolar = true;
        set_config(&config);
    } else if(polarity == BIPOLAR && config.cr_unipolar){
        config.cr_unipolar = true;
        set_config(&config);
    }
}


void AD7193::set_gain(gain_t gain){
    if(config.cr_gain != (uint8_t) gain){
        config.cr_gain = (uint8_t) gain;
        set_config(&config);
		full_scale_calibration();
    }
}


void AD7193::set_difftype(difftype_t type){
    if(type == DIFF_PSEUDO && !config.cr_pseudo){
        config.cr_pseudo = true;
        set_config(&config);
    } else if(type == DIFF_FULLY && config.cr_pseudo){
        config.cr_pseudo = false;
        set_config(&config);
    }
}


void AD7193::set_chop(bool chop){
    if(config.cr_chop != chop){
        config.cr_chop = chop;
        set_config(&config);
    }
}


void AD7193::set_rej60(bool rej60){
    if(config.md_rej60 != rej60){
        config.md_rej60 = rej60;
        set_config(&config);
    }
}


void AD7193::set_reference(ref_t reference){
    if(reference == REFIN1 && config.cr_refsel){
        config.cr_refsel = false;
        set_config(&config);
    } else if(reference == REFIN2 && !config.cr_refsel){
        config.cr_refsel = true;
        set_config(&config);
    }
}


void AD7193::set_reference_voltage(double voltage_mv){
	reference_voltage = voltage_mv;
}


AD7193::config_t AD7193::default_config(){
    AD7193::config_t default_config;

    // Configuration Register Power-on/Reset status
    default_config.cr_chop = false;
    default_config.cr_refsel = false;
    default_config.cr_pseudo = false;
    setChannelInConfig(&default_config, CHAN0);
    default_config.cr_burn = false;
    default_config.cr_refdet = false;
    default_config.cr_buf = true;
    default_config.cr_unipolar = false;
    default_config.cr_gain = 7;

    // Mode register
    default_config.md_mode = MODE_CONTCONV;
    default_config.md_dat_sta = false;
    default_config.md_clk = CLK_INT;
    default_config.md_averaging = 0;
    default_config.md_sinc3 = false;
    default_config.md_enpar = false;
    default_config.md_clk_div = false;
    default_config.md_single = false;
    default_config.md_rej60 = false;
    default_config.md_filter_output = 96;

    return default_config;
}


void AD7193::update_with_conf_reg(AD7193::config_t* config, uint32_t reg){
    config->cr_chop = (reg >> 23) & 1;
    config->cr_refsel = (reg >> 20) & 1;
    config->cr_pseudo = (reg >> 18) & 1;
    config->cr_channels = (reg >> 8) & 0x3ff;
    config->cr_burn = (reg >> 7) & 1;
    config->cr_refdet = (reg >> 6) & 1;
    config->cr_buf = (reg >> 4) & 1;
    config->cr_unipolar = (reg >> 3) & 1;
    config->cr_gain = reg & 7;
}


void AD7193::update_with_mode_reg(AD7193::config_t* config, uint32_t reg){
    config->md_mode = (mode_t) ((reg >> 21) & 7);
    config->md_dat_sta = (reg >> 20) & 1;
    config->md_clk = (clk_t) ((reg >> 18) & 3);
    config->md_averaging = (reg << 16) & 3;
    config->md_sinc3 = (reg >> 15) & 1;
    config->md_enpar = (reg >> 13) & 1;
    config->md_clk_div = (reg >> 12) & 1;
    config->md_single = (reg >> 11) & 1;
    config->md_rej60 = (reg >> 10) & 1;
    config->md_filter_output = (reg) & 0x3ff;
}


uint32_t AD7193::gen_config_register(AD7193::config_t* config){
    uint32_t reg = 0;

    reg |= (uint32_t) config->cr_chop << 23;
    reg |= (uint32_t) config->cr_refsel << 20;
    reg |= (uint32_t) config->cr_pseudo << 18;
    reg |= (uint32_t) config->cr_channels << 8;
    reg |= (uint32_t) config->cr_burn << 7;
    reg |= (uint32_t) config->cr_refdet << 6;
    reg |= (uint32_t) config->cr_buf << 4;
    reg |= (uint32_t) config->cr_unipolar << 3;
    reg |= (uint32_t) config->cr_gain;

    return reg;
}


uint32_t AD7193::gen_mode_register(AD7193::config_t* config){
    uint32_t reg = 0;

    reg |= (uint32_t) config->md_mode << 21;
    reg |= (uint32_t) config->md_dat_sta << 20;
    reg |= (uint32_t) config->md_clk << 18;
    reg |= (uint32_t) config->md_averaging << 16;
    reg |= (uint32_t) config->md_sinc3 << 15;
    reg |= (uint32_t) config->md_enpar << 13;
    reg |= (uint32_t) config->md_clk_div << 12;
    reg |= (uint32_t) config->md_single << 11;
    reg |= (uint32_t) config->md_rej60 << 10;
    reg |= (uint32_t) config->md_filter_output;

    return reg;
}


uint32_t AD7193::buffer_to_register(size_t size, uint8_t* buffer){
    uint32_t data = 0;
    for(size_t i = 0; i < size; i++){
        data <<= 8;
        data |= buffer[i];
    }
    return data;
}


void AD7193::setChannelInConfig(AD7193::config_t* c, AD7193::channel_t chan){
    c->cr_channels = 1 << chan;
}



void AD7193::addChannelToConfig(AD7193::config_t* c, AD7193::channel_t chan){
    c->cr_channels |= 1 << chan;
}


void AD7193::removeChannelFromConfig(AD7193::config_t *c, AD7193::channel_t chan){
    c->cr_channels &= ~(1 << chan);
}
