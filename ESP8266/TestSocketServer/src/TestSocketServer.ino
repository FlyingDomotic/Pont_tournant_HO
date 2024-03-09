/*
 *	   Exemple de serveur Web avec gestion d'un bouton et d'une LED (14-WebOta)
 *
 *		Change l'état de la LED lorsque le bouton est poussé.
 *		Le serveur Web embarque indique l'état de la LED.
 *		Il permet aussi de forcer son allumage, son extinction ou sa bascule.
 *		Le serveur Web asynchrone propose 3 boutons pour changer l'état de la LED.
 *		Version avec OTA (mise à jour du firmware par le réseau)
 *
 *	Matériel
 *		ESP8266 version NodeMCU
 *			LED connectée en D4, allumé=bas
 *			Bouton connecté en D3, poussé=bas
 *
 *	V1.0.0 FF - Juillet 2023 - Pour le FabLab
 *
 *	GNU GENERAL PUBLIC LICENSE - Version 3, 29 June 2007
 *
 */

#define DEBUG												// Trace des états et actions
#define TRACE_OTA											// Trace la mise à jour par le réseau
//#define POINT_ACCES_PRIVE									// Embarque un point d'accès privé (sinon, se connecte sur un Wifi existant)
#define NOM_ESP "MonEsp8266"								// Nom DNS de cet ESP (doit être unique sur le réseau)

#define PINOCHE_BOUTON D3									// Entrée ESP du bouton
#define PINOCHE_LED D4										// Sortie ESP de la LED
#define BOUTON_PULL_UP false								// Bouton sans tirage au niveau haut
#define ALLUME_LED LOW										// LED allumée à l'état BAS
#define ETEINS_LED HIGH										// LED éteinte à l'état HAUT
#define BOUTON_ANTI_REBOND (50)								// Anti-rebond à 50 ms
#define BOUTON_BAS true										// Bouton actif au niveau bas

#include <EasyButton.h>										// Librairie Easy Button
#include <ESP8266WiFi.h>									// Librairie Wifi
#include <ESPAsyncWebServer.h>								// Librairie serveur Web asynchrone
#include <ArduinoOTA.h>										// Librairie OTA (mise à jour par le réseau)

// Définition du nom du point d'accès à créer
const char* ssid = "WIFI_SERVEUR_WEB";
const char* password = "PASSWORD";
 
// Classe serveur Web asynchrone
AsyncWebServer serveurWeb(80);								// Utiliser le port 80
 
// Variables
int etatLed = ETEINS_LED;									// État initial de la LED

// Classe Easy Button
EasyButton bouton(PINOCHE_BOUTON, BOUTON_ANTI_REBOND, BOUTON_BAS, BOUTON_PULL_UP);

// Aligne l'étt de la LED avec le contenu de la variable etatLed
void modifieLed() {
	digitalWrite(PINOCHE_LED, etatLed);						// Écrire le nouvel état de la LED
	#ifdef DEBUG
		if (etatLed == ALLUME_LED) {						// Est-ce que la LED est allumée ?
			Serial.println("La LED est allumée");
		} else {
			Serial.println("La LED est éteinte");
		}
	#endif
}

// Renvoie l'état de la LED
void afficheEtat(AsyncWebServerRequest *requete) {
	String message = "";									// Contenu de la réponse
	message += "<!DOCTYPE HTML>\n";							// Document de type HTML
	message += "<meta charset=\"UTF-8\">\n";				// Indiquer qu'on utilise des caractères accentués
	message += "<html>\n";									// Début du HTML
	message += "Bienvenue sur le serveur Web de test du FabLab !<br>\n"; // Message
	message += "<br><br>\n";
	message += "La LED est ";
	message += etatLed == ALLUME_LED ? "allumée":"éteinte";
	message += "\n<br><br>";
	message += "<a href=\"/LED=ON\"\"><button>Allumer</button></a>\n";
	message += "<a href=\"/LED=OFF\"\"><button>Éteindre</button></a>\n";  
	message += "<a href=\"/LED=INVERSE\"\"><button>Inverser</button></a>\n";  
	message += "</html>\n";									// Fin du HTML
	requete->send_P(200, "text/html", message.c_str());
}

// Routine activée lors de la réception de la commande /LED=ON
void setLedOn(AsyncWebServerRequest *requete) {
	#ifdef DEBUG
		Serial.println("Force la LED on");
	#endif
	etatLed = ALLUME_LED;
	modifieLed();
	afficheEtat(requete);
}

