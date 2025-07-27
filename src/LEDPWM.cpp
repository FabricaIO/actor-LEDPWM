#include "LEDPWM.h"

/// @brief Creates a LEDPWM object
/// @param Name The device name
/// @param Pin The pin to use
/// @param ledcChannel The LEDC channel to use
/// @param configFile The name of the configuration to use
LEDPWM::LEDPWM(String Name, int Pin, int ledcChannel, String configFile) : Actor(Name) {
	LEDPWM_config.Pin = Pin;
	LEDPWM_config.ledc_channel = ledcChannel;
	config_path =  "/settings/act/" + configFile;
}

/// @brief Starts the LEDPWM object
/// @return True on success
bool LEDPWM::begin() {
	// Set description
	Description.actionQuantity = 2;
	Description.type = "output";
	Description.actions = {{"state", 0}, {"dutycycle", 1}};
	// Create settings directory if necessary
	bool result - false;
	if (!checkConfig(config_path)) {
		// Set defaults
		result = saveConfig(config_path, getConfig());
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path), false);
	}
	// Set initial duty cycle
	if (result) {
		SetDutyCycle(LEDPWM_config.dutyCycle);
	}
	return result;
}

/// @brief Receives an action
/// @param action The action to process (0 for set state, 1 for set duty cycle)
/// @param payload A 0 or 1 to turn LED off or on, or a duty cycle value
/// @return JSON response with OK or error message
std::tuple<bool, String> LEDPWM::receiveAction(int action, String payload) {
	if (action == 0) {
		if (payload == "0") {
			SetDutyCycle(0);
		} else if (payload == "1") {
			SetDutyCycle(LEDPWM_config.dutyCycle);
		} else {
			return { true, R"({"success": false, "Response": "Invalid payload"})" };
		}
		return { true, R"({"Response": "OK"})" };
	} else if (action == 1) {
		int duty = payload.toInt();
		if (duty < 0 || duty > (1 << LEDPWM_config.ledc_resolution) - 1) {
			return { true, R"({"success": false, "Response": "Duty cycle out of range"})" };
		}
		LEDPWM_config.dutyCycle = duty;
		if (LEDPWM_config.saveDutyCycle) {
			// Save duty cycle to config
			saveConfig(config_path, getConfig());
		}
		SetDutyCycle(LEDPWM_config.dutyCycle);		
		return { true, R"({"success": true})" };
	}
	return { true, R"({"success": false, "Response": "Invalid action"})" };
}

/// @brief Gets the current config
/// @return A JSON string of the config
String LEDPWM::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["Name"] = Description.name;
	doc["Pin"] = LEDPWM_config.Pin;
	doc["saveDutyCycle"] = LEDPWM_config.saveDutyCycle;
	doc["dutyCycle"] = LEDPWM_config.dutyCycle;
	doc["ledc_channel"] = LEDPWM_config.ledc_channel;
	doc["ledc_resolution"] = LEDPWM_config.ledc_resolution;
	doc["ledc_frequency"] = LEDPWM_config.ledc_frequency;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool LEDPWM::setConfig(String config, bool save) {
	// Detach pin in case it changes
	ledcDetachPin(LEDPWM_config.Pin);
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	// Assign loaded values
	Description.name = doc["Name"].as<String>();
	LEDPWM_config.Pin = doc["Pin"].as<int>();
	LEDPWM_config.saveDutyCycle = doc["saveDutyCycle"].as<bool>();
	LEDPWM_config.dutyCycle = doc["dutyCycle"].as<uint32_t>();
	LEDPWM_config.ledc_channel = doc["ledc_channel"].as<uint8_t>();
	LEDPWM_config.ledc_resolution = doc["ledc_resolution"].as<int>();
	LEDPWM_config.ledc_frequency = doc["ledc_frequency"].as<int>();

	if (save) {
		if (!saveConfig(config_path, config)) {
			return false;
		}
	}
	return configureOutput();
}

/// @brief Configures LEDC output
/// @return True on success
bool LEDPWM::configureOutput() {
	if (ledcSetup(LEDPWM_config.ledc_channel, LEDPWM_config.ledc_frequency, LEDPWM_config.ledc_resolution) != 0) {
		// Attach pin to LEDC channel
		ledcAttachPin(LEDPWM_config.Pin, LEDPWM_config.ledc_channel);
		return true;
	}
	return false;
}

/// @brief Sets the duty cycle on the LEDC channel
/// @param cycle The duty cycle to set
/// @return True on success
bool LEDPWM::SetDutyCycle(uint32_t cycle) {
	ledcWrite(LEDPWM_config.ledc_channel, cycle);
	return true;
}