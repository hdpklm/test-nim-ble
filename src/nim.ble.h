#ifndef NIM_BLE_H_
#define NIM_BLE_H_ "NIM_BLE_H_"

#include "NimBLEDevice.h"

#define NIM_BLE_NAME "test nim"
#define NIM_BLE_POWER ESP_PWR_LVL_P9
#define NIM_BLE_MAX_SERVICES 10

#define service_public "1233"
#define charact_cmd "0001"
#define charact_log "0002"
#define charact_measure "0003"

// para la pipa solo
NimBLEService *mService;
NimBLECharacteristic *chr_cmd;
NimBLECharacteristic *chr_log;
NimBLECharacteristic *chr_measure;
#define ble_cmd_stream_len 1024
char ble_cmd_stream[ble_cmd_stream_len] = "";

struct nimData
{
	int freq;
	float real;
	float imag;
};

#define GLOBAL_DENDING_LENGTH 200
int global_sending_index = 0;
bool global_sending_data = false;
nimData global_sending_arr[GLOBAL_DENDING_LENGTH];

void ble_send_log(const char *);
void ble_send_cmd(const char *);
void ble_send_measure(const char *);

int nim_services_index = 0;
static NimBLEServer *ble_server;
static NimBLEService *nim_services_array[NIM_BLE_MAX_SERVICES];

int ble_available();
int ble_read(char *, int);
void nim_ble_set_security();
void nim_ble_create_server();
void nim_ble_service_init_done();
uint16_t nim_ble_decode_props(char *props_c);
NimBLEService *nim_ble_create_service(char *uuid);
NimBLECharacteristic *getCharacteristic(const char *service, const char *charact);
NimBLECharacteristic *nim_ble_create_charact(NimBLEService *pService, char *charact, char *props_c);

class ServerCallbacks : public NimBLEServerCallbacks
{
	void onConnect(NimBLEServer *pServer)
	{
		Serial.println("Client connected");
		Serial.println("Multi-connect support: start advertising");
		NimBLEDevice::startAdvertising();
	};
	void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
	{
		Serial.print("Client address: ");
		Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
		/** We can use the connection handle here to ask for different connection parameters.
		 *  Args: connection handle, min connection interval, max connection interval
		 *  latency, supervision timeout.
		 *  Units; Min/Max Intervals: 1.25 millisecond increments.
		 *  Latency: number of intervals allowed to skip.
		 *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
		 */
		pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
		// pServer->updateConnParams(desc->conn_handle, 25, 90, 0, 1000);
	};
	void onDisconnect(NimBLEServer *pServer)
	{
		Serial.println("Client disconnected - start advertising");
		NimBLEDevice::startAdvertising();
	};
	void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
	{
		Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
	};

	uint32_t onPassKeyRequest()
	{
		Serial.println("Server Passkey Request");
		/** This should return a random 6 digit number for security
		 *  or make your own static passkey as done here.
		 */
		return 123456;
	};

	bool onConfirmPIN(uint32_t pass_key)
	{
		Serial.print("The passkey YES/NO number: ");
		Serial.println(pass_key);
		/** Return false if passkeys don't match. */
		return true;
	};

