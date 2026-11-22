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

## Paramètres techniques

| Registre | Contenu | Statut |
| --- | --- | --- |
| `0x0570`–`0x05AF` | Bloc des paramètres techniques (menu installateur) | 🟡 — **jamais écrit par ce firmware** |

## Recherches en cours

- **Hystérésis de standby** (mesurée sur Celia, consigne 23,0) : arrêt quand l'ambiante dépasse la consigne pendant le
  délai de standby ; redémarrage à 21,9 °C, soit sous consigne − 1,0 °C. Aucun réglage dans les menus : valeur fixe.
- Vitesse extracteur vs vis sans fin (tr/min) : à recouper avec les écrans d'info de la télécommande.
- Leonardo P.MIN, T° fumées P.MIN, extraction d'urgence : à localiser dans `0x0570`–`0x05AF`.

**Comment contribuer :** active le mode exploration, note l'heure, change **un seul** réglage à la télécommande,
attends une passe complète (~45 min) et compare l'historique des capteurs « Exploration 0xXXXX ». Ouvre une issue
avec le modèle du poêle, le réglage modifié (avant → après) et le bloc qui a changé.
