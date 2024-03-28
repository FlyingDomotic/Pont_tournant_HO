/*
 *	   Gestion d'un pont tournant férroviaire miniature
 *
 *	Matériel
 *		ESP8266
 *		Moteur pas à pas avec driver (enable/pulse/direction) (par exemple NEMA 24 avec DM556T)
 *		Encodeur absolu RS485 (par exemple QY3806-485RTU)
 *		Adaptateur série/RS485
 *		Lecteur MP3 avec chip XY-V17B (optionnel)
 *		LED de rotation (optionnel)
 *
 *	Connexions
 *		Par défaut, les pinoches suivantes sont définies dans le fichier constants.h :
 *			D0 (enablePin) : Signal enable du moteur
 *			D1 (pulsePin) : Signal pulse du moteur
 * 			D2 (directionPin) : Signal direction du moteur
 *			D5 (rxPin) : RX du module RS485
 *			D6 (txPin) : TX du module RS485
 *			D7 (mp3Pin) : Ligne de communication (one line) du module MP3
 *			D8 (runPin) : Haut pendant la rotation (commande LED et son de rotation)
 *
 *	V1.0.0 FF - Mars 2024 - Pour le FabLab
 *
 *	GNU GENERAL PUBLIC LICENSE - Version 3, 29 June 2007
 *
 */

#include <ESP8266WiFi.h>											// Wifi
#include <ESPAsyncWebServer.h>										// Serveur Web asynchrone
#include <ArduinoOTA.h>												// OTA (mise à jour par le réseau)
#include <arduino.h>												// Arduino
#include <preferences.h>											// Préférences liées au pont
#include <stepperCommand.h>											// Moteur
#include <rotationSensor.h>											// Encodeur
#include <mp3_player_module_wire.h>									// MP3 player
#include <cmath>													// Fonctions mathématiques
#include <LittleFS.h>												// Gestion du système de fichier
#include <littleFsEditor.h>											// Editeur de système de fichier LittleFS
#include <ArduinoJson.h>											// Documents JSON

//	---- Variables ----

//	Classe serveur Web asynchrone
AsyncWebServer webServer(80);										// Web serveur sur le port 80
AsyncEventSource events("/events");									// Définir la racine des évenements 

//	Nom du point d'accès Wifi et du module
String ssid;
String pwd;
String espName = "PontTournant";

//	Nom du fichier de configuration
#define SETTINGS_FILE "settings.json"

//	Moteur
StepperCommand
	stepper(pulsePin, directionPin, enablePin, traceDebug);			// Classe moteur
float lastAngleSent = 0;											// Dernier angle envoyé aux utilisateurs
float currentAngle = 0;												// Angle courant
unsigned long microStepsToGo = 0;									// Nombre de micro steps restants à envoyer
enum stepperState {													// Etats de la rotation
	stepperStopped,
	stepperStarting,
	stepperRunning,
	stepperStopping
};
stepperState rotationState = stepperStopped;						// Etat courant de la rotation

//	Encodeur
RotationSensor encoder(rxPin, txPin, traceDebug);				// Classe encodeur
uint16_t resultValue;												// Dernier résultat encodeur
unsigned long resultValueTime = 0;									// Heure du dernier résultat de l'encodeur

// Lecteur MP3
Mp3PlayerModuleWire player(mp3Pin);									// Classe lecteur MP3
unsigned long playerStartedTime = 0;								// Heure de la dernière lecture
unsigned long playerDuration = 0;									// Durée de lecture du son

//	---- Routines liées aux préférences ----