	void onAuthenticationComplete(ble_gap_conn_desc *desc)
	{
		/** Check that encryption was successful, if not we disconnect the client */
		if (!desc->sec_state.encrypted)
		{
			NimBLEDevice::getServer()->disconnect(desc->conn_handle);
			Serial.println("Encrypt connection failed - disconnecting client");
			return;
		}
		Serial.println("Starting BLE work!");
	};
};

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
	void onRead(NimBLECharacteristic *pCharacteristic)
	{
		char *read = (char *)pCharacteristic->getValue().c_str();
		Serial.print(pCharacteristic->getUUID().toString().c_str());
		Serial.print(": onRead(), value: ");
		Serial.println(read);
	};

	void onWrite(NimBLECharacteristic *pCharacteristic)
	{
		Serial.print(pCharacteristic->getUUID().toString().c_str());
		Serial.print(": onWrite(), value: ");
		char *read = (char *)pCharacteristic->getValue().c_str();
		Serial.println(read);
		uint16_t len = strlen(read);
		if (len == 0)
		{
			Serial.println("Empty command");
			return;
		}

		int len_cmd = strlen(ble_cmd_stream);
		int max_len = ble_cmd_stream_len - 2;
		if (len + len_cmd >= max_len)
		{
			Serial.println("ble_cmd_stream is full");
			return;
		}

		strcat(ble_cmd_stream, read);
		strcat(ble_cmd_stream, "\n");

		// if (read[0] == 't')
		// {
		// 	Serial.println("Send Measure");
		// 	for (int i = 0; i < GLOBAL_DENDING_LENGTH; i++)
		// 	{
		// 		float r1 = rand();
		// 		float r2 = rand();
		// 		r1 = r1 == 0 ? 1 : 1 / r1;
		// 		r2 = r2 == 0 ? 1 : 1 / r2;

		// 		// global_sending_arr[i].freq = i;
		// 		// global_sending_arr[i].real = r1;
		// 		// global_sending_arr[i].imag = r2;

		// 		nimData n = global_sending_arr[global_sending_index];
		// 		String str = String(i) + "," + r1 + "," + r2;
		// 		char *buff = (char *)str.c_str();
		// 		Serial.printf("send row; [%s]\n", buff);

		// 		send_log("global_sending_arr[global_sending_index]");
		// 		// global_sending_data = true;
		// 		// while (!global_sending_data)
		// 		// {
		// 		// }
		// 		delay(10);
		// 	}

		// 	// global_sending_data = true;
		// 	// global_sending_index = 0;
		// 	send_measure("end");
		// }

		// Serial.printf("Send Log: [%s]\n", read);
		// send_log(read);
	};
	/** Called before notification or indication is sent,
	 *  the value can be changed here before sending if desired.
	 */
	void onNotify(NimBLECharacteristic *pCharacteristic)
	{
		Serial.println("Sending notification to clients");
	};

	/** The status returned in status is defined in NimBLECharacteristic.h.
	 *  The value returned in code is the NimBLE host return code.
	 */
	void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code)
	{
		String str = ("Notification/Indication status code: ");
		str += status;
		str += ", return code: ";
		str += code;
		str += ", ";
		str += NimBLEUtils::returnCodeToString(code);
		Serial.println(str);

		global_sending_data = false;
	};

	void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
	{
		String str = "Client ID: ";
		str += desc->conn_handle;
		str += " Address: ";
		str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
		if (subValue == 0)
		{
			str += " Unsubscribed to ";
		}
		else if (subValue == 1)
		{
			str += " Subscribed to notfications for ";
		}
		else if (subValue == 2)
		{
			str += " Subscribed to indications for ";
		}
		else if (subValue == 3)
		{
			str += " Subscribed to notifications and indications for ";
		}
		str += std::string(pCharacteristic->getUUID()).c_str();

		Serial.println(str);
	};
};

static CharacteristicCallbacks chrCallbacks;

void nim_ble_init()
{
	NimBLEDevice::init(NIM_BLE_NAME);

	/** Optional: set the transmit power, default is 3db */
	NimBLEDevice::setPower(NIM_BLE_POWER); /** +9db */
	// NimBLEDevice::setMTU(550);

	nim_ble_set_security();
	nim_ble_create_server();

	mService = nim_ble_create_service(service_public);
	chr_cmd = nim_ble_create_charact(mService, charact_cmd, "rwni");
	chr_log = nim_ble_create_charact(mService, charact_log, "rwni");
	chr_measure = nim_ble_create_charact(mService, charact_measure, "rwni");

	chr_cmd->setCallbacks(&chrCallbacks);
	chr_log->setCallbacks(&chrCallbacks);
	chr_measure->setCallbacks(&chrCallbacks);

	nim_ble_service_init_done();
}

