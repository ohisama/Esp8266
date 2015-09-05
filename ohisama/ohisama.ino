#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);
char buff[256];
String sendAT(String command, const int timeout)
{
	String response = "";
	Serial.print(command);
	long int time = millis();
	while ((time + timeout) > millis())
	{
		while (Serial.available())
		{
			char c = Serial.read();
			response += c;
		}
	}
	mySerial.print(response);
	return response;
}
void setup()
{
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	mySerial.begin(115200);
	Serial.begin(115200);
	sendAT("AT+RST\r\n", 2000);
	sendAT("AT+CWMODE=1\r\n", 1000);
	sendAT("AT+CWJAP=\"WARPSTAR-CB39F1\",\"AA7FE89BFEDDC\"\r\n", 3000);
	sendAT("AT+CIPMUX=1\r\n", 1000);
	sendAT("AT+CIPSERVER=1,80\r\n", 1000);
	sendAT("AT+CIPSTO=15\r\n", 1000);
	sendAT("AT+CIFSR\r\n", 1000);
}
void loop()
{
	int ch_id, packet_len;
	char * pb;
	int i = 0;
	if (Serial.available())
	{
		//delay(600);
		mySerial.print("ok0\r\n");
		while (Serial.available() > 0)
		{
			buff[i++] = Serial.read();
			if (i > 1 && buff[i - 2] == 13 && buff[i - 1] == 10)
			{
				buff[i] = 0;
				i = 0;
				break;
			}
		}
		mySerial.print(buff);
		mySerial.print("ok2\r\n");
		if (strncmp(buff, "+IPD,", 5) == 0)
		{
			int connectionId = buff[5] - 48;
			// request: +IPD,ch,len:data
			sscanf(buff + 5, "%d,%d", &ch_id, &packet_len);
			String html;
			html = "HTTP/1.1 404 Not Found\r\n\r\n";
			if (packet_len > 0) 
			{
				pb = buff + 5;
				while (* pb != ':') pb++;
				pb++;
				if (strncmp(pb, "GET /favicon", 12) == 0) 
				{
					html = "HTTP/1.1 404 Not Found\r\n\r\n";
					mySerial.print("/favicon->");
				}
				else if (strncmp(pb, "GET /oi", 7) == 0) 
				{
					html = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nConnection:close\r\n\r\n{\"ohisama\":\"hi\"}\r\n";
					mySerial.print("/oi ->");
				}
				else if (strncmp(pb, "GET /ledoff", 11) == 0) 
				{
					html = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nConnection:close\r\n\r\n{\"ohisama\":\"led off\"}\r\n";
					digitalWrite(13, LOW);
					mySerial.print("/ledoff ->");
				}
				else if (strncmp(pb, "GET /ledon", 10) == 0) 
				{
					html = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nConnection:close\r\n\r\n{\"ohisama\":\"led on\"}\r\n";
					digitalWrite(13, HIGH);
					mySerial.print("/ledon ->");
				}
				else
				{
					html = "HTTP/1.1 404 Not Found\r\n\r\n";
					mySerial.print("/ ->");
				}
			}
			String cipSend = "AT+CIPSEND=";
			cipSend += connectionId;
			cipSend += ",";
			cipSend += html.length();
			cipSend += "\r\n";
			sendAT(cipSend, 1000);
			sendAT(html, 2000);
			String closeCommand = "AT+CIPCLOSE=";
			closeCommand += connectionId;
			closeCommand += "\r\n";
			sendAT(closeCommand, 1000);
		}
	}
}