void dumpSettings(void) {
	Serial.printf("ssid = %s\n", ssid.c_str());
	Serial.printf("pwd = %s\n", pwd.c_str());
	Serial.printf("name = %s\n", espName.c_str());
	Serial.printf("degreesPerStep = %.1f\n", degreesPerStep);
	Serial.printf("microStepsPerStep = %.1f\n", microStepsPerStep);
	Serial.printf("driverMinimalMicroSec = %d\n", driverMinimalMicroSec);
	Serial.printf("requiredRPM = %.1f\n", requiredRPM);
	Serial.printf("maxDelta = %d\n", maxDelta);
	Serial.printf("encoderOffsetAngle = %.1f\n", encoderOffsetAngle);
	Serial.printf("imageOffsetAngle = %.1f\n", imageOffsetAngle);
	Serial.printf("slaveId = %d\n", slaveId);
	Serial.printf("regId = %d\n", regId);
	Serial.printf("radius = %f\n", radius);
	Serial.printf("enableSound = %s\n", enableSound ? "true" : "false");
	Serial.printf("soundVolume = %d\n", soundVolume);
	Serial.printf("beforeSoundIndex = %d\n", beforeSoundIndex);
	Serial.printf("beforeSoundDuration = %.1f\n", beforeSoundDuration);
	Serial.printf("moveSoundIndex = %d\n", moveSoundIndex);
	Serial.printf("afterSoundIndex = %d\n", afterSoundIndex);
	Serial.printf("afterSoundDuration = %.1f\n", afterSoundDuration);
	Serial.printf("trackCount = %d\n", trackCount);
	Serial.printf("traceDebug = %s\n", traceDebug ? "true" : "false");
	Serial.printf("traceCode = %s\n", traceCode ? "true" : "false");

	char name[10];
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		Serial.printf("%s = %.1f\n", name, indexes[i]);
	}
}

bool readSettings(void) {
	File settingsFile = LittleFS.open(SETTINGS_FILE, "r");
	if (!settingsFile) {
		Serial.printf("Failed to open config file\n");
		return false;
	}

	JsonDocument settings;
	auto error = deserializeJson(settings, settingsFile);
	settingsFile.close();
	if (error) {
		Serial.printf("Failed to parse config file\n");
		return false;
	}

	degreesPerStep = settings["degreesPerStep"].as<float>();
	microStepsPerStep = settings["microStepsPerStep"].as<float>();
	driverMinimalMicroSec =settings["driverMinimalMicroSec"].as<uint16_t>();
	requiredRPM = settings["requiredRPM"].as<float>();
	maxDelta = settings["maxDelta"].as<uint8_t>();
	enableEncoder = settings["enableEncoder"].as<bool>();
	encoderOffsetAngle = settings["encoderOffsetAngle"].as<float>();
	imageOffsetAngle = settings["imageOffsetAngle"].as<float>();
	slaveId = settings["slaveId"].as<uint8_t>();
	regId = settings["regId"].as<uint8_t>();
	radius = settings["radius"].as<float>();
	trackCount = settings["trackCount"].as<uint8_t>();
	traceDebug = settings["traceDebug"].as<bool>();
	traceCode = settings["traceCode"].as<bool>();
	ssid = settings["ssid"].as<String>();
	pwd = settings["pwd"].as<String>();
	espName = settings["name"].as<String>();
	enableSound = settings["enableSound"].as<bool>();
	soundVolume = settings["soundVolume"].as<uint8_t>();
	beforeSoundIndex = settings["beforeSoundIndex"].as<uint8_t>();
	beforeSoundDuration = settings["beforeSoundDuration"].as<float>();
	moveSoundIndex = settings["moveSoundIndex"].as<uint8_t>();
	afterSoundIndex = settings["afterSoundIndex"].as<uint8_t>();
	afterSoundDuration = settings["afterSoundDuration"].as<float>();

	char name[10];
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		indexes[i] = settings[name].as<float>();
	}
	dumpSettings();
	stepper.setParams(degreesPerStep, microStepsPerStep, driverMinimalMicroSec, requiredRPM, traceDebug);
	encoder.setParams(encoderOffsetAngle, traceDebug);
	player.set_volume(soundVolume);
	return true;
}

