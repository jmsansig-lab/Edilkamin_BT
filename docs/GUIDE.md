# Edilkamin BT Bridge

**Contrôle local des poêles à pellets Edilkamin « The Mind » depuis Home Assistant, en Bluetooth, sans cloud.**
*Local Bluetooth control of Edilkamin “The Mind” pellet stoves from Home Assistant — no cloud.*

[🇫🇷 Français](#-français) · [🇬🇧 English](#-english) · [Installer / Install](https://jmsansig-lab.github.io/Edilkamin_BT/)

---

Basé sur / based on [netmb/Edilkamin_BT](https://github.com/netmb/Edilkamin_BT).

---

## 🇫🇷 Français

### Ce que ça fait

Un ESP32 (Wemos D1 Mini ESP32) se connecte en Bluetooth au poêle et publie tout dans Home Assistant via
MQTT (découverte automatique). Le poêle n'a plus besoin du cloud Edilkamin.

Fonctions héritées de netmb/Edilkamin_BT :
- thermostat (marche/arrêt, consigne, puissance P1–P5, ventilateur 1), ventilateurs 2 et 3 si présents ;
- modes Relax, Auto, Chrono, Standby ;
- capteurs : température ambiante, fumées, état, compteurs d'heures et d'allumages ;
- diagnostics matériels (cartes, versions), acquittement d'alarme.

### Nouveautés de ce fork (v3.0.0)

| Fonction | Détail |
| --- | --- |
| **Dernière alarme** | Nouveau capteur « Last alarm » lu directement dans l'historique d'alarmes du poêle (100 entrées, registre `0x033A`). État = libellé anglais (identique à l'intégration cloud) ; attributs : `code`, `label_fr`, `date` (UTC), `slot`, `logged_alarms`. Idéal pour une notification. |
| **Puissance réelle** | Capteur « Actual power » (`0x02E7`) : la puissance que le poêle applique vraiment (peut différer de la consigne en modulation). |
| **AirKare** | Interrupteur « AirKare » (bit 10 du registre de commande `0x031C`) : seul ce bit est modifié, après relecture ; marche/arrêt du poêle préservé. |
| **Délai de standby** | Capteur « Standby delay » (`0x052D`, secondes), en lecture seule. |
| **Température au dixième** | Le thermostat affiche 0,1 °C au lieu de 0,5 °C. |
| **Mode exploration** | Interrupteur « Mode exploration » (désactivé par défaut) : lit **toute** la mémoire Modbus `0x0000–0x06FF` en lecture seule et publie chaque bloc de 16 registres en hexadécimal. Sert à découvrir de nouveaux paramètres. |
| **Mises à jour sans fil** | Page `http://<IP-de-l-ESP>/update` : envoi de `firmware.bin` depuis le navigateur. |
| **Installateur web** | Compilé et publié automatiquement par GitHub Actions, avec les tests. |

### Matériel

- **Wemos D1 Mini ESP32** (4 Mo de flash). Les autres cartes ne sont pas prises en charge par l'installateur web.
- Poêle Edilkamin compatible avec l'application **The Mind** (Bluetooth). Testé sur **Celia** (canalisable,
  2 ventilateurs) ; le projet d'origine a été développé sur une **Slide2 7-UP**.

### Installation (nouvel ESP32)

1. Ouvre **https://jmsansig-lab.github.io/Edilkamin_BT/** dans Chrome ou Edge sur ordinateur.
2. Branche l'ESP32 en USB (maintiens BOOT si la connexion échoue), clique « Install ».
3. Connecte-toi au Wi-Fi **`Edilkamin_BT_AP`**, saisis ton Wi-Fi, le serveur MQTT (adresse, port, utilisateur,
   mot de passe) et le serveur NTP.
4. **Mets le poêle en mode appairage** lors de la première connexion.
5. L'appareil **Edilkamin** apparaît dans Home Assistant (intégration MQTT).

### Mise à jour depuis une version 2.x ou netmb

La v3 change la table de partitions pour permettre les mises à jour sans fil. Une seule fois :
1. Installe via la page web **sans cocher « Erase device »** (le Wi-Fi et l'appairage Bluetooth sont conservés).
2. L'ESP ouvre le réseau **`Edilkamin_BT_AP`** : ressaisis les réglages MQTT (ils étaient stockés dans
   l'ancienne zone de fichiers).
3. Les entités existantes dans Home Assistant sont conservées (mêmes identifiants MQTT).

### Mises à jour suivantes, sans câble

1. Télécharge `firmware.bin` depuis la page d'installation.
2. Ouvre `http://<IP-de-l-ESP>/update` — utilisateur **`admin`**, mot de passe = **ton mot de passe MQTT**
   (ou `edilkamin` s'il est vide).
3. Envoie le fichier ; l'ESP redémarre seul. Un envoi interrompu ou un fichier invalide ne touche pas
   au firmware en place.

> ⚠️ La page de mise à jour est en HTTP simple : ne l'expose pas sur Internet.

### Comportement Bluetooth (normal)

Le pont se connecte **30 s**, puis se déconnecte **2 min**, en boucle. C'est voulu : le poêle n'accepte
qu'une connexion à la fois, et cette pause laisse l'application The Mind et la télécommande fonctionner.
Les commandes envoyées pendant la pause sont exécutées à la connexion suivante.

### Codes d'alarme

| Code | Libellé (EN) | Libellé (FR) |
| --- | --- | --- |
| 1 | Low incoming air | Manque d'air comburant |
| 2 | Wrong RPM exhaust fan | Vitesse extracteur incorrecte |
| 3 | No flame | Pas de flamme / fin de pellets |
| 4 | Failed ignition | Échec d'allumage |
| 5 | Failed airflow sensor | Défaut capteur de débit d'air |
| 6 | Failed thermocouple | Défaut thermocouple |
| 7 | Exhaust too hot | Fumées trop chaudes |
| 8 | Stove too hot | Poêle en surchauffe |
| 9 | Failed gear motor | Défaut motoréducteur |
| 10 | Circuit board too hot | Carte électronique en surchauffe |
| 11 | Chimney pressure | Pression cheminée |
| 12 | Room temperature sensor failed | Défaut sonde ambiante |
| 13 | Room temperature sensor 2 failed | Défaut sonde ambiante 2 |
| 14 | Room temperature sensor 3 failed | Défaut sonde ambiante 3 |
| 20 | Failed triac gear motor | Défaut triac motoréducteur |
| 21 | Power outage | Coupure de courant |

Un code inconnu est affiché « Code N ».

### Mode exploration (pour aider à décoder)

- Active l'interrupteur **« Mode exploration »**. Le pont lit 112 blocs de 16 registres (fonction Modbus 03,
  **lecture seule**), 6 blocs par fenêtre Bluetooth, en alternance avec les lectures normales. Une passe complète
  prend environ 45 min, puis recommence toutes les heures.
- Chaque bloc devient un capteur « Exploration 0xXXXX » (catégorie diagnostic) dont l'état est la valeur brute
  en hexadécimal ; Home Assistant historise les changements.
- Envoyer `CLEAR` sur `edilkamin/322707E4/scan/set` supprime ces entités.
- Les registres déjà identifiés sont décrits dans **[docs/registers.md](docs/registers.md)**. Les contributions
  (captures avant/après un changement de réglage à la télécommande) sont les bienvenues !

### Sécurité

- Le mode exploration et les nouveaux capteurs **ne font que lire**. Le code refuse avant envoi toute commande
  d'exploration qui ne serait pas une lecture.
- Aucun paramètre de combustion ou technique n'est modifiable par ce firmware.
- Projet communautaire, **non affilié à Edilkamin**. Utilisation à tes risques ; garde toujours les sécurités
  d'origine du poêle.

---

## 🇬🇧 English

### What it does

An ESP32 (Wemos D1 Mini ESP32) talks Bluetooth to the stove and exposes everything to Home Assistant over
MQTT with auto-discovery. No Edilkamin cloud needed.

Inherited from netmb/Edilkamin_BT: climate entity (on/off, setpoint, power P1–P5, fan 1), fans 2 and 3 when
present, Relax / Auto / Chrono / Standby, temperature and status sensors, counters, hardware diagnostics,
alarm acknowledgment.

### What this fork adds (v3.0.0)

- **Last alarm** sensor read from the stove's own 100-entry alarm log (`0x033A`): state is the English label
  (same wording as the cloud integration); attributes `code`, `label_fr`, `date` (UTC), `slot`, `logged_alarms`.
- **Actual power** (`0x02E7`) and **Standby delay** (`0x052D`, seconds, read-only) sensors.
- Room temperature shown with **0.1 °C** precision.
- **Exploration mode** switch (off by default): read-only dump of the whole Modbus map `0x0000–0x06FF`,
  one hex sensor per 16-register block, to help decode new parameters. See [docs/registers.md](docs/registers.md).
- **Wireless updates** at `http://<esp-ip>/update` (user `admin`, password = your MQTT password, or `edilkamin`
  if empty).
- Firmware built, tested and published to GitHub Pages by GitHub Actions.

### Install

1. Open **https://jmsansig-lab.github.io/Edilkamin_BT/** in desktop Chrome or Edge, plug the ESP32 in, click Install.
2. Join the **`Edilkamin_BT_AP`** Wi-Fi network and enter Wi-Fi, MQTT and NTP settings.
3. Put the stove in pairing mode for the first connection. The **Edilkamin** device appears in Home Assistant.

**Upgrading from 2.x / netmb:** v3 changes the partition table (two OTA slots). Flash **without “Erase device”**:
Wi-Fi and the Bluetooth bond are kept, but the MQTT settings must be entered once more in `Edilkamin_BT_AP`.
Home Assistant entities keep their IDs.

**Bluetooth cycle:** 30 s connected / 2 min paused, by design, so The Mind app and the remote still work.

**Safety:** exploration and new sensors are read-only; no combustion or technical parameter can be written.
Community project, **not affiliated with Edilkamin**; use at your own risk.

---

## Développement / Development

```bash
pio pkg install -e wemos_d1_mini32
bash tests/run_host_tests.sh      # host tests (ASan + UBSan), BLE/AES mocked
PIO_CMD=$(command -v pio) bash build_webflash.sh
```

- Partitions : `nvs` · `otadata` · `app0`/`app1` (2 × 1,875 Mo) · LittleFS 192 Ko (`partitions.csv`).
- MQTT : préfixe `edilkamin/322707E4/…` ; découverte `homeassistant/…`.
- Documentation technique d'origine (protocole BLE, ventilateurs, diagnostics) :
  [docs/UPSTREAM_README.md](docs/UPSTREAM_README.md).

## Licence / License

Le dépôt d'origine ne précise pas de licence. Ce fork reste un travail dérivé de netmb/Edilkamin_BT ; une
licence sera ajoutée en accord avec l'auteur d'origine. Les améliorations de ce fork peuvent être proposées
en retour au projet d'origine.

*The upstream repository has no license file. This fork is a derivative of netmb/Edilkamin_BT; a license will
be added in agreement with the original author, and improvements can be contributed back upstream.*
