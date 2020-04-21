Contiki-NG configuration
========================
[Source Github](https://github.com/contiki-ng/contiki-ng/wiki/The-Contiki%E2%80%90NG-configuration-system#network-stack-mac-layer)

Configuration af Contiki-NG
===========================

## Udskiftning af MAC-Layer

* * *

I make filen kan man sætte en variable `MAKE_MAC`

Den kan sættes til de følgende værdier

*   `MAKE_MAC_NULLMAC`

*   `MAKE_MAC_CSMA` (default)

*   `MAKE_MAC_TSCH`

*   `MAKE_MAC_BLE`

*   `MAKE_MAC_OTHER`

Selvom disse er sat, skal man stadig være opmærksom på at det også kan være sat i .h filen i `NETSTACK_CONF_MAC` dette før der kan være andre implementeringer af de forskellige protokoller.

Læs mere på source siden.

## Udskiftning af NET Layer

* * *

Man kan vælge net lag ved at sætte `MAKE_NET` variablen i Make filen.

Den kan sættes til følgende værdier:

*   `MAKE_NET_NULLNET`

*   `MAKE_NET_IPV6` (default)

*   `MAKE_NET_OTHER`

Denne kan også overskrives i .h filen med `NETSTACK_CONF_NETWORK`

## Routing protocol

* * *

Man kan sætte MAKE\_ROUTING i make filen

Den kan sættes til følgende værdi:

*   `MAKE_ROUTING_NULLROUTING`

*   `MAKE_ROUTING_RPL_LITE` (default)

*   `MAKE_ROUTING_RPL_CLASSIC`