void writeSettings(void) {
	JsonDocument settings;
	char name[10];

	settings["ssid"] = ssid.c_str();
	settings["pwd"] = pwd.c_str();
	settings["name"] = espName.c_str();
	settings["degreesPerStep"] = degreesPerStep;
	settings["microStepsPerStep"] = microStepsPerStep;
	settings["driverMinimalMicroSec"] = driverMinimalMicroSec;
	settings["requiredRPM"] = requiredRPM;
	settings["maxDelta"] = maxDelta;
	settings["enableEncoder"] = enableEncoder;
	settings["encoderOffsetAngle"] = encoderOffsetAngle;
	settings["imageOffsetAngle"] = imageOffsetAngle;
	settings["slaveId"] = slaveId;
	settings["regId"] = regId;
	settings["radius"] = radius;
	settings["enableSound"] = enableSound;
	settings["soundVolume"] = soundVolume;
	settings["beforeSoundIndex"] = beforeSoundIndex;
	settings["beforeSoundDuration"] = beforeSoundDuration;
	settings["moveSoundIndex"] = moveSoundIndex;
	settings["afterSoundIndex"] = afterSoundIndex;
	settings["afterSoundDuration"] = afterSoundDuration;
	settings["traceDebug"] = traceDebug;
	settings["traceCode"] = traceCode;
	settings["trackCount"] = trackCount;
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		settings[name] = indexes[i];
	}
	File settingsFile = LittleFS.open(SETTINGS_FILE, "w");
	if (!settingsFile) {
		Serial.printf("Can't open for write settings file\n");
	}
	uint16_t bytes = serializeJsonPretty(settings, settingsFile);
	if (!bytes) {
		Serial.printf("Can't write settings file\n");
	}
	settingsFile.flush();
	settingsFile.close();
	events.send("Ok", "settings");									// Envoie l'évènement "settings"
}

//	---- Routines liées à l'encodeur ----

// Activé a la fin d'une transaction Modbus
Modbus::ResultCode modbusCb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
	if (event != Modbus::EX_SUCCESS) {								// On a une erreur
		if (traceDebug) {
			Serial.printf("Read error: 0x%x\n", event);
		}
	} else {
		float angle = encoder.computeAngle(resultValue);				// Résupère l'angle depuis le dernier résultat
		if (abs(angle - lastAngleSent) >= 1.0) {						// N'envoyer le message que si la différence est significative
			if (traceDebug) {
				Serial.printf("Angle: %f\n", angle);
			}
			// Envoyer un event de type "angle" au navigateur
			char msg[10];												// Buffer pour le message
			snprintf_P(msg, sizeof(msg), PSTR("%.1f"), angle);			// Convertir l'angle en message
			events.send(msg, "angle");									// Envoie l'évènement "angle"
			resultValueTime = millis();									// Charger l'heure du dernier résultat
			lastAngleSent = angle;										// Sauver le dernier angle envoyé
		}
		currentAngle = angle;											// Angle courant
	}
	return Modbus::EX_SUCCESS;
}

//	---- Routines liées au moteur ----

// Activé au début d'une rotation
void startRotation(void) {
	digitalWrite(runPin, HIGH);										// Mettre la sortie rotation à l'état haut (on)
	if (rotationState == stepperStopped) {							// Est-ce qu'on est stoppé ?
		rotationState = stepperRunning;								// Mettre en running par défaut
		if (enableSound) {											// Est-ce que le son est activé ?
			if (beforeSoundIndex) {									// Est-ce qu'on a un son de démarrage ?
				rotationState = stepperStarting;					// Passer l'état en starting
				playerStartedTime = millis();						// Charger l'heure de lancement
				playerDuration = beforeSoundDuration * 1000.0;		// Convertir la durée en ms 
				playIndex(beforeSoundIndex);						// Jouer le son
			} else {
				if (moveSoundIndex) {								// Est-ce qu'il y a un son de rotation
					playIndex(moveSoundIndex);						// Jouer le son
				} else {
					playStop();
				}
			}
		}
	} else if (rotationState == stepperStopping) {					// Est-ce qu'on est en cours d'arrêt ?
		rotationState = stepperRunning;								// Repasser en running
		if (moveSoundIndex) {										// Est-ce qu'il y a un son de rotation
			playIndex(moveSoundIndex);								// Jouer le son
		} else {
			playStop();
		}
	}
}

// Activé à la fin d'une rotation
void stopRotation(void) {
	if (enableSound) {												// Est-ce que le son est activé ?
		if (afterSoundIndex) {										// Est-ce qu'on a un son d'arrêt ?
			rotationState = stepperStopping;						// Passer en arrêt
			playerDuration = afterSoundDuration * 1000.0;			// Convertir la durée en ms 
			playIndex(afterSoundIndex);								// Charger l'heure de lancement
			return;
		}
	}
	playStop();														// Arrêt du son
	rotationState = stepperStopped;									// Forcer l'état à stoppé
	digitalWrite(runPin, LOW);										// Mettre la sortie rotation à l'état bas (off)
}

