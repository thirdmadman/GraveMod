#include <Arduino.h>
#include <U8g2lib.h>
unsigned long
out,
lastMillis,
prevMillis,
firestart,
lastscrupd,
lasttnotify = 0, //Just time when was last notify
lastactive = 0 /// Time when was last activity
;
bool btnpressed, fire = false;
///log varribles witchs need for switch modes///
bool editmode = false,
firemode,
showfire = false,
editPWR = false,
edittime = false,
editfullpwr = false,
acceptfullpwr = false,
editft = false,
battrlownotify = false,
ftaggressive = false,
ftnormal = true,
srcon = false
;

int clicks = 0,
power = 70, // Default POWER
editmodepos = 1,
editftpos = 1, // Positions in menu of edition fire type
timeBeforeFrozeUpScreen = 3000, //time before srceen stop upd but still indicates
timeBeforeSleep = 30000; //time before srceen go sleep

int MOSFETpin = 9, // Pin wich driving mosfets (with PWM avalible)
measureMOSFETPin = 8; // Pin wich driving mosfet of voltage divider
long hightime = 200, lowtime = 100;
#define bttn 3 /// Pin for buttn "fire"
#define battpin A0 /// Pin for measure voltage of battery
#define respin A1 /// Pin for measure voltage on middle point of voltage divider
#define Vcc 4.983 /// high accuracy voltage of Vcc
float  Resprec = 2.70; ///  high accuracy resistance of  R1 of voltage divider
float
Vbatt,
coilRes,
coilReslast = 100,
maxfiretime = 3000, /// Time before cutdown power
resxread
;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);
void setup()
{
	// Timer0 уже используется millis() - прерываемся где-то посередине и вызываем ниже функцию "Compare A"
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
	//Serial.begin(9600);
	pinMode(MOSFETpin, OUTPUT);
	pinMode(13, OUTPUT);
	pinMode(measureMOSFETPin, OUTPUT);
  u8g2.begin();
}


// Прерывание вызывается один раз в миллисекунду, ищет любые новые данные, и сохраняет их
SIGNAL(TIMER0_COMPA_vect)
{
	unsigned long currentMillis = millis();
	out = millis() - lastMillis;
	//if (out <= 0) {out = currentMillis;}

	if (digitalRead(bttn) == HIGH && btnpressed == false)
	{
		prevMillis = lastMillis;
		lastMillis = currentMillis;
		btnpressed = true;
	}
	else if (digitalRead(bttn) == LOW && btnpressed == true)
	{
		//Serial.println(editmode);
		firemode = false;
		fire = false;
		//Serial.println(currentMillis - prevMillis );
		if ((currentMillis - prevMillis < 1000) && editmode == false)
		{
			if (out < 200 && out > 50)
			{
				if (clicks < 5)
				{
					++clicks;
					//Serial.print("clicks: ");
					//Serial.println(clicks);

					if (clicks == 4)
					{
						editmode = true;
						//Serial.println("edmode true");
					}
				}
				//Serial.println(prevMillis);
				//Serial.println(currentMillis);
				//Serial.println(lastMillis);
				//Serial.println(out);
			}
		}
		else if (editmode == true)
		{
			//Serial.println(out);
			if (editPWR == false && edittime == false && editfullpwr == false && editft == false)
			{
				if (out < 250 && out > 50)
				{
					if (editmodepos < 4)
					{
						editmodepos++;
					}
					else if (editmodepos + 1 > 4)
					{
						editmodepos = 1;
					}
					//Serial.println(editmodepos); /// DEV info
				}
				else if (out > 250 && out < 5000)
				{
					//Serial.println("IF");
					if (editmodepos == 1)
					{
						editPWR = true;
						edittime = false;
						editfullpwr = false;
						editft = false;
						//Serial.println("done 1");
					}
					else if (editmodepos == 2)
					{
						edittime = true;
						editPWR = false;
						editfullpwr = false;
						editft = false;
						//Serial.println("done 2");
						//Serial.println(out);
					}
					else if (editmodepos == 3)
					{
						editfullpwr = true;
						editPWR = false;
						edittime = false;
						editft = false;
						//Serial.println("done 3");
					}
					else if (editmodepos == 4)
					{
						editft = true;
						editfullpwr = false;
						editPWR = false;
						edittime = false;
						//Serial.println("done 3");
					}
				}
			}
			else if (editPWR == true)
			{
				//editPWR STRT
				if (power < 100)
				{
					power += 5;
				}
				else if (power + 5 >= 105)
				{
					power = 5;
				}
				//editPWR END
			}
			else if (edittime == true)
			{
				//edittime STRT
				if (maxfiretime < 10000)
				{
					maxfiretime += 500;
				}
				else if (maxfiretime + 500 >= 10500)
				{
					maxfiretime = 500;
				}
				//edittime END
			}
			else if (editfullpwr == true)
			{
				//Serial.println("editfullpwr");
				//Serial.println(acceptfullpwr);
				if (acceptfullpwr == false)
				{
					acceptfullpwr = true;
				}
				else if (acceptfullpwr == true)
				{
					acceptfullpwr = false;
				}
			}
			else if (editft == true)
			{
				if (editftpos < 2)
				{
					editftpos++;
				}
				else if (editftpos + 1 > 2)
				{
					editftpos = 1;
				}
				if (editftpos == 1)
				{
					ftnormal = true;
					ftaggressive = false;
				}
				else if (editftpos == 2)
				{
					ftnormal = false;
					ftaggressive = true;
				}
			}
		}
		else if (currentMillis - prevMillis > 1000 && editmode == false)
		{
			clicks = 0;
			pinMode(10, OUTPUT);
			digitalWrite(10, LOW);
		}
		btnpressed = false;
		lastactive = currentMillis;
	}
	else if (digitalRead(bttn) == HIGH && btnpressed == true && editmode == false)
	{
		if (out > 200 && firemode == false)
		{
			firemode = true;
			firestart = currentMillis;
		}
		else if (out > 200 && firemode == true)
		{
			fire = true;
		}
	}
	else if (digitalRead(bttn) == HIGH && btnpressed == true && editmode == true)
	{
		if (out >= 1500 && (editPWR == true || edittime == true || editfullpwr == true || editft == true))
		{
			//Serial.println("EXIT");
			editmode = false;
			edittime = false;
			editPWR = false;
			editfullpwr = false;
			editft = false;
			if (acceptfullpwr == true) {
				power = 100;
				acceptfullpwr = false;
			}
			editmodepos = 1;
			editftpos = 1;
			//Serial.println("edmode false");
		}
	}

}


