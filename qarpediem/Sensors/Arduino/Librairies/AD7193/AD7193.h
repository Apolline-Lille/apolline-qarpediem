#ifndef __AD7193_H__
#define __AD7193_H__

#include <Arduino.h>
#include <SPI.h>

// AD7193 use 24-bit register. Therefore, it can't return this value
#define AD7193_NODATA       0xf0f0f0f0
#define AD7193_COM_ADDR(x)  (x & 0x7) << 3  // Register address

class AD7193 {
public:
	/* Public types definition */
	typedef enum {
		CHAN0 = 0,
		CHAN1 = 1,
		CHAN2 = 2,
		CHAN3 = 3,
		CHAN4 = 4,
		CHAN5 = 5,
		CHAN6 = 6,
		CHAN7 = 7,
		CHANTEMP = 8,
		CHANSHORT = 9,
	} channel_t;

	typedef enum {
		MODE_CONTCONV = 0,
		MODE_SINGCONV = 1,
		MODE_IDLE = 2,
		MODE_PWDOWN = 3,
		MODE_INTZEROCAL = 4,
		MODE_INTFULLCAL = 5,
		MODE_SYSZEROCAL = 6,
		MODE_SYSFULLCAL = 7
	} mode_t;

	typedef enum {
		CLK_EXTK1K2 = 0,
		CLK_EXTK2 = 1,
		CLK_INT = 2,
		CLK_INTK2 = 3
	} clk_t;

	typedef enum {
		BIPOLAR,
		UNIPOLAR
	} polarity_t;

	typedef enum {
		GAIN1 = 0,
		GAIN8 = 3,
		GAIN16 = 4,
		GAIN32 = 5,
		GAIN64 = 6,
		GAIN128 = 7
	} gain_t;

	typedef enum {
		DIFF_PSEUDO,
		DIFF_FULLY
	} difftype_t;

	typedef enum {
		REFIN1,
		REFIN2
	} ref_t;

	static const uint32_t COM_READ = 1 << 6;
	static const uint32_t COM_WRITE = 0;
	static const uint32_t COM_CREAD = 1 << 2; // Enable continuous read

	typedef struct {
		SPISettings spi;
		bool cread;
		uint32_t cs_pin;

		// Configuration Register
		bool cr_chop;
		bool cr_refsel;
		bool cr_pseudo;
		uint32_t cr_channels;
		bool cr_burn;
		bool cr_refdet;
		bool cr_buf;
		bool cr_unipolar;
		uint8_t cr_gain;

		// Mode register
		mode_t md_mode;
		bool md_dat_sta;
		clk_t md_clk;
		uint8_t md_averaging;
		bool md_sinc3;
		bool md_enpar;
		bool md_clk_div;
		bool md_single;
		bool md_rej60;
		uint16_t md_filter_output;
	} config_t;

	/**
	 * Init the AD7193 and the SPI hardware
	 * @param cs_pin Chip Select PIN
	 * @param speed  Clock speed (in Hz)
	 */
	AD7193(uint32_t cs_pin, uint32_t speed = 4000000);

	/**
	 * Perform a interior full scale calibration (cf. AD7193 documentation)
	 * @return True if success
	 */
	bool full_scale_calibration(void);

	/**
	 * Calibrate a channel
	 * @param  channel The channel that will be calibrate
	 * @return         True if calibration was successfull
	 */
	bool calibrate_channel(channel_t channel);

	/**
	 * Wait for data to be available on the AD7193
	 * @param  break_time Timeout
	 * @return True if data is available, false otherwise
	 */
	bool wait_data_available(uint32_t break_time = 10000);

	/**
	 * Get voltage applied to a channel. You can use this variant if you have
	 * only one selected channel or if you don't care from which channel
	 * the data come from.
	 * @method reading
	 * @return voltage applied to a channel or AD7193_NODATA if the ADC is not
	 * responding
	 */
	double reading(void);

	/**
	 * Get voltage applied to a channel
	 * @method reading
	 * @param  from_channel if not NULL, return the channel the data is from
	 * @return voltage applied to channel in from_channel or AD7193_NODATA if
	 *                 no data is returned by the ADC
	 */
	double reading(channel_t* from_channel);

	/**
	 * Get voltage applied to channel
	 * @method reading_from_channel
	 * @param  channel
	 * @return voltage applied to channel or AD7193_NODATA if the channel is not
	 * enabled or if the ADC doesn't answer
	 */
	double reading_from_channel(channel_t channel);

	/**
	 * Get raw data returned by the AD7193 for a channel
	 * @method reading_raw
	 * @param  from_channel if not null, return the channel the data is coming
	 *                      from
	 * @return raw data returned by the ADC (on 24-bit) or AD7193_NODATA if no
	 * data is available
	 */
	uint32_t reading_raw(channel_t* from_channel);

	/**
	 * Get raw data returned by the AD7193 for channel
	 * @method reading_from_channel_raw
	 * @param channel
	 * @return raw data returned by the ADC for channel (on 24-bit)
	 * or AD7193_NODATA if no data is available
	 */
	uint32_t reading_from_channel_raw(channel_t channel);

	/**
	 * Convert data from the data register into millivolts
	 * @param  data Data from the Data Register of the ADC
	 * @return      Value in millivolts
	 */
	double raw_to_millivolts(uint32_t data);

