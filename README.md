# Railway turntable control / Contrôle d'une plaque tournante ferroviaire

[English version and French version in the same document]

Controls a railway turntable with a RPi, a stepper, rotation sensor through a web interface

[ Versions françaises et anglaises dans le même document]

Contrôle une plaque tournante ferroviaire au travers d'un RPi, avec un moteur pas à pas, un capteur de rotation, via une interface Web

## What's for?/A quoi ça sert ?

This code has been written to help a FabLab's member to implement a railway turntable for it HO scale railway, just clicking on an image of turntable. He already bought one RPI 4, with a WaveShare's RS485 CAN HAT, an ACCNT QY3806-485RTU rotary encoder, a NMEA 29 stepper with a DM556 driver, and power supply to feed them.

It would probably be more efficient to use AN ESP8266 instead of a RPI 4,and perhaps a zero position sensor instead of rotary encoder, by equipments where already bought and partly installed.

On command side, idea was to have an image or turntable, and just click on track where we want to rotate turntable.

Ce code a été élaboré à la demande d'un membre du FabLab, qui souhaitait commander une plaque tournante sur un circuit ferroviaire à l'échelle HO, en cliquant sur une image de la plaque. Il avait déjà acheté un RPI 4, avec une interface RS485 CAN HAT de chez waveshare, un capteur de rotation modbus QY3806-485RTU de chez ACCNT, un moteur pas à pas NMEA 29 avec un driver DM556, et les alimentations nécessaires pour tout ça.

Il aurait problement été plus efficace d'utiliser un ESP8266 plutôt qu'un RaspBerry, et peut être un détecteur de zéro plutôt qu'un capteur de rotation, mais le matériel était déjà acheté et en place sur la maquette.

Côté commande, l'idée est d'avoir une image du pont et de cliquer sur la voie vers laquelle on souhaite orienter le pont.

## Driving choices / Choix retenus

Idea is to create a web server, which will display a turntable image, and detect clicks (or pushes on a touch screen). Clicks close to a point on image will rotate turntable. Rotation angle will be computed from current angle (from rotary encoder), and angle associated to the point.

Code will be written in Python, as it easily manages web server, GPIO to drive DM556, as well as modbus rotary encoder. An HTML file will display turntable image and return click relative position (0%-100%), allowing to reduce/enlarge image without impact.

L'idée est de créer un serveur Web, qui affichera une image du pont et détectera les clics (ou les appuis sur un écran tactile). Les clics proches d'un point sur l'image déclencheront la rotation du pont. L'angle de rotation sera déterminé à partir de l'angle actuel (récupéré par le capteur de position) et l'angle associé au point.

Le code sera écrit en Python, qui gère facilement un serveur Web, les GPIO pour commander le DM556, ainsi que la parti modbus du capteur de rotation. Un fichier HTML affichera l'image et retournera la position relative (0%-100%) en x et y de l'image cliquée, pour permettre d'agrandir ou réduire l'image sans impact.

## Interfaces

Connection between RPi and rotary encoder is done by RS485 CAN HAT, which is a Hat plugged on RPi connector. A small interface, based on 4N25 optocoupler, with a limitation resistor on LED side, has been designed to connect 24V DM556 driver to 3.3V RPi.

La connexion entre le RPi et le capteur de rotation est réalisée par le module RS485 CAN HAT, qui est un module gigogne "planté" sur le connecteur du RPi. Une petite interface à base d'opto coupleurs 4N25, avec une résistance de limitation côté LED, a été réalisée pour connecter le DM556 alimenté en 24V, alors que le RPi est en 3,3V.

## Prerequisites/Prérequis

Python should be installed on RPi (which is done by default)

In addition, RPi.GPIO, pymodbus, aiohttp and websocket modules should be installed before running code.