void drawBatt(double voltsBatt) {
	double voltsOnBatt = voltsBatt;
	int voltsx;
	int boxPoseX = 110;
	u8g2.drawBox(boxPoseX + 3, 28, 6, 2);
	u8g2.drawFrame(boxPoseX, 3, 12, 25);
	if (voltsOnBatt >= 3.5 && voltsOnBatt <= 4.3)
	{
		voltsx = floor((voltsOnBatt - 3.5) / ((4.2 - 3.5) / 23.0));
		if (voltsx > 23) {
			voltsx = 23;
		}
		u8g2.drawBox(boxPoseX + 1, 4, 10, voltsx);
		u8g2.setCursor(70, 20);
		u8g2.print(100 - ((4.2 - voltsOnBatt) / 0.007));
	}
	else if (voltsOnBatt < 3.5)
	{
		u8g2.setFont(u8g_font_9x18B);
		u8g2.setCursor(boxPoseX + 2, 20);
		u8g2.print("!");
		if (battrlownotify == true)
		{
			u8g2.setFont(u8g_font_6x10);
			u8g2.setCursor(78, 14);
			u8g2.print("LOW");
			u8g2.setCursor(70, 28);
			u8g2.print("BATTR");
			if (millis() - lasttnotify >= 1000)
			{
				battrlownotify = false;
				lasttnotify = millis();
			}
		}
		else
		{
			if (millis() - lasttnotify >= 1000)
			{
				battrlownotify = true;
				lasttnotify = millis();
			}

		}
	}
}