	/**
	 * Return the ID of the AD7193
	 * @return id of the ADC. If it return 0, the ADC is not responding.
	 */
	uint32_t get_id(void);

	/**
	 * Check if the ADC is answering correctly by checking the ID returned
	 * @method is_alive
	 * @return true if the ADC is alive and is responding correctly
	 */
	bool is_alive(void);

	void enable_channel(channel_t channel);
	void disable_channel(channel_t channel);
	void disable_all_channels();
	void set_polarity(polarity_t polarity);
	void set_gain(gain_t gain);
	void set_mode(mode_t mode);
	void set_difftype(difftype_t type);
	void set_chop(bool chop);
	void set_rej60(bool rej60);
	void set_reference(ref_t reference);

	/**
	 * Set reference voltage that will be used when converting into millivolts
	 * @param voltage_mv Reference voltage in millivolts
	 */
	void set_reference_voltage(double voltage_mv);

	/**
	 * Synchronize stored configuration with the one stored in the ADC
	 */
	void sync_config(void);

	/**
	 * Get the ADC7193 current stored configuration
	 * @param sync Synchronize current configuration with the one from the ADC
	 */
	config_t get_config(void);

	/**
	 * Apply config to the ADC
	 * @param config New configuration of the ADC
	 */
	void set_config(config_t* config);

	/**
	 * Set/Add/Remove channel in config
	 */
	static void addChannelToConfig(config_t* c, channel_t chan);
	static void removeChannelFromConfig(config_t* c, channel_t chan);
	static void setChannelInConfig(config_t* c, channel_t chan);

	/**
	 * Reset the AD7193
	 */
	void reset(void);

private:
	/* Private type definition */
	typedef enum {
		REG_COM = 0,     // Communcation Register (WO, 8-bit)
		REG_STATUS = 0,  // Status Register (RO, 8-bit)
		REG_MODE = 1,    // Mode Register (RW, 24-bit)
		REG_CONFIG = 2,  // Configuration Register (RW, 24-bit)
		REG_DATA = 3,    // Data Register (RO, 24-bit)
		REG_ID = 4,      // ID Register (RO, 8-bit)
		REG_OFFSET = 6,  // Offset Register (RW, 24-bit)
		REG_FLSCALE = 7, // Full-Scale Register (RW, 24-bit)
	} ad7193_reg_t;

	static const size_t REGSIZE_COM = 1;
	static const size_t REGSIZE_STATUS = 1;
	static const size_t REGSIZE_MODE = 3;
	static const size_t REGSIZE_CONFIG = 3;
	static const size_t REGSIZE_DATA = 3;
	static const size_t REGSIZE_ID = 1;
	static const size_t REGSIZE_OFFSET = 3;
	static const size_t REGSIZE_FLSCALE = 3;

	/* Constant definition */
	static const uint8_t PIN_MISO = MISO;
	static const uint8_t CHIP_ID = 2;
	static const uint8_t CHIP_MASKID = 0x0F;

	config_t config;
	double reference_voltage = 2500;

	/**
	 * Start and end a transaction (SPI and CS PIN management)
	 */
	void begin_transaction(void);
	void end_transaction(void);

	/**
	 * Get the content of a register
	 * @param reg     Targetted register
	 * @param regSize Size of the register
	 * @return Register content
	 */
	uint32_t get_register(ad7193_reg_t reg, size_t regSize);

	/**
	 * Set the content of a register
	 * @param reg  Targetted register
	 * @param size Size of the register
	 * @param buf  New register content (will be overwritten)
	 */
	void set_register(ad7193_reg_t reg, size_t size, uint32_t dat);


	/**
	 * Wait for the RDY pin to go low
	 * @param  break_time Timeout
	 * @return           True if data is available, false otherwise
	 */
	bool wait_rdy_low(uint32_t break_time = 10000);

	/**
	 * Write an operation in the Communcation Register
	 * @param op_mode Read/Write
	 * @param reg     Register targetted for the operation
	 */
	void write_com_register(uint32_t op_mode, ad7193_reg_t reg);

	/**
	 * Read an incoming register on the SPI bus
	 * @param size Size of the register in bytes
	 */
	uint32_t read_incoming_register(size_t size);

	/**
	 * Write a register in the SPI bus
	 * @param size Size of the register
	 * @param dat  Data to send
	 */
	void write_register(size_t size, uint32_t dat);

	/**
	 * Return default configuration using default value for registers
	 */
	static config_t default_config(void);

	/**
	 * Push config in the AD7193
	 */
	void push_config(config_t* newconfig);

	/**
	 * Generate Configuration Register value from config
	 */
	static uint32_t gen_config_register(config_t* config);

	/**
	 * Generate a Mode Register value from config
	 */
	static uint32_t gen_mode_register(config_t* config);

	/**
	 * Update configuration with the given Configuration Register value
	 */
	static void update_with_conf_reg(config_t* config, uint32_t reg);

	/**
	 * Update configuration with the given Configuration Mode value
	 */
	static void update_with_mode_reg(config_t* config, uint32_t reg);

	/**
	 * Convert a buffer received through SPI to a register
	 */
	static uint32_t buffer_to_register(size_t size, uint8_t* buffer);
};

#endif // __AD7193_H__
