// 4-channel RC receiver for controlling
// an RC car / boat / plane / quadcopter / etc.
// using an ESP8266 and an Android phone with RoboRemo app

// Disclaimer: Don't use RoboRemo for life support systems
// or any other situations where system failure may affect
// user or environmental safety.

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Servo.h>

#define RESXu 1024u

#define SSID "car 2.0"
#define PSK ""
#define PORT 9876

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

enum {
	UNKNOWN = -1,

	SERVO,
	SERVO_EXP,
	SPEED,

};

typedef struct {
	char *roboremo_id;
	int16_t min;
	int16_t max;
	int16_t val;
} ctrl_t;

ctrl_t controls[] = {
	[SERVO] =		{"servo", -100, 100, 0},
	[SERVO_EXP] =	{"servo_exp", 0, 10, 0},
	[SPEED] =		{"speed", -100, 100, 0},
};

#define servo_exp (controls[SERVO_EXP].val)

Servo servo;

WiFiClient client;
WiFiUDP udp;



unsigned long lastCmdTime = 60000;
unsigned long aliveSentTime = 0;



int16_t calc100to256(int8_t x) // return x*2.56
{
  return ((int16_t)x << 1) + (x >> 1) + (x >> 4);
}

int16_t calc100toRESX(int8_t x) // return x*10.24
{
	int16_t res = ((int16_t)x * 41) >> 2;
	int8_t sign = x < 0 ? 1 : 0;
	x -= sign;
	res -= x >> 6;
	res -= sign;

	return res;
}

int16_t calcRESXtoServo(int16_t x)  // return x/1.024
{
// *1000/1024 = x - x/32 + x/128
	return ((x - (x >> 5) + (x >> 7) + 1000) >> 1) + 1000;
}

// input parameters;
//  x 0 to 1024;
//  k 0 to 100;
// output between 0 and 1024
unsigned int expou(unsigned int x, unsigned int k)
{
	k = calc100to256(k);

	uint32_t value = (uint32_t) x * x;
	value *= (uint32_t)k;
	value >>= 8;
	value *= (uint32_t)x;

	value >>= 12;
	value += (uint32_t)(256 - k) * x + 128;

	return value >> 8;
}

int expo(int x, int k)
{
	if (k == 0)
		return x;

	int y;
	bool neg = (x < 0);

	if (neg)
		x = -x;

	if (x > (int)RESXu)
		x = RESXu;

	if (k < 0)
		y = RESXu - expou(RESXu - x, -k);
	else
		y = expou(x, k);

	return neg ? -y : y;
}

int16_t in2out(int16_t x)
{
	return calcRESXtoServo(expo(calc100toRESX(x), servo_exp * 10));
}

void init_gpios(void)
{
}

void setup()
{
	delay(1000);

	Serial.begin(115200);

	IPAddress ip(192, 168, 0, 1); // From RoboRemo app, connect to this IP
	IPAddress netmask(255, 255, 255, 0);

	WiFi.softAPConfig(ip, ip, netmask); // configure ip address for softAP 
	WiFi.softAP(SSID, PSK); // configure ssid and password for softAP

	udp.begin(PORT); // start UDP server

	Serial.println("ESP8266 RC receiver 1.1 powered by RoboRemo");
	Serial.println((String)"SSID: " + SSID + "  PASS: " + PSK);
	Serial.println((String)"RoboRemo app must connect to " + ip.toString() + ":" + PORT);
}

ctrl_t *get_ctrl(int id)
{
	if (id >= 0 && id < ARRAY_SIZE(controls))
		return &controls[id];

	return NULL;
}

//return ctrl id
int parse_pkt(char *pkt_buf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(controls); i++) {
		if (!strncmp(pkt_buf, controls[i].roboremo_id, strlen(controls[i].roboremo_id))
				&& pkt_buf[strlen(controls[i].roboremo_id)] == ' ') {

			int val = atoi(pkt_buf + strlen(controls[i].roboremo_id));
			if (val < controls[i].min || val > controls[i].max)
				return UNKNOWN;

			if (controls[i].val == val)
				return UNKNOWN;

			controls[i].val = val;

			return i;
		}
	}

	return UNKNOWN;
}

void loop()
{
	int pkt_size = udp.parsePacket();
	if (pkt_size) {
		//Serial.printf("Received %d bytes from %s, port %d\n", pkt_size, udp.remoteIP().toString().c_str(), udp.remotePort());
		char pkt_buf[64];
		int len = udp.read(pkt_buf, sizeof(pkt_buf));
		if (len > 0) {
			pkt_buf[len] = 0;

			int id = parse_pkt(pkt_buf);
			ctrl_t *ctrl = get_ctrl(id);
			if (ctrl)
				Serial.printf("ctrl: \"%s\" val: %d\n", ctrl->roboremo_id, ctrl->val);

			switch (id) {
				case SERVO:
					if (!servo.attached())
						servo.attach(4, 1000, 2000);
					servo.writeMicroseconds(in2out(ctrl->val));
				break;
				case SERVO_EXP:
				break;
				case SPEED:
				break;
			}
		}

		//Serial.printf("UDP packet contents: %s\n", pkt_buf);

	}

#if 0
  // if contact lost for more than half second
  if(millis() - lastCmdTime > 500) {  
    for(int i=0; i<chCount; i++) {
      // set all values to middle
      servoCh[i].writeMicroseconds(1500);
      servoCh[i].detach(); // stop PWM signals
    }
  }

  
  if(!client.connected()) {
    client = server.available();
    return;
  }

  // here we have a connected client

  if(client.available()) {
    char c = (char)client.read(); // read char from client (RoboRemo app)

    if(c=='\n') { // if it is command ending
      cmd[cmdIndex] = 0;
      exeCmd();  // execute the command
      cmdIndex = 0; // reset the cmdIndex
    } else {      
      cmd[cmdIndex] = c; // add to the cmd buffer
      if(cmdIndex<99) cmdIndex++;
    }
  } 

  if(millis() - aliveSentTime > 500) { // every 500ms
    client.write("alive 1\n");
    // send the alibe signal, so the "connected" LED in RoboRemo will stay ON
    // (the LED must have the id set to "alive")
    
    aliveSentTime = millis();
    // if the connection is lost, the RoboRemo will not receive the alive signal anymore,
    // and the LED will turn off (because it has the "on timeout" set to 700 (ms) )
  }
#endif
}