Python doit être installé sur le RPi (c'est fait de base).

Les modules RPi.GPIO, pymodbus, aiohttp et websocket doivent installés par PIP avant de lancer le code.

## Installation

Follow these steps:

1. Clone repository in folder where you want to install it
```
cd <where_ever_you_want>
git clone https://github.com/FlyingDomotic/railwayTurntable.git railwayTurntable
```
2. Change .py files protection to allow execution (chmod +x *.py)
3. Follow configuration steps hereunder.

Suivez ces étapes :

1. Clonez le dépôt GitHub dans le répertoire où vous souhaitez l'installer
```
cd <là_où_vous_voulez_l'installer>
git clone https://github.com/FlyingDomotic/railwayTurntable.git railwayTurntable
```
2. Modifier la protection des fichiers .py pour autoriser l'exécution (chmod +x *.py)
3. Suivez les étapes de configuration ci-dessous.

## Update / Mise à jour

1. Go to folder where you installed railwayTurntable and pull new version
```
cd <xxxx/railwayTurntable>
git pull
```
2. Restart web server

Note: if you did any changes in files and `git pull` command doesn't work for you anymore, you could stash all local changes using
```
git stash
```
or
```
git checkout <modified file>
```

1. Aller dans le répertoire où vous avez installé le code et charger la nouvelle version
```
cd <xxxx/railwayTurntable>
git pull
```
2. Relancez le serveur web

Note: si vous avez fait des modifs dans les fichiers du plug-in et que la commande `git pull` ne fonctionne pas, vous pouvez écraser les modifications locales avec la commande
```
git stash
```
or
```
git checkout <fichier modifié>
```

## Settings/Paramètres

All scripts settings are done in constants.py file. During initial installation, you should rename constantsTemplate.py to constants.py.

Part of configuration is dedicated to hardware settings (given values are those used by FabLab's configuration, yours could be different). These parameters are to be set before running scripts:
	- degreesPerStep = 1.8 : Number of degrees rotated by one stepper step
	- microStepsPerStep = 2 : Number of micro-steps per step
	- driverMinimalMicroSec = 3 : Number of micro-seconds between 2 variation of signal to be taken by stepper driver
	- requiredRPM = 2 : Number of rotation per minute of turntable
	- pulseGpio = 23 : Number of GPIO used to send pulse
	- directionGpio = 24 : Number of GPIO used to change rotation direction
	- enableGpio = 22 : Number of GPIO used to enable stepper
	- traceDebug = True : Dusplay debug messages if set to True

Remaining parameters are linked to railway tracks. Base is to number them from 1 to n, in a continuous way (15 in our case). You need then to dtermine exact angles of each track, knowing that there's an offset to displace rotation origin.

Initially, define offset to zero, and approximate values for each track angle :
	- offsetAngle = 0 : Offset angle to change rotation origin
	- indexes = [0.0, 30.0, 60.0, 75.0, 90.0, 105.0, 120.0, 135.0, 150.0, 165.0, 180.0, 195.0, 210.0, 225.0, 270.0] : Angles of different tracks, in trigonometric way (which is reverse clockwise). Count of values should be in line with existing connected tracks.

Then start stepperAlign.py script, after positioning turntable face to track #1. Turn table with +x.x or -x.x where x.x represents an angle (for example +1.4 to turn table of one degree and four tenth of degree). When turntable's rails are exactly face to track #1, note angle value and report it into "offsetAngle", putting "-" in front of value.

Restart stepperAlign.py script, input "1" and verify that track #1 is exactly in front of turntable rails. If not, reset "offsetAngle" to zero and rerun script to redo the preceding procedure.

We'll then setup each track (2, 3...), inputing its number , then +/-x.x to precisely position track in front of turntable rails, and note angle value, which will be set into "indexes" variable.

When all values will be adjusted into constants.py file, rerun stepperAlign.py script and check that all tracks are correctly align, after inputing their number. Adjust if needed.

We have now tp setup web interface. Take a photo of turntable, in top view. Convert it in JPEG format, adjust size to display it in one part in your browser, name it "imagePont.jpg", and copy it in script folder.

Start "webServer.py" script, connect on RPi address with browser, on port 8000, and open "test.html" file. For example: "http://192.168.1.4:8000/test.html"

Click then on each track reference point. Write down x and y percentages, that should be written and write them into lines :
	- xPos = [91.7, 86.2, 68.0, 57.7, 47.2, 34.7, 25.7, 18.0, 11.7, 9.5, 8.7, 11.0, 15.2, 21.2, 50.7] : x percentage of each track on image,
	- yPos = [51.4, 27.9, 10.1, 6.5, 5.4, 8.1, 13.7, 20.5, 31.7, 41.6, 52.6, 63.8, 73.3, 81.1, 94.4] : y percentage of each track on image.

Lastly, it's possible to set click sensitivity aournd reference points using:
	- maxDelta = 3 : Percentage authorized around reference points

When setup is satisfying, we can use turntable connecting browser to RPI default's page, for example "http://192.168.1.4:8000/"

Le paramétrage de l'ensemble des scripts se fait dans le fichier constants.py. Lors de la première installation, renommer le fichier constantsTemplate.py en constants.py.

Une partie des paramètres concerne les caractéristiques physiques de la configuration (les valeurs indiquées sont celles utilisées par la réalisation du FabLab, les votres peuvent être différentes). Ces paramètres sont à définir avant de lancer les scripts :
	- degreesPerStep = 1.8 : Nombre de degrés de rotation de la plaque par pas du moteur
	- microStepsPerStep = 2 : Nombre de micro-pas par pas
	- driverMinimalMicroSec = 3 : Nombre de micro-secondes minimal entre 2 variation d'un signal pour être pris en compte par le driver du moteur
	- requiredRPM = 2 : Nombre de tours par minute de la plaque
	- pulseGpio = 23 : Numéro de GPIO de la commande d'impulsion (pulse)
	- directionGpio = 24 : Numéro de GPIO de la commande de direction
	- enableGpio = 25 : Numéro de GPIO de la commande d'activation (enable)
	- traceDebug = True : Affiche des messages de trace si défini à True

Les paramètres restants sont liés aux voies. Le principe de base est de les numéroter de 1 à n, de façon continue (dans notre cas 15). Il faut ensuite déterminer les angles exacts de rotation pour chaque voie, sachant qu'il est possible d'utiliser un offset pour recaler l'origine de la rotation.

Initialement, définir l'offset à zéro, et des valeurs approximatives pour les angles des différentes voies :
	- offsetAngle = 0 : Angle de recalage de l'origine de la rotation
	- indexes = [0.0, 30.0, 60.0, 75.0, 90.0, 105.0, 120.0, 135.0, 150.0, 165.0, 180.0, 195.0, 210.0, 225.0, 270.0] : Angles des différentes voies (dans le sens trigonométrique, c'est à dire inverse des aiguilles d'une montre). Le nombre de valeurs doit refléter le nombre de voies présentes.

Lancer ensuite le script stepperAlign.py, après avoir positionné la plaque en face de la voie 1. Faire tourner la plaque par les commandes +x.x ou -x.x où x.x représente un angle (par exemple +1.4 pour faire tourner la plaque d'un degré et quatre dixièmes de degrés). Lorsque les voies sont exactement en face de la voie 1, noter la valeur de l'angle, et l'indiquer dans "offsetAngle" en le faisant précéder du signe '-'.

Relancer le script stepperAlign.py, saisir "1" et vérifier que la voie 1 est pile en face de la plaque. Sinon, remettre "offsetAngle" à zéro et relancer le script pour refaire la procédure précédente.

On va ensuite caler chacune des voies, en saissant son numéro (2, 3, ...), puis +/-x.x pour positionner finement la voie, et noter précieusement la valeur de l'angle, qui sera reportée dans la variables "indexes".

Une fois toutes les valeurs ajustées dans le fichier constants.py, relancer le script stepperAlign.py et vérifier que chaque voie est bien alignée après avoir saisi son numéro. Ajuster si nécessaire.

Reste à paramétrer l'interface Web. Prendre une photo de la plaque, vue de dessus. La convertir en JPEG, ajuster la taille pour qu'elle soit affichable en une seule fois dans votre nagigateur, la nommer "imagePont.jpg", puis la copier dans le répertoire des scripts.

Lancer le script "webServer.py", et se connecter avec le navigateur sur l'adresse IP du RPI, sur le port 8000, et ouvrir le fichier test.html. Par exemple : "http://192.168.1.4:8000/test.html"

Cliquer ensuite sur chacun des points de référence des voies. Noter les pourcentages x et y, qu'il faudra reporter dans constants.py, sur les lignes :
	- xPos = [91.7, 86.2, 68.0, 57.7, 47.2, 34.7, 25.7, 18.0, 11.7, 9.5, 8.7, 11.0, 15.2, 21.2, 50.7] : pourcentage des x de chaque voie dans l'image,
	- yPos = [51.4, 27.9, 10.1, 6.5, 5.4, 8.1, 13.7, 20.5, 31.7, 41.6, 52.6, 63.8, 73.3, 81.1, 94.4] : pourcentage des y de chaque voie dans l'image.

Enfin, il est possible de régler la sensibilité du clic autour des points de réference en utilisant :
	- maxDelta = 3 : Pourcentage autorisé autour des points de référence

Une fois le paramétrage satisfaisant, on peut utiliser le pont en connectant son navigateur sur la page par défaut sur RPi, par exemple "http://192.168.1.4:8000/"

Pins and GPIO used/Pinoches et GPIO utilisées

The following pins and GPIO are used by the original hardware:

Les pinoches et GPIO suivantes sont utilisées par le montage d'origine :

+--------+----------+-------------------+------+--------+
| Device | Function |    Description    |  Pin |  GPIO  |
+--------+----------+-------------------+------+--------+
|  CAN   |    3V3   |  3.3V Power Input | 3.3V |        |
|  CAN   |    GND   |      Ground       |  GND |        |
|  CAN   |    SCK   |  SPI Clock Input  |  23  | GPIO11 |
|  CAN   |   MOSI   |  SPI Data Input   |  21  |  GPIO9 |
|  CAN   |   MISO   |  SPI Data Output  |  19  | GPIO10 |
|  CAN   |    CS    | Command Selection |  24  |  GPIO8 |
|  CAN   |    INT   |  Interrupt Output |  22  | GPIO25 |
|  RS485 |    3V3   |  3.3V Power Input | 3.3V |        |
|  RS485 |    GND   |      Ground       |  GND |        |
|  RS485 |    RXD   |   UART Receives   |  10  | GPIO15 |
|  RS485 |    TXD   |  UART Transmits   |  8   | GPIO14 |
|  RS485 |    RSE   |   Tx/Rx Control   |  26  |  GPIO7 |
|  DM556 |    PUL   |       Pulse       |  16  | GPIO23 |
|  DM556 |    DIR   |     Direction     |  18  | GPIO24 |
|  DM556 |    ENA   |      Enable       |  15  | GPIO22 |
+--------+----------+-------------------+------+--------+
