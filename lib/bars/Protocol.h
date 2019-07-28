/*
 * Protocol.h
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_
/*
 * Equipmenttypes
 */
enum equipmentTypes : unsigned int {
	None = 48,
	All = 49,
	Light = 50,
	Control = 51
};

enum messageTypes :  unsigned int{
	SubscribeMessage = '0',
	SetMessage = '1',
	SyncMessage = '2',
	PushMessage = '3',
	StatusMessage = '4'
};

/*
 * Hier sind die Nachrichtentypen definiert.
 * Der Receivebuffer kann mit den folgenden Methoden auf
 * den Nachrichtentyp überprüft werden.
 */
inline bool subscribing(char* buffer, size_t length){
	if(length >= 1 && buffer[0] == SubscribeMessage){
		return true;
	}
	return false;
}

inline bool setting(char* buffer, size_t length){
	if(length >= 1 && buffer[0] == SetMessage){
		return true;
	}
	return false;
}

inline bool synchronising(char* buffer, size_t length){
	if(length >= 1 && buffer[0] == SyncMessage){
		return true;
	}
	return false;
}

inline bool pushing(char* buffer, size_t length){
	if(length >= 1 && buffer[0] == PushMessage){
		return true;
	}
	return false;
}

inline bool statusing(char* buffer, size_t length){
	if(length >= 1 && buffer[0] == StatusMessage){
		return true;
	}
	return false;
}

/*
 * Nachrichtenschablonen, die zum effizienten Erzeugen und
 * Auslesen der Nachrichten genutzt werden können.
 */

struct subscribeMessage {
	union {
		struct{
			char messageType;
			char equipmentType;
		};
		char buffer[2];
	};
	size_t maxSize = 2;
	inline subscribeMessage(){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = '0';
		this->buffer[0] = SubscribeMessage;
	}
	inline subscribeMessage(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
		this->buffer[0] = SubscribeMessage;
	}
	inline subscribeMessage& operator= (const subscribeMessage& mesg){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = mesg.buffer[i];
		return *this;
	}
	inline void create(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
	}
};

struct settingMessage {
	union {
		struct{
			char messageType;
			char basePattern;
			char baseColor;
			char baseSpeed;
			char baseBrightness;
			char frontPattern;
			char frontColor;
			char frontSpeed;
			char frontBrightness;
			char strobePattern;
			char strobeColor;
			char strobeSpeed;
			char strobeBrightness;
			char mainDim;
			char dutyCycle;
		};
		char buffer[15];
	};
	size_t maxSize = 15;
	inline settingMessage(){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = '0';
		this->buffer[0] = SetMessage;
	}
	inline settingMessage(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
			this->buffer[0] = SetMessage;
		}
	}
	inline settingMessage& operator= (const settingMessage& mesg){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = mesg.buffer[i];
		return *this;
	}
	inline void create(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
	}
};

struct synchronisingMessage {
	union {
		struct{
			char messageType;
			char direction;
			uint16_t beat_period_millis;
			uint8_t beat_distinctivness;
		};
		char buffer[5];
	};
	size_t maxSize = 5;
	inline synchronisingMessage(){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = '0';
		this->buffer[0] = SyncMessage;
	}
	inline synchronisingMessage(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
			this->buffer[0] = SyncMessage;
		}
	}
	inline synchronisingMessage& operator= (const synchronisingMessage& mesg){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = mesg.buffer[i];
		return *this;
	}
	inline void create(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
	}
};

struct pushingMessage {
	union {
		struct{
			char messageType;
			char pushing1;
			char pushing2;
			char pushing3;
			char pushing4;
			char pushing5;
			char pushing6;
		};
		char buffer[7];
	};
	size_t maxSize = 7;
	inline pushingMessage(){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = '0';
		this->buffer[0] = PushMessage;
	}
	inline pushingMessage(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
			this->buffer[0] = PushMessage;
		}
	}
	inline pushingMessage& operator= (const pushingMessage& mesg){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = mesg.buffer[i];
		return *this;
	}
	inline void create(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
	}
};

struct statusingMessage {
	union {
		struct{
			char messageType;
			char status;
		};
		char buffer[2];
	};
	size_t maxSize = 2;
	inline statusingMessage(){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = '0';
		this->buffer[0] = SyncMessage;
	}
	inline statusingMessage(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
			this->buffer[0] = SyncMessage;
		}
	}
	inline statusingMessage& operator= (const statusingMessage& mesg){
		for(unsigned char i = 0; i < maxSize; i++)
			this->buffer[i] = mesg.buffer[i];
		return *this;
	}
	inline void create(char* buffer,  size_t length){
		if(length == maxSize){
			for(unsigned char i = 0; i < maxSize; i++)
				this->buffer[i] = buffer[i];
		}
	}
};



#endif /* PROTOCOL_H_ */