// Déplace le pont sur une voie donnée
void moveToTrack(uint8_t index) {
	float requiredAngle = indexes[index];
	float deltaAngle = requiredAngle - currentAngle;
	if (deltaAngle > 180.0) {
		deltaAngle -= 360.0;
	}
	if (deltaAngle < -180.0) {
		deltaAngle += 360.0;
	}
	microStepsToGo = stepper.microStepsForAngle(deltaAngle);
	if (microStepsToGo) {
		startRotation();
	}
	if (traceCode) {
		Serial.printf("Current angle = %.1f, required angle = %.1f, delta angle = %.1f, micro step count = %ld, angle per micro step = %.1f\n", currentAngle, requiredAngle, deltaAngle, microStepsToGo, stepper.anglePerMicroStep());
	}
}

//	---- Routines liées au module MP3 ----

void playIndex(uint8_t index) {
	if (traceCode) {
		Serial.printf("Playing index %d\n", index);
	}
	playerStartedTime = millis();									// Load starting time
	player.set_track_index(index);									// Set file index
	player.play();													// Start playing file
}

void playStop() {
	if (traceCode) {
		Serial.printf("Stop playing\n");
	}
	player.stop();
}

//	---- Routines liées au serveur Web ----

// Routine activée lors d'une demande de setup
void setupReceived(AsyncWebServerRequest *request) {
	AsyncWebServerResponse *response = request->beginResponse(LittleFS, "setup.htm", "text/html");
	request->send(response);
}

// Routine activée lors de la demande de debug
void debugReceived(AsyncWebServerRequest *request) {
	JsonDocument settings;
	settings["lastAngleSent"] = lastAngleSent;
	settings["currentAngle"] = currentAngle;
	settings["microStepsToGo"] = microStepsToGo;
	settings["resultValue"] = resultValue;
	settings["resultValueTime"] = millis() - resultValueTime;
	settings["playerStartedTime"] = millis() - playerStartedTime;
	settings["playerDuration"] = playerDuration;
	settings["rotationState"] = rotationState;
	char buffer[1024];
	serializeJsonPretty(settings, buffer, sizeof(buffer));
	request->send(200, "application/json", String(buffer));
}


// Routine activée lors de la réception de la position d'un clic
void clickReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	int8_t closestIndex = -1;
	int separator1 = position.indexOf("/");							// Position du premier "/"
	if (separator1 >= 0) {
		int separator2 = position.indexOf("/", separator1+1); 		// Position du second "/"
		if (separator2 > 0) {
			float x = position.substring(separator1+1, separator2).toFloat();
			float y = position.substring(separator2+1).toFloat();
            float closestDistance = 9999.0;
			for (uint8_t i = 1; i <= trackCount; i++) {
				float xPos = 50.0 + (radius * cos(indexes[i] * PI / 180.0));
				float yPos = 50.0 - (radius * sin(indexes[i] * PI / 180.0));
				if (abs(x - xPos) < maxDelta and abs(y - yPos) < maxDelta) {
					float distance = sqrt(pow(abs(x - xPos), 2) + pow(abs(y - yPos), 2));
					if (distance < closestDistance) {
						closestDistance = distance;
						closestIndex = i;
					}
				}
			}
			if (traceCode) {
				Serial.printf("Index: %d\n", closestIndex);
			}
			if (closestIndex >=0) {
				moveToTrack(closestIndex);
			}
		}
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Routine activée lors de la Reçu d'un demande de movement
void moveReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");								// Position du premier "/"
	if (separator1 >= 0) {
		int16_t count = position.substring(separator1+1).toInt();
		stepper.setDirection(-count);
		microStepsToGo = abs(-count);
		startRotation();
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Routine activée lors de la réception de définition du calage de l'image
void imgRefReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	imageOffsetAngle = -currentAngle;
	writeSettings();
	request->send_P(200, "", "<status>Ok</status>");
}

// Routine activée lors de la réception d'un changement de voie
void gotoReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position du premier "/"
	if (separator1 >= 0) {
		int8_t track = position.substring(separator1+1).toInt();
		if (track > 0 && track <= trackCount)  {
			moveToTrack(track);
			request->send_P(200, "", "<status>Ok</status>");
			return;
		}
	}
	request->send_P(400, "", "<status>Bad track number</status>");
}