void loop()
{
	digitalWrite(2, LOW);
	String firtsline, voltsstr;
	float volts;
	if ((fire == true) && ((millis() - firestart) <= maxfiretime))
	{
		if (power < 100)
		{
			if (ftaggressive == true)
			{
				if (int(-0.3 * (millis() - firestart) + 255) >= int(2.55 * power))
				{
					analogWrite(MOSFETpin, int(-0.3 * (millis() - firestart) + 255));
				}
				else
				{
					analogWrite(MOSFETpin, int(2.55 * power));
				}
			}
			else if (ftnormal == true)
			{
				analogWrite(MOSFETpin, int(2.55 * power));
			}
		}
		else
		{
			digitalWrite(MOSFETpin, HIGH);
		}
		unsigned long out2 = millis();
		//----------------------------exp strt
		int sizeoft = 50;
		float Vbatt3[sizeoft - 1];
		float temp; // временная переменная для обмена элементов местами
		for (int i = 0; i < sizeoft - 1; i++) {
			Vbatt3[i] = analogRead(battpin);
		}
		for (int i = 0; i < sizeoft - 1; i++) {
			for (int j = 0; j < sizeoft - i - 1; j++) {
				if (Vbatt3[j] > Vbatt3[j + 1]) {
					// меняем элементы местами
					temp = Vbatt3[j];
					Vbatt3[j] = Vbatt3[j + 1];
					Vbatt3[j + 1] = temp;
				}
			}
		}
		//Vbatt = Vbatt3[48];
		Vbatt = Vbatt3[23] + Vbatt3[24] + Vbatt3[25] + Vbatt3[26];
		Vbatt /= 4;

		//-----------------exp end
		volts = Vbatt * (Vcc / 1023.0);
		out2 = millis() - lastscrupd;
		if (showfire == false)
		{
			digitalWrite(measureMOSFETPin, LOW);
			lastscrupd = millis();
			showfire = true;
		}

		if (showfire == true && out2 >= 100)
		{
			u8g2.firstPage();
			do {
				u8g2.setFont(u8g_font_6x10);
				u8g2.drawFrame(0, 0, 128, 32);
				u8g2.setCursor(4, 14);
				u8g2.print("Vape!");
				u8g2.setCursor(56, 14);
				u8g2.print((millis() - firestart) / 1000.00);
				u8g2.setCursor(4, 28);
				u8g2.print("Vbatt: ");
				u8g2.setCursor(56, 28);
				u8g2.print(volts);
				u8g2.setFont(u8g_font_9x18B);
				u8g2.setCursor(92, 20);
				u8g2.print(int((volts * (((volts / coilRes))*power))));
				//u8g2.setCursor(90, 28);//dev ifo
				//u8g2.print(int(2.55 * power));//dev ifo
			} while (u8g2.nextPage());
			lastscrupd = millis();
			digitalWrite(13, HIGH);

		}
	}
	else if ((fire == true) && ((millis() - firestart) > maxfiretime))
	{
		digitalWrite(measureMOSFETPin, LOW);
		digitalWrite(MOSFETpin, LOW);
		digitalWrite(13, LOW);
		u8g2.firstPage();
		do {
			u8g2.setFont(u8g2_font_ncenB10_tr);
			u8g2.drawFrame(0, 0, 128, 32);
			u8g2.setCursor(18, 20);
			u8g2.print("OVERTIMED");
		} while (u8g2.nextPage());
		lastscrupd = millis();
	}
	else if ((fire == false))
	{
		analogWrite(MOSFETpin, 0);
		digitalWrite(MOSFETpin, LOW); // TODO: fix this tow
		digitalWrite(measureMOSFETPin, LOW);
		digitalWrite(13, LOW);
		if (millis() - lastactive < timeBeforeSleep)
		{
			if (srcon == false)
			{
				u8g2.sleepOff();
				srcon = true;
			}
			showfire = false;
			if (millis() - lastscrupd >= timeBeforeFrozeUpScreen)
			{
				if (millis() - lastactive <= 5000) // IMPORTANT find out what is "5000" and why!?
				{
					digitalWrite(measureMOSFETPin, HIGH);
					delay(10);
					float resxread2[50];
					float Vbatt2[50];
					float temp; // временная переменная для обмена элементов местами
					int sizeoft = 49;
					for (int i = 0; i < sizeoft - 1; i++) {
						resxread2[i] = analogRead(respin);
					}
					digitalWrite(measureMOSFETPin, LOW);
					delay(10);
					for (int i = 0; i < sizeoft - 1; i++) {
						Vbatt2[i] = analogRead(battpin);
					}
					for (int i = 0; i < sizeoft - 1; i++) {
						for (int j = 0; j < sizeoft - i - 1; j++) {
							if (resxread2[j] > resxread2[j + 1]) {
								// меняем элементы местами
								temp = resxread2[j];
								resxread2[j] = resxread2[j + 1];
								resxread2[j + 1] = temp;
							}
						}
					}
					for (int i = 0; i < sizeoft - 1; i++) {
						for (int j = 0; j < sizeoft - i - 1; j++) {
							if (Vbatt2[j] > Vbatt2[j + 1]) {
								// меняем элементы местами
								temp = Vbatt2[j];
								Vbatt2[j] = Vbatt2[j + 1];
								Vbatt2[j + 1] = temp;
							}
						}
					}
					resxread = 0;
					for (int i = 23; i < 27; i++) {
						resxread += resxread2[i];
					}
					resxread /= 4;
					Vbatt = 0;
					for (int i = 23; i < 27; i++) {
						Vbatt += Vbatt2[i];
					}
					Vbatt /= 4;

					//Resprec = (Vcc / 1024.00) * resxread;
					//Serial.println(resxread);
				}

				volts = Vbatt * (Vcc / 1023.000);
				u8g2.firstPage();
				// rebuild the picture after some delay
				//delay(80);
				do {
					u8g2.setFont(u8g_font_6x10);
					u8g2.drawFrame(0, 0, 128, 32);
					if (editmode == true)
					{
						if (editPWR == false && edittime == false && editfullpwr == false && editft == false)
						{
							if (editmodepos <= 2)
							{
								u8g2.setCursor(20, 14);
								u8g2.print("1. Edit PWR");
								u8g2.setCursor(20, 28);
								u8g2.print("2. Edit time");
								if (editmodepos == 1)
								{
									u8g2.drawBox(8, 7, 7, 7);
								}
								else if (editmodepos == 2)
								{
									u8g2.drawBox(8, 21, 7, 7);
								}
							}
							else if (editmodepos <= 4 && editmodepos >= 3)
							{
								u8g2.setCursor(20, 14);
								u8g2.print("3. Full pwr");
								u8g2.setCursor(20, 28);
								u8g2.print("4. Fire type");
								if (editmodepos == 3)
								{
									u8g2.drawBox(8, 7, 7, 7);
								}
								else if (editmodepos == 4)
								{
									u8g2.drawBox(8, 21, 7, 7);
								}
							}
						}
						else if (editPWR == true)
						{
							u8g2.setCursor(5, 20);
							u8g2.print("PWR %: ");
							u8g2.setCursor(80, 20);
							u8g2.print(power);
						}
						else if (edittime == true)
						{
							u8g2.setCursor(10, 11);
							u8g2.print("time mSec: ");
							u8g2.setCursor(75, 11);
							u8g2.print(maxfiretime);
							u8g2.drawFrame(13, 16, 102, 12);
							u8g2.drawBox(14, 17, int(maxfiretime / 100), 10);
						}
						else if (editfullpwr == true)
						{
							u8g2.setCursor(40, 20);
							u8g2.print("YES");
							u8g2.setCursor(80, 20);
							u8g2.print("NO");

							if (acceptfullpwr == true)
							{
								u8g2.drawBox(30, 13, 6, 6);
							}
							else if (acceptfullpwr == false)
							{
								u8g2.drawBox(70, 13, 6, 6);
							}
						}
						else if (editft == true)
						{
							u8g2.drawFrame(4, 4, 24, 24);
							u8g2.drawLine(4, 16, 27, 16);
							u8g2.drawFrame(32, 4, 24, 24);
							u8g2.drawLine(32, 4, 44, 16);
							u8g2.drawLine(44, 16, 55, 16);
							if (editftpos == 1)
							{
								u8g2.drawFrame(3, 3, 26, 26);
							}
							else if (editftpos == 2)
							{
								u8g2.drawFrame(31, 3, 26, 26);
							}
						}
					}
					if (editmode == false)
					{
						drawBatt(volts); //new func for drawing battr and notify
						u8g2.setFont(u8g_font_9x18B);
						u8g2.setCursor(5, 20);
						u8g2.print(power);
						u8g2.setFont(u8g_font_6x10);
						u8g2.setCursor(40, 28);
						u8g2.print(volts);
						u8g2.setCursor(40, 14);
						//coilReslast = coilRes;
						coilRes = (Vbatt / resxread - 1.0000) * Resprec * 100;
						u8g2.print(coilRes);
					}
				} while (u8g2.nextPage());
			}
		}
		else if (millis() - lastactive >= timeBeforeSleep)
		{
			u8g2.sleepOn();
			srcon = false;
		}
	}
	else {
		u8g2.firstPage();
		do {
			u8g2.setCursor(5, 10);
			u8g2.print("WE GOT ELSE");
		} while (u8g2.nextPage());
	}
}