void ble_send_log(const char *log)
{
	chr_log->setValue((uint8_t *)log, strlen(log));
	chr_log->notify();
}

void ble_send_cmd(const char *cmd)
{
	chr_cmd->setValue((uint8_t *)cmd, strlen(cmd));
	chr_cmd->notify();
}

void ble_send_measure(const char *measure)
{
	chr_measure->setValue((uint8_t *)measure, strlen(measure));
	chr_measure->notify(true);
}

void nim_ble_service_init_done()
{
	NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

	for (int i = 0; i < nim_services_index; i++)
	{
		NimBLEService *pService = nim_services_array[i];

		/** Start the services when finished creating all Characteristics and Descriptors */
		pService->start();

		/** Add the services to the advertisment data **/
		pAdvertising->addServiceUUID(pService->getUUID());
	}

	/** If your device is battery powered you may consider setting scan response
	 *  to false as it will extend battery life at the expense of less data sent. */
	pAdvertising->setScanResponse(true);
	pAdvertising->start();
}

void nim_ble_set_security()
{
	/** Set the IO capabilities of the device, each option will trigger a different pairing method.
	 *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
	 *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
	 *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
	 */
	// NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
	// NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

	/** 2 different ways to set security - both calls achieve the same result.
	 *  no bonding, no man in the middle protection, secure connections.
	 *
	 *  These are the default values, only shown here for demonstration.
	 */
	// NimBLEDevice::setSecurityAuth(false, false, true);
	NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
}

void nim_ble_create_server()
{
	ble_server = NimBLEDevice::createServer();
	ble_server->setCallbacks(new ServerCallbacks());
}

NimBLEService *nim_ble_create_service(char *service)
{
	NimBLEService *pService = ble_server->createService(NimBLEUUID(service));
	nim_services_array[nim_services_index++] = pService;

	return pService;
}

NimBLECharacteristic *nim_ble_create_charact(NimBLEService *pService, char *charact, char *props_c)
{
	uint16_t props = nim_ble_decode_props(props_c);
	NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(charact, props);

	pCharacteristic->setCallbacks(&chrCallbacks);

	return pCharacteristic;
}

uint16_t nim_ble_decode_props(char *props_c)
{
	uint16_t props = 0;
	for (int i = 0; i < 4; i++)
	{
		char p = props_c[i];
		ESP_LOGV(NIM_BLE_H_, "nim_ble_decode_props: find char: %c", p);

		switch (p)
		{
		case 'r':
			props |= NIMBLE_PROPERTY::READ;
			break;
		case 'w':
			props |= NIMBLE_PROPERTY::WRITE;
			break;
		case 'R':
			props |= NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ;
			break;
		case 'W':
			props |= NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE;
			break;
		case 'n':
			props |= NIMBLE_PROPERTY::NOTIFY;
			break;
		case 'i':
			props |= NIMBLE_PROPERTY::INDICATE;
			break;
		}
	}

	return props;
}

NimBLECharacteristic *getCharacteristic(const char *service, const char *charact)
{
	if (ble_server->getConnectedCount())
	{
		NimBLEService *pSvc = ble_server->getServiceByUUID(service);
		if (pSvc)
		{
			NimBLECharacteristic *pChr = pSvc->getCharacteristic(charact);
			if (pChr)
			{
				return pChr;
			}
		}
	}

	return nullptr;
}

int ble_available()
{
	char *pos = strchr(ble_cmd_stream, '\n');
	uint16_t len = pos ? pos - ble_cmd_stream : 0;
	return len;
}

int ble_read(char *buff, int count)
{
	uint16_t len = ble_available();
	if (len < count)
		return 0;

	memcpy(buff, ble_cmd_stream, len);
	memmove(ble_cmd_stream, ble_cmd_stream + len, strlen(ble_cmd_stream) - len);

	return len;
}

#endif