// Routine activée lors de la réception du chargement de l'angle d'une voie
void setAngleReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position du premier "/"
	if (separator1 >= 0) {
		int8_t track = position.substring(separator1+1).toInt();
		if (track == 1) {
			indexes[1] = 0.0;
			encoderOffsetAngle = -currentAngle;
		} else {
			indexes[track] = currentAngle;
		}
		writeSettings();
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Routine activée lors de la réception du changement d'une variable
void setChangedReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		Serial.printf("Reçu %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position du premier "/"
	if (separator1 >= 0) {
		int separator2 = position.indexOf("/", separator1+1); 		// Position du second "/"
		if (separator2 > 0) {
			String fieldName = position.substring(separator1+1, separator2);
			String fieldValue = position.substring(separator2+1);
			fieldValue.toLowerCase();
			if (fieldName == "trackCount") {
				trackCount = fieldValue.toInt();
			} else if (fieldName == "degreesPerStep") {
				degreesPerStep = fieldValue.toFloat();
			} else if (fieldName == "microStepsPerStep") {
				microStepsPerStep = fieldValue.toFloat();
			} else if (fieldName == "driverMinimalMicroSec") {
				driverMinimalMicroSec = fieldValue.toInt();
			} else if (fieldName == "requiredRPM") {
				requiredRPM = fieldValue.toFloat();
			} else if (fieldName == "slaveId") {
				slaveId = fieldValue.toInt();
			} else if (fieldName == "regId") {
				regId = fieldValue.toInt();
			} else if (fieldName == "radius") {
				radius = fieldValue.toFloat();
			} else if (fieldName == "maxDelta") {
				maxDelta = fieldValue.toInt();
			} else if (fieldName == "traceDebug") {
				traceDebug = (fieldValue == "true");
			} else if (fieldName == "traceCode") {
				traceCode = (fieldValue == "true");
			} else if (fieldName == "ssid") {
				ssid = fieldValue;
			} else if (fieldName == "pwd") {
				pwd = fieldValue;
			} else if (fieldName == "name") {
				espName = fieldValue;
			} else if (fieldName == "enableEncoder") {
				enableEncoder = (fieldValue == "true");
			} else if (fieldName == "enableSound") {
				enableSound = (fieldValue == "true");
			} else if (fieldName == "soundVolume") {
				soundVolume = fieldValue.toInt();
			} else if (fieldName == "beforeSoundIndex") {
				beforeSoundIndex = fieldValue.toInt();
			} else if (fieldName == "beforeSoundDuration") {
				beforeSoundDuration = fieldValue.toFloat();
			} else if (fieldName == "moveSoundIndex") {
				moveSoundIndex = fieldValue.toInt();
			} else if (fieldName == "afterSoundIndex") {
				afterSoundIndex = fieldValue.toInt();
			} else if (fieldName == "afterSoundDuration") {
				afterSoundDuration = fieldValue.toFloat();
			} else {
				Serial.printf("Can't set field %s\n", fieldName.c_str());
			}
			writeSettings();
		}
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Retourne une erreur 404
void send404Error(AsyncWebServerRequest *request) {
	char msg[50];
	snprintf_P(msg, sizeof(msg), PSTR("File %s not found"), request->url().c_str());
	request->send(404, "text/plain", msg);
	if (traceCode) {
		Serial.println(msg);
	}
}

void notFound(AsyncWebServerRequest *request) {
	send404Error(request);
}

//	---- Routines liées à la mise à jour par le réseau (OTA) ----

void onStartOTA() {
	if (ArduinoOTA.getCommand() == U_FLASH) {
		if (traceCode) {
			Serial.println("Début de mise à jour du logiciel");
		}
	} else { 														// Mise à jour du système de fichiers
		if (traceCode) {
			Serial.println("Début de mise à jour du système de fichiers");
		}
		LittleFS.end();
	}
}
		
void onEndOTA() {
	if (traceCode) {
		Serial.println("Fin de mise à jour");
	}
}
	
void onErrorOTA(ota_error_t erreur) {
	Serial.print("Erreur OTA (");
	Serial.print(erreur);
	Serial.print(") : Erreur ");
	if (erreur == OTA_AUTH_ERROR) {
		Serial.println("authentification");
	} else if (erreur == OTA_BEGIN_ERROR) {
		Serial.println("lancement");
	} else if (erreur == OTA_CONNECT_ERROR) {
		Serial.println("connexion");
	} else if (erreur == OTA_RECEIVE_ERROR) {
		Serial.println("réception");
	} else if (erreur == OTA_END_ERROR) {
		Serial.println("terminaison");
	} else {
		Serial.println("inconnue !");
	}
}

//	---- Initialisation au lancement du programme ----

void setup() {
	Serial.begin(74880);										// Initialise le port série à 74880 bds (~ 7.500 caractères par seconde)
	Serial.println("Le programme s'initialise ...");

	// Lance le service de fichier en flash
	LittleFS.begin();

	// Charge les préférences
	if (!readSettings()) {
		Serial.printf("No settings, stopping !\n");
		while (true) {
			yield();
		}
	};
	
	WiFi.hostname(espName.c_str());									// Définit le nom de ce module
	IPAddress adresseIP;											// Adresse IP du serveur ou du client

	if (ssid) {
		Serial.print("\nConnexion au Wifi ");
		Serial.print(ssid.c_str());
		Serial.print(" ");

		uint16_t loopCount = 0;										// Nombre de tentatives
		WiFi.begin(ssid.c_str(), pwd.c_str());						// Démarrer la connexion à un accès Wifi existant
		while (WiFi.status() != WL_CONNECTED && loopCount < 120) {	// Attendre la connexion ou 120 boucles (1mn)
			delay(500);												// Attendre 0,5 s
			Serial.print(".");										// Afficher un point
			loopCount++;
		}															// Et reboucler
		adresseIP = WiFi.localIP();									// Adresse IP du client
	}

	if (WiFi.status() != WL_CONNECTED) {						// On n'est pas connecté
		char buffer[32];
		snprintf(buffer, sizeof(buffer),"PontTournant_%X", ESP.getChipId());
		Serial.print("\nCréation du point d'accès Wifi ");
		Serial.println(buffer);
		WiFi.softAP(buffer, "");								// Démarrer un point d'accès Wifi
		adresseIP = WiFi.softAPIP();							// Adresse IP de ce serveur
	}

	Serial.println();
	Serial.print("URL de connexion: http://");
	Serial.print(adresseIP);									// Afficher l'adresse IP de ce serveur/client
	Serial.print("/ ou http://");
	Serial.print(espName);
	Serial.println("/");

	// Trace du process de mise à jour par le réseau
	ArduinoOTA.onStart(onStartOTA);
	ArduinoOTA.onEnd(onEndOTA);
	ArduinoOTA.onError(onErrorOTA);

	//ArduinoOTA.setPassword("mon mot de passe");					// Dé-commenter pour associer un mot de passe à l'OTA
	ArduinoOTA.begin();												// Initialise la mise à jour par le réseau

	events.onConnect([](AsyncEventSourceClient *client){			// Routine appellée à la connexion d'un client
		float angle = encoder.computeAngle(resultValue);
		// Envoie un event de type "angle" au client
		char msg[10];
		if (enableEncoder) {
			snprintf_P(msg, sizeof(msg), PSTR("%.1f"), angle);
		} else {
			snprintf_P(msg, sizeof(msg), PSTR("%.1f"), currentAngle);
		}
		client->send(msg, "angle");
		client->send("Ok", "data");
	});

    // Liste des URI à intercepter avant le traitement standard des requêtes
    webServer.on("/pos", HTTP_GET, clickReceived);					// Traitement de la requête /pos
    webServer.on("/move", HTTP_GET, moveReceived);					// Traitement de la requête /move
    webServer.on("/imgref", HTTP_GET, imgRefReceived);				// Traitement de la requête /imgref
    webServer.on("/goto", HTTP_GET, gotoReceived);					// Traitement de la requête /goto
    webServer.on("/changed", HTTP_GET, setChangedReceived);			// Traitement de la requête /setchanged
    webServer.on("/setup", HTTP_GET, setupReceived);				// Traitement de la requête /setup
    webServer.on("/debug", HTTP_GET, debugReceived);				// Traitement de la requête /debug
	webServer.addHandler(&events);									// Définir les évènements Web
	webServer.addHandler(new LittleFSEditor());						// Définir l'editeur du file système 
	webServer.onNotFound (notFound);								// Routine a appeler si l'URL donnée n'est pas dans la liste ci-dessus
	webServer.serveStatic("/",LittleFS, "/").setDefaultFile("index.htm"); // Servir "/", page par défaut = index.htm
	webServer.begin();												// Lancer le serveur Web

	stepper.begin();												// Initialiser

	digitalWrite(runPin, LOW);										// Mettre la sortie rotation à l'état bas (off)
	pinMode(runPin, OUTPUT);										// Passer la pinoche rotation en sortie

	encoder.setCallback(modbusCb);									// Callback en fin de requête Modbus
	encoder.begin();												// Initialiser l'encodeur

	player.set_volume(soundVolume);									// Définition du volume (max = 30)
	player.set_play_mode(1);										// Joue un fichier en boucle
}

// ---- Boucle permanente ----
void loop() {
	
	if (rotationState == stepperRunning) {							// Est-ce qu'on est en rotation ?
		stepper.turnOneMicroStep();									// Tourner d'un micro step
		currentAngle = fmodf(currentAngle + stepper.anglePerMicroStep(), 360.0); // Met à jour l'angle actuel, modulo 360
		microStepsToGo--;											// Enlever un micro step
		if (!microStepsToGo) {										// On a terminé la rotation
			stopRotation();											// Arrêt de la rotation
		}
	} else if (rotationState == stepperStarting) {					// Est-ce qu'on démarre ?
		if ((millis() - playerStartedTime) > playerDuration) {		// Est-ce qu'on a dépassé le temps de lecture ?
			if (moveSoundIndex) {									// Est-ce qu'il y a un son de rotation
				playIndex(moveSoundIndex);							// Jouer le son
			} else {
				playStop();											// Stopper le son de lancement
			}
			rotationState = stepperRunning;							// We're now running
		}
	} else if (rotationState == stepperStopping) {					// Est-ce qu'on s'arrête ?
		if ((millis() - playerStartedTime) > playerDuration) {		// Est-ce qu'on a dépassé le temps de lecture ?
			playStop();												// Arrêt du son
			rotationState = stepperStopped;							// Forcer l'état à stoppé
			digitalWrite(runPin, LOW);								// Mettre la sortie rotation à l'état bas (off)
		}
	}

	if (enableEncoder) {
		if (encoder.active()){										// Encodeur actif ?
			encoder.loop();											// Passer la main à l'encodeur
		}
	}
	
	if ((enableEncoder && !encoder.active()) || !enableEncoder) {
		if ((millis() - resultValueTime) > 250) {					// Dernier résultat plus de 500 ms?
			if (enableEncoder) {
				encoder.getAngle(&resultValue);						// Relancer une demande
			}
			if (abs(currentAngle - lastAngleSent) >= 0.1) {			// N'envoyer le message que si la différence est significative
				if (traceDebug) {
					Serial.printf("Angle: %f\n", currentAngle);
				}
				// Envoie un event de type "angle" au navigateur
				char msg[10];										// Buffer pour le message
				snprintf_P(msg, sizeof(msg), PSTR("%.1f"), currentAngle);	// Convertir l'angle en message
				events.send(msg, "angle");							// Envoie l'évènement "angle"
				resultValueTime = millis();							// Charger l'heure du dernier résultat
				lastAngleSent = currentAngle;						// Sauver le dernier angle envoyé
			}
		}
	}

	ArduinoOTA.handle();											// Passer la main à l'OTA
}
