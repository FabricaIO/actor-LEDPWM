/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Actor.h>

/// @brief Allows the use of the LEDC module for controling LED brightness via PWM 
class LEDPWM : public Actor {
	public:
		/// @brief LEDPWM settings
		struct {
			/// @brief Pin of channel
			int Pin;

			/// @brief If the duty cycle should be saved to the configuration file when updated
			bool saveDutyCycle = false;

			/// @brief Duty cycle of the LED strand
			uint32_t dutyCycle = 127;

			/// @brief The LEDC channel in use
			uint8_t ledc_channel = 0;

			/// @brief Resolution of LEDC in bits
			int ledc_resolution = 8;

			/// @brief Frequency of LEDC in Hz
			int ledc_frequency = 8000;
		} LEDPWM_config;

		LEDPWM(String Name, int Pin, int ledcChannel = 0, String configFile = "LEDPWM.json");
		bool begin();
		std::tuple<bool, String> receiveAction(int action, String payload);
		String getConfig();
		bool setConfig(String config, bool save);

	protected:
		/// @brief Full path of configuration file
		String config_path;

		bool configureOutput();
		bool SetDutyCycle(uint32_t cycle);
};