// Routine activée lors de la réception de la commande /LED=OFF
void setLedOff(AsyncWebServerRequest *requete) {
	#ifdef DEBUG
		Serial.println("Force la LED off");
	#endif
	etatLed = ETEINS_LED;
	modifieLed();
	afficheEtat(requete);
}
// Routine activée lors de la réception de la commande /LED=INVERSE
void setLedInverse(AsyncWebServerRequest *requete) {
	#ifdef DEBUG
		Serial.println("Bascule l'état de la LED");
	#endif
	etatLed = !etatLed;
	modifieLed();
	afficheEtat(requete);
}

// Routine activée lors de l'appui sur le bouton
void appuiBouton() {
	#ifdef DEBUG
		Serial.println("Le bouton est poussé");				// Tracer l'état du bouton
	#endif
	etatLed = ! etatLed;									// Oui, inverser l'état de la LED
	modifieLed();
}

#if defined(DEBUG) && defined(TRACE_OTA)
	void onStartOTA() {
		if (ArduinoOTA.getCommand() == U_FLASH) {
			Serial.println("Début de mise à jour du logiciel");
		} else { // U_FS
			Serial.println("Début de mise à jour du système de fichiers");
			//FS.end();									// Dé-commenter la ligne si on utilise le système de fichier
		}
	}
		
	void onEndOTA() {
		Serial.println("Fin de mise à jour");
	}
			
	void onProgressOTA(unsigned int progres, unsigned int total) {
		Serial.print("Progrès MAJ: ");
		Serial.println(progres / (total / 100));
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
	#endif

// Initialisation au lancement du programme
void setup() {
	#ifdef DEBUG
		Serial.begin(74880);								// Initialise le port série à 74880 bds (~ 7.500 caractères par seconde)
		Serial.println("Le programme s'initialise ...");
	#endif
	digitalWrite(PINOCHE_LED, ETEINS_LED);					// Éteins la LED
	pinMode(PINOCHE_LED, OUTPUT);							// Définit la pinoche de la LED en sortie
	bouton.begin();											// Lance la gestion du bouton
	bouton.onPressed(appuiBouton);							// Routine à appeler lors de l'appui sur le bouton

	WiFi.hostname(NOM_ESP);									// Définit le nom de ce module
	IPAddress adresseIP;									// Adresse IP du serveur ou du client

	#ifdef POINT_ACCES_PRIVE								// Si on a demandé un point d'accès privé
		#ifdef DEBUG
			Serial.print("Création du Wifi ");
			Serial.println(ssid);
		#endif
		WiFi.softAP(ssid, password);						// Démarrer un point d'accès Wifi
		adresseIP = WiFi.softAPIP();						// Adresse IP de ce serveur
	#else													// Sinon, on se connecte à un réseau existant
		#ifdef DEBUG
			Serial.print("Connexion au Wifi ");
			Serial.print(ssid);
			Serial.print(" ");
		#endif

		WiFi.begin(ssid, password);							// Démarrer la connexion à un accès Wifi existant
		while (WiFi.status() != WL_CONNECTED) {				// Tant que le Wifi n'est pas connecté ?
			delay(500);										// Attendre 0,5 s
			#ifdef DEBUG
				Serial.print(".");							// Afficher un point
			#endif
		}													// Et reboucler
		adresseIP = WiFi.localIP();							// Adresse IP du client
	#endif

	#ifdef DEBUG
		Serial.println();
		Serial.print("URL de connexion: http://");
		Serial.print(adresseIP);							// Afficher l'adresse IP de ce serveur/client
		Serial.print("/ ou http://");
		Serial.print(NOM_ESP);
		Serial.println("/");
	#endif

	// Trace du process de mise à jour par le réseau (optionnel)
	#if defined(DEBUG) && defined(TRACE_OTA)
		ArduinoOTA.onStart(onStartOTA);
		ArduinoOTA.onEnd(onEndOTA);
		ArduinoOTA.onProgress(onProgressOTA);
		ArduinoOTA.onError(onErrorOTA);
	#endif

	//ArduinoOTA.setPassword("mon mot de passe");			// Dé-commenter pour associer un mot de passe à l'OTA
	ArduinoOTA.begin();										// Initialise la mise à jour par le réseau

	serveurWeb.on("/", HTTP_GET, afficheEtat);				// Routines réalisant les différente actions
	serveurWeb.on("/LED=ON", HTTP_GET, setLedOn);
	serveurWeb.on("/LED=OFF", HTTP_GET, setLedOff);
	serveurWeb.on("/LED=INVERSE", HTTP_GET, setLedInverse);
	serveurWeb.onNotFound (afficheEtat);					// Routine a appeler si l'URL donnée n'est pas dans la liste ci-dessus

	serveurWeb.begin();										// Lancer le serveur Web
}

// Boucle permanente
void loop() {
	bouton.read();											// Passer la main à la classe EasyButton
	ArduinoOTA.handle();									// Passer la main à l'OTA
}
