# Version 2026.11.10-jms1

Base : netmb/Edilkamin_BT 2026.11.10 (validée le 19/11/2026 sur un cycle complet
allumage → combustion → extinction, poêle canalisable, 2 ventilateurs).

## Changements

1. **Température ambiante au dixième** : la découverte HA du thermostat passe de
   `precision: 0.5` à `0.1`. Le pas de consigne reste 0,5 °C.
2. **Mode exploration (lecture seule)**, désactivé au démarrage.
   - Interrupteur HA « Mode exploration » (`edilkamin/322707E4/scan/set`,
     `ON` / `OFF` / `CLEAR`).
   - Lit 46 blocs de 16 registres (fonction Modbus 03 uniquement) :
     `0x02B0–0x030F`, `0x0310–0x046F`, `0x04C0–0x04FF`, `0x0500–0x05AF`,
     `0x0600–0x062F`.
   - Chaque bloc devient un capteur diagnostic « Exploration 0xXXXX » dont l'état
     est la valeur brute en hexadécimal (64 caractères), historisée par HA.
   - Ordonnancement : un créneau de requête sur deux, 6 blocs maximum par fenêtre
     Bluetooth de 30 s. La pause de 2 min et les commandes utilisateur sont
     inchangées et restent prioritaires.
   - Une passe complète dure environ 8 fenêtres (~20 min), puis le scan attend
     30 min avant la passe suivante.
   - Un bloc en erreur (exception Modbus ou délai dépassé) affiche `ERREUR` et
     n'est plus lu après deux échecs, jusqu'au prochain redémarrage.
   - `CLEAR` arrête le scan et supprime les entités et valeurs retenues.

## Garanties vérifiées par les tests hôte

- Aucune commande du scan n'est une écriture (fonction 3, 16 registres) ; une
  commande falsifiée dans la file est refusée avant envoi.
- Réponse avec CRC invalide ignorée ; exception Modbus → `ERREUR`.
- Budget par fenêtre, alternance avec le polling normal, garde MTU (≥ 67),
  aucun scan sur le créneau de réveil sans connexion.
- Tous les tests d'origine passent toujours (AddressSanitizer + UBSan).

## Non vérifié à ce stade

- Compilation ESP32 et essai sur le poêle (faits par GitHub Actions puis au flash).
- Comportement du boîtier Bluetooth du poêle sur des lectures de 16 registres.
