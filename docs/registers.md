# Carte des registres Modbus / Modbus register map

Observé sur une **Celia** (carte LX0320, The Mind), via le mode exploration (lecture FC03 seule).
Les valeurs sont des registres 16 bits big-endian. *Observed on a Celia stove (read-only FC03).*

Légende / legend : ✅ confirmé (recoupé avec la télécommande, l'app ou le cloud) · 🟡 probable · ❔ à confirmer.

## Identification

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x000A` (8) | Numéro de série carte mère (texte) | ✅ |
| `0x0012` (8) | Nom carte mère (texte, ex. `LX0320 2.0`) | ✅ |
| `0x001A`–`0x0039` | Bootloader / application et versions (texte) | ✅ |
| `0x0044`, `0x0064` | Module de connexion : nom, version | ✅ |
| `0x0076`, `0x0096` | Interface locale : nom, version | ✅ |
| `0x0206`, `0x0226` | Module radio : nom, version | ✅ |
| `0x0238`, `0x0258` | Panneau de commande : nom, version | ✅ |
| `0x02FE` (octet bas) | Révision matérielle carte mère (dixièmes) | ✅ |
| `0x061F` | Code base de données | ✅ |

## État de fonctionnement

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x02BB` | Température fumées mesurée (dixièmes de °C) | ✅ |
| `0x02BC` | Température fumées visée (dixièmes) : P1 150,0 · P2 172,5 · P3 195,0 · P5 240,0 ; 0 à l'arrêt | ✅ |
| `0x02C6` / `0x02C7` | Extracteur : vitesse visée / mesurée (tr/min) : P1 ~1 450 · P2 ~1 780 · P3 ~1 970 · allumage ~2 090 · extinction 1 000 | ✅ |
| `0x02B2` / `0x02B7` | Températures internes (dixièmes) : ~30 °C / ~50 °C en chauffe, retour vers l'ambiante à l'arrêt | 🟡 |
| `0x02F4` | Bit `0x0040` = AirKare actif | ✅ |
| `0x02AA` | 0x3000 à l'arrêt, 0x2000 en chauffe | 🟡 |
| `0x02E4` | Phase (octet bas) / sous-phase (octet haut) | ✅ |
| `0x02E5` | Détail (octet bas) / état (octet haut) | ✅ |
| `0x02E6` | Sous-état / alarme brute | ✅ |
| `0x02E7` (octet bas) | **Puissance réellement appliquée** (1–5) | ✅ |
| `0x02E9` | Température ambiante (dixièmes de °C) | ✅ |
| `0x02FC` | Heure de début du standby | 🟡 |
| `0x0316`–`0x0319` | Horloge du poêle | ✅ |

## Commandes

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x031C` | Registre de commande partagé : bit 0 = marche/arrêt, bit 10 (`0x0400`) = AirKare. Écrit en lecture-modification-écriture, un seul bit à la fois. | ✅ |

## Compteurs

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x0334` | Nombre d'allumages | ✅ |
| `0x0335`–`0x0339` | Heures de fonctionnement P1 à P5 | ✅ |

## Historique d'alarmes

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x033A` | Octet haut : index de la prochaine case libre (0–99) ; octet bas : nombre d'alarmes enregistrées | ✅ |
| `0x033C` + 4×n | Case n (0–99) : horodatage UNIX UTC (mot bas, mot haut), code d'alarme, `0x00FF` | ✅ |

Exemple : `52FD 6A02 0015 00FF` → code 21 (coupure de courant) le 2026-05-11 22:06:53 UTC.

## Réglages utilisateur

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x0525` | Consigne de température | ✅ |
| `0x052C` | Standby activé / désactivé | ✅ |
| `0x052D` | Délai de standby (secondes) — 300 = décompte de la télécommande | ✅ |
| `0x0546` (octet haut) | Q.té granulé (menu 1111), % signé : BAS -15 · MOYEN 0 · HAUT +15 (probable) | ✅ |
| `0x0548` (octet bas) | Configuration ventilateurs (voir README d'origine) | ✅ |
| `0x052A` | Ventilateurs 3 / 2 (octet haut / bas) : 0 = arrêt (canalisation coupée), 1–5 = 20–100 %, 6 = auto | ✅ |
| `0x0579` ou `0x052F` | Hystérésis ambiante (dixièmes, 10 = 1,0 °C) — test 1,0 → 1,1 à faire | ❔ |

## Menu technique (lecture seule)

| Registre | Réglage télécommande | Valeur Celia |
| --- | --- | --- |
| `0x057D` | T° fumée vent1 ON (dixièmes) | 110 °C |
| `0x057E` | Hystérésis vent1 (dixièmes) | 30 °C |
| `0x057F` | T° fum. vent2-3 ON (dixièmes) | 120 °C |
| `0x0580` | Hystérésis vent 2-3 (dixièmes) | 20 °C |
| `0x0581` | Durée nettoyage initial (s) | 30 |
| `0x0582` | Durée charge pellets (s) | 140 |
| `0x0583` | Durée charge supplémentaire (s) | 90 |
| `0x0584` | t CHARGE MAN (s) | 170 |
| `0x0585` | Attente flamme (s) | 50 |
| `0x0586` | Nettoyage chaud Leonardo (adim) | 180 |
| `0x0587` | Extracteur nettoyage chaud (rpm) | 2500 |
| `0x0588` | ? (écran non photographié) | 50 |
| `0x0589` | Nettoyage initial Leonardo (adim) | 165 |
| `0x058A` | Extracteur nettoyage initial (rpm) | 2500 |
| `0x058B` | ? (écran non photographié) | 0xFF32 |
| `0x058C` | Durée allumage 1 (s) | 480 |
| `0x058D` | Durée allumage 2 (s) | 600 |
| `0x058E` | Leonardo allumage 1 (adim) | 220 |
| `0x058F` | Leonardo allumage 2 (adim) | 172 |
| `0x0590` | Extracteur allumage 1 (rpm) | 2100 |
| `0x0591` | Extracteur allumage 2 (rpm) | 1800 |
| `0x0593` | RPM vis allumage | 1500 |
| `0x0594` | DT allumage (dixièmes) | 10 °C |
| `0x0595` | T° fumées ON (dixièmes) | 70 °C |
| `0x0596` | Durée warmup (s) | 240 |
| `0x0597` | RPM vis warmup | 1000 |
| `0x0598` | Leonardo warmup (adim) | 170 |
| `0x0599` | RPM extracteur warmup | 2000 |
| `0x059B` | Fréquence nettoyage P3-5 (min) | 20 |
| `0x059C` | Fréquence nettoyage P1-2 (min) | 25 |
| `0x059D` | Durée nettoyage P3-5 (s) | 20 |
| `0x059E` | Durée nettoyage P1-2 (s) | 20 |
| `0x059F` | RPM nettoyage P3-5 | 2500 |
| `0x05A0` | RPM nettoyage P1-2 | 2500 |
| `0x05A2` | Vis nettoyage P3-5 (rpm) | 0 |
| `0x05A3` | Vis nettoyage P1-2 (rpm) | 0 |
| `0x05A4` | RPM extracteur extinction | 2500 |
| `0x05A5` | RPM extinction limite | 1000 |
| `0x05A7` | Durée extinction (s) | 500 |
| `0x05A8` | T° fumées OFF (dixièmes) | 80 °C |
| `0x05A9` | RPM extracteur refroidissement | 1000 |
| `0x05AC` / `0x05AD` | T° fumées P.MIN / P.MAX (dixièmes) | 150 / 240 °C |
| `0x05AE` | Leonardo P.MAX (adim) | 148 |
| `0x05B3` | Leonardo P.MIN (adim) | 120 |
| `0x05B5`–`0x05B7` (octets, bas puis haut) | Ventilateur 1 front V1→V5, % | 25 / 30 / 45 / 75 / 100 |
| `0x05B7` haut → `0x05B9` | Ventilateur 2 canal V1→V5, % | 20 / 30 / 45 / 60 / 100 |
| `0x05BA`–`0x05BC` bas | Ventilateur 3 canal V1→V5, % | 65 / 70 / 75 / 85 / 100 |
| `0x05BD` | Extraction d'urgence P1 (octet bas) / P2 (octet haut), % | 40 / 41 |
| `0x05BE` | Extraction d'urgence P3 (octet bas) / P4 (octet haut), % | 42 / 44 |
| `0x05BF` | Extraction d'urgence P5 (octet bas), % | 46 |

Octets regroupés : lecture octet bas puis octet haut, dans l'ordre du menu. `0x05BC` octet haut = EXTR. EN URGENCE
(0 = OFF) 🟡.

| Registre | Écran | Valeur |
| --- | --- | --- |
| `0x05AF` | RPM EXTR. P.MAX | 1800 ✅ |
| `0x05AB` | RPM VIS P.MAX | 2200 ✅ |
| `0x05B0` / `0x05B1` | RPM EXTR P.MIN / RPM VIS P.MIN (ordre exact à confirmer) | 1000 / 1000 🟡 |
| `0x0578` | 2 octets à 1 : probablement LEONARDO ON + AUTOREGULATION ON | 🟡 |

Écrans sans registre identifié : ANTI-BLOC.EXTR. (OFF), REGENERATION (OFF), EKLEANER (OFF), FONCTION AIRKARE
(GRANDE), CAPTEUR PELLET (OFF), NOMBRE VENTILO (2), RESET AIRKARE (action, pas de valeur). Les ON/OFF ne suivent
pas l'ordre du menu : ils sont stockés ailleurs (bits). Valeurs sans écran (réglages internes) : `0x0588` (50),
`0x05B2` (1500), `0x05B4` (1000), `0x0592` / `0x05A1` (0x3232), `0x058B` / `0x059A` / `0x05AA` (0xFFxx), `0x05A6`.

## Paramètres techniques

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x0570`–`0x05AF` | Bloc des paramètres techniques (menu installateur) | 🟡 — **jamais écrit par ce firmware** |

## Recherches en cours

- **Hystérésis de standby** (mesurée sur Celia, consigne 23,0) : arrêt quand l'ambiante dépasse la consigne pendant le
  délai de standby ; redémarrage à 21,9 °C, soit sous consigne − 1,0 °C. Aucun réglage dans les menus : valeur fixe.
- Vitesse extracteur vs vis sans fin (tr/min) : à recouper avec les écrans d'info de la télécommande.

**Comment contribuer :** active le mode exploration, note l'heure, change **un seul** réglage à la télécommande,
attends une passe complète (~45 min) et compare l'historique des capteurs « Exploration 0xXXXX ». Ouvre une issue
avec le modèle du poêle, le réglage modifié (avant → après) et le bloc qui a changé.

## Live (v3.3.1)

| Registre | Signification | Statut |
| --- | --- | --- |
| `0x02C3` | Motoréducteur consigne (rpm) | ✅ |
| `0x02C4` | Motoréducteur mesure instantanée (rpm, par impulsions 0–~2800 ; vidéo télécommande 23/11 18:14) | ✅ |
| `0x02BF` | Leonardo consigne | ✅ |
| `0x02D9` | Leonardo mesure (123 = consigne 123, 23/11 18:07) | ✅ |

## Entretien (reset « RESET HEURES », menu 6446, 27/11/2026 ~16:16)

| Registre | Avant reset | Après | Interprétation |
| --- | --- | --- | --- |
| `0x0624` | 234 | 0 | 🟡 compteur depuis entretien (P1 ?) |
| `0x0625` | 389 | 0 | 🟡 (P2 ?) |
| `0x0626` | 861 | 0 | 🟡 (P3 ?) |
| `0x0627` | 280 | 0 | 🟡 (P4 ?) |
| `0x0628` | 0x0101 (257) | 0 | 🟡 (P5 ? ou 2 octets) |
| Warning flags bit `0x00010000` | 1 | 0 | ✅ alerte « clé entretien » de la télécommande |

Somme avant reset : 2 021. Heures totales et allumages inchangés (3 178 h, 1 584).
À confirmer : ces registres doivent remonter au rythme des heures par puissance.
