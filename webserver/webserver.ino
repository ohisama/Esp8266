#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);
bool state = true;
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
	sendAT("AT+CWMODE=2\r\n", 1000);
	sendAT("AT+CIFSR\r\n", 1000);
	sendAT("AT+CIPMUX=1\r\n", 1000);
	sendAT("AT+CIPSERVER=1,80\r\n", 1000);
}
void loop()
{
	if (Serial.available())
	{
		if (Serial.find("+IPD,"))
		{
			delay(100);
			int connectionId = Serial.read() - 48;
			String html;
			if (Serial.find("favicon"))
			{
				html = "HTTP/1.1 404 Not Found\r\n\r\n";
				mySerial.print("/favicon->");
			}
			else
			{
				mySerial.print("/ ->");
				if (state)
				{
					html = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:close\r\n\r\n<body><h1>LED OFF</h1></body>";
					state = false;
					digitalWrite(13, LOW);
				}
				else
				{
					html = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:close\r\n\r\n<body><h1>LED ON</h1></body>";
					state = true;
					digitalWrite(13, HIGH);